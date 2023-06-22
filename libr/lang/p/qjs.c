/* radare - LGPL - Copyright 2020-2023 pancake */

#include <r_lib.h>
#include <r_core.h>
#include <r_vec.h>

#define countof(x) (sizeof (x) / sizeof ((x)[0]))

#include "quickjs.h"
#include "../js_require.c"
#include "../js_r2papi.c"

typedef struct {
	R_BORROW JSContext *ctx;
	JSValue func;
} QjsContext;

typedef struct qjs_core_plugin {
	QjsContext qctx;
	char *name;
	// void *data;  // can be added later if needed
} QjsCorePlugin;

typedef void (*CorePluginFini)(QjsCorePlugin *ps, void *user);

R_GENERATE_VEC_IMPL_FOR(CorePlugin, QjsCorePlugin);

typedef struct qjs_plugin_manager_t {
	R_BORROW RCore *core;
	R_BORROW JSRuntime *rt;
	RVecCorePlugin core_plugins;
	CorePluginFini fini_core_plugin;
	// XXX add arch state here too
} QjsPluginManager;

typedef struct qjs_plugin_data_t {
	QjsPluginManager pm;
	QjsContext qc;  // default context for running normal JS code
	// R2_590 add more state as needed to remove all globals
} QjsPluginData;

static void core_plugin_fini (QjsCorePlugin *cp, void *user) {
	free (cp->name);
}

static bool plugin_manager_init (QjsPluginManager *pm, RCore *core, JSRuntime *rt) {
	pm->core = core;
	pm->rt = rt;
	RVecCorePlugin_init (&pm->core_plugins);
	pm->fini_core_plugin = core_plugin_fini;
	return true;
}

static void plugin_manager_add_core_plugin(QjsPluginManager *pm, const char *name, JSContext *ctx, JSValue func) {
	r_return_if_fail (pm);

	QjsCorePlugin *cp = RVecCorePlugin_emplace_back (&pm->core_plugins);
	if (cp) {
		cp->name = name? strdup (name): NULL;
		cp->qctx.ctx = ctx;
		cp->qctx.func = func;
	}
}

static QjsCorePlugin *plugin_manager_find_core_plugin(const QjsPluginManager *pm, const char *name) {
	r_return_val_if_fail (pm, NULL);

	QjsCorePlugin *cp;
	R_VEC_FOREACH (&pm->core_plugins, cp) {
		if (!strcmp (cp->name, name)) {
			return cp;
		}
	}

	return NULL;
}

static bool plugin_manager_remove_core_plugin(QjsPluginManager *pm, const char *name) {
	r_return_val_if_fail (pm, false);

	bool found = false;
	size_t i = 0;
	QjsCorePlugin *cp;
	R_VEC_FOREACH (&pm->core_plugins, cp) {
		if (!strcmp (cp->name, name)) {
			found = true;
			break;
		}

		i++;
	}

	if (found) {
		RVecCorePlugin_remove (&pm->core_plugins, i, pm->fini_core_plugin, NULL);
		return true;
	}

	return false;
}

static void plugin_manager_fini (QjsPluginManager *pm) {
	RVecCorePlugin_fini (&pm->core_plugins, pm->fini_core_plugin, NULL);
	JS_FreeRuntime (pm->rt);
	pm->rt = NULL;
}

#include "qjs/loader.c"
#include "qjs/arch.c"
#include "qjs/core.c"

///////////////////////////////////////////////////////////

static bool eval(JSContext *ctx, const char *code);

static void eval_jobs(JSContext *ctx) {
	JSRuntime *rt = JS_GetRuntime (ctx);
	JSContext *pctx = NULL;
	do {
		int res = JS_ExecutePendingJob (rt, &pctx);
		if (res == -1) {
			R_LOG_ERROR ("Exception in pending job");
		}
	} while (pctx);
}

static void r2qjs_dump_obj(JSContext *ctx, JSValueConst val) {
	const char *str = JS_ToCString (ctx, val);
	if (str) {
		R_LOG_ERROR ("%s", str);
		JS_FreeCString (ctx, str);
	} else {
		R_LOG_ERROR ("[exception]");
	}
}

static void js_std_dump_error1(JSContext *ctx, JSValueConst exception_val) {
	JSValue val;
	bool is_error;

	is_error = JS_IsError (ctx, exception_val);
	r2qjs_dump_obj (ctx, exception_val);
	if (is_error) {
		val = JS_GetPropertyStr (ctx, exception_val, "stack");
		if (!JS_IsUndefined (val)) {
			r2qjs_dump_obj (ctx, val);
		}
		JS_FreeValue (ctx, val);
	}
}

static void js_std_dump_error(JSContext *ctx) {
	JSValue exception_val;
	exception_val = JS_GetException (ctx);
	js_std_dump_error1 (ctx, exception_val);
	JS_FreeValue (ctx, exception_val);
}

static JSValue r2log(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
	size_t plen;
	const char *n = JS_ToCStringLen2 (ctx, &plen, argv[0], false);
	r_cons_printf ("%s\n", n);
	return JS_NewBool (ctx, true);
}

static JSValue r2error(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
	size_t plen;
	const char *n = JS_ToCStringLen2 (ctx, &plen, argv[0], false);
	eprintf ("%s\n", n);
	return JS_NewBool (ctx, true);
}

static JSValue b64(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
	size_t plen;
	bool decode = false;
	if (argc > 1) {
		decode = true;
	}
	char *ret = NULL;
	if (argc > 0) {
		const char *n = JS_ToCStringLen2 (ctx, &plen, argv[0], false);
		if (R_STR_ISNOTEMPTY (n)) {
			if (decode) {
				int res = 0;
				ut8 *bret = sdb_decode (n, &res);
				ret = r_str_ndup ((const char *)bret, res);
				free (bret);
			} else {
				ret = sdb_encode ((const ut8*)n, -1);
			}
		}
	}
	JSValue v = JS_NewString (ctx, r_str_get (ret));
	free (ret);
	return v;
}

static JSValue r2plugin(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
	size_t plen;
	const char *n = JS_ToCStringLen2 (ctx, &plen, argv[0], false);
	if (R_STR_ISNOTEMPTY (n)) {
		if (!strcmp (n, "core")) {
			return r2plugin_core_load (ctx, this_val, argc, argv);
		} else if (!strcmp (n, "arch")) {
			return r2plugin_arch (ctx, this_val, argc, argv);
#if 0
		} else if (!strcmp (n, "bin")) {
			return r2plugin_bin (ctx, this_val, argc, argv);
		} else if (!strcmp (n, "io")) {
			return r2plugin_io (ctx, this_val, argc, argv);
#endif
		} else {
			// invalid throw exception here
			return JS_ThrowRangeError(ctx, "invalid r2plugin type");
		}
	}
	return JS_NewBool (ctx, false);
}

static JSValue r2plugin_unload(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
	if (argc != 1 || !JS_IsString (argv[0])) {
		return JS_ThrowRangeError (ctx, "r2.unload takes only one string as argument");
	}
	JSRuntime *rt = JS_GetRuntime (ctx);
	QjsPluginData *pd = JS_GetRuntimeOpaque (rt);
	size_t plen;
	const char *name = JS_ToCStringLen2 (ctx, &plen, argv[0], false);
	pd->pm.core->lang->cmdf (pd->pm.core, "L-%s", name);
	bool res = plugin_manager_remove_core_plugin (&pd->pm, name);
	// invalid throw exception here
	// return JS_ThrowRangeError(ctx, "invalid r2plugin type");
	return JS_NewBool (ctx, res);
}

// WIP experimental
static JSValue r2cmd0(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
	JSRuntime *rt = JS_GetRuntime (ctx);
	QjsPluginData *pd = JS_GetRuntimeOpaque (rt);
	size_t plen;
	const char *n = JS_ToCStringLen2 (ctx, &plen, argv[0], false);
	int ret = 0;
	if (R_STR_ISNOTEMPTY (n)) {
		ret = pd->pm.core->lang->cmdf (pd->pm.core, "%s@e:scr.null=true", n);
	}
	// JS_FreeValue (ctx, argv[0]);
	return JS_NewInt32 (ctx, ret);
}

// WIP experimental
static JSValue r2call0(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
	JSRuntime *rt = JS_GetRuntime (ctx);
	QjsPluginData *pd = JS_GetRuntimeOpaque (rt);
	size_t plen;
	const char *n = JS_ToCStringLen2 (ctx, &plen, argv[0], false);
	int ret = 0;
	if (R_STR_ISNOTEMPTY (n)) {
		pd->pm.core->lang->cmdf (pd->pm.core, "\"\"e scr.null=true");
		ret = pd->pm.core->lang->cmdf (pd->pm.core, "\"\"%s", n);
		pd->pm.core->lang->cmdf (pd->pm.core, "\"\"e scr.null=false");
	}
	// JS_FreeValue (ctx, argv[0]);
	return JS_NewInt32 (ctx, ret);
}

static JSValue r2cmd(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
	JSRuntime *rt = JS_GetRuntime (ctx);
	QjsPluginData *pd = JS_GetRuntimeOpaque (rt);
	size_t plen;
	const char *n = JS_ToCStringLen2 (ctx, &plen, argv[0], false);
	char *ret = NULL;
	if (R_STR_ISNOTEMPTY (n)) {
		ret = pd->pm.core->lang->cmd_str (pd->pm.core, n);
	}
	// JS_FreeValue (ctx, argv[0]);
	return JS_NewString (ctx, r_str_get (ret));
}

static JSValue js_write(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
	int i;
	const char *str;
	size_t len;

	for (i = 0; i < argc; i++) {
		if (i != 0) {
			putchar (' ');
		}
		str = JS_ToCStringLen (ctx, &len, argv[i]);
		if (!str) {
			return JS_EXCEPTION;
		}
		if (len > 0) {
			fwrite (str, 1, len, stdout);
		}
		JS_FreeCString (ctx, str);
	}
	return JS_UNDEFINED;
}

static JSValue js_flush(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
	r_cons_flush ();
	fflush (stdout);
	return JS_UNDEFINED;
}

static JSValue js_print(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
	JSValue a = js_write (ctx, this_val, argc, argv);
	putchar ('\n');
	return a;
}

static JSValue js_os_pending(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int magic) {
	eval_jobs (ctx);
	return JS_UNDEFINED;
}

static JSValue js_os_read_write(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int magic) {
	int fd;
	uint64_t pos, len;
	size_t size;
	int ret;

	if (JS_ToInt32 (ctx, &fd, argv[0])) {
		return JS_EXCEPTION;
	}
	if (JS_ToIndex (ctx, &pos, argv[2])) {
		return JS_EXCEPTION;
	}
	if (JS_ToIndex (ctx, &len, argv[3])) {
		return JS_EXCEPTION;
	}
	uint8_t *buf = JS_GetArrayBuffer (ctx, &size, argv[1]);
	if (!buf) {
		return JS_EXCEPTION;
	}
	if (pos + len > size) {
		return JS_ThrowRangeError (ctx, "read/write array buffer overflow");
	}
	if (magic) {
		ret = write (fd, buf + pos, len);
	} else {
		ret = read (fd, buf + pos, len);
	}
	return JS_NewInt64 (ctx, ret);
}

static const JSCFunctionListEntry js_os_funcs[] = {
	JS_CFUNC_MAGIC_DEF ("read", 4, js_os_read_write, 0 ),
	JS_CFUNC_MAGIC_DEF ("write", 4, js_os_read_write, 1 ),
	JS_CFUNC_MAGIC_DEF ("pending", 4, js_os_pending, 0 ),
#if 0
	JS_CFUNC_MAGIC_DEF("setReadHandler", 2, js_os_setReadHandler, 0 ),
	JS_CFUNC_DEF("setTimeout", 2, js_os_setTimeout ),
	JS_CFUNC_DEF("clearTimeout", 1, js_os_clearTimeout ),
#endif
	// JS_CFUNC_DEF("open", 2, js_os_open ),
	// OS_FLAG(O_RDONLY),
};

static int js_os_init(JSContext *ctx, JSModuleDef *m) {
	return JS_SetModuleExportList(ctx, m, js_os_funcs, countof (js_os_funcs));
}

static JSModuleDef *js_init_module_os(JSContext *ctx) {
	JSModuleDef *m = JS_NewCModule (ctx, "os", js_os_init);
	if (m) {
		JS_AddModuleExportList (ctx, m, js_os_funcs, countof (js_os_funcs));
	}
	return m;
}

static const JSCFunctionListEntry js_r2_funcs[] = {
	JS_CFUNC_DEF ("cmd", 1, r2cmd),
	JS_CFUNC_DEF ("plugin", 2, r2plugin),
	JS_CFUNC_DEF ("unload", 1, r2plugin_unload),
	// JS_CFUNC_DEF ("cmdj", 1, r2cmdj), // can be implemented in js
	JS_CFUNC_DEF ("log", 1, r2log),
	JS_CFUNC_DEF ("error", 1, r2error),
	JS_CFUNC_DEF ("cmd0", 1, r2cmd0),
	// implemented in js JS_CFUNC_DEF ("call", 1, r2call);
	JS_CFUNC_DEF ("call0", 1, r2call0),
};

static int js_r2_init(JSContext *ctx, JSModuleDef *m) {
	return JS_SetModuleExportList (ctx, m, js_r2_funcs, countof (js_r2_funcs));
}

static JSModuleDef *js_init_module_r2(JSContext *ctx) {
	JSModuleDef *m = JS_NewCModule (ctx, "r2", js_r2_init);
	if (m) {
		JSValue global_obj = JS_GetGlobalObject (ctx);
		JSValue name = JS_NewString(ctx, "r2");
		JSValue v = JS_NewObjectProtoClass(ctx, name, 0);
		JS_SetPropertyStr (ctx, global_obj, "r2", v);
		JS_SetPropertyFunctionList(ctx, v, js_r2_funcs, countof(js_r2_funcs));
		// JS_AddModuleExportList (ctx, m, js_r2_funcs, countof(js_r2_funcs));
	}
	return m;
}

static void register_helpers(JSContext *ctx) {
#if 0
	JSRuntime *rt = JS_GetRuntime (ctx);
	js_std_set_worker_new_context_func (JS_NewCustomContext);
	js_std_init_handlers (rt);

	JS_SetModuleLoaderFunc (rt, NULL, js_module_loader, NULL);
#endif
	/*
	JSModuleDef *m = JS_NewCModule (ctx, "r2", js_r2_init);
	if (!m) {
		return;
	}
	js_r2_init (ctx, m);
	*/
	js_init_module_os (ctx);
	js_init_module_r2 (ctx);
	r2qjs_modules (ctx);
	// JS_AddModuleExportList (ctx, m, js_r2_funcs, countof (js_r2_funcs));
	JSValue global_obj = JS_GetGlobalObject (ctx);
	// JS_SetPropertyStr (ctx, global_obj, "r2", global_obj); // JS_NewCFunction (ctx, b64, "b64", 1));
	JS_SetPropertyStr (ctx, global_obj, "b64", JS_NewCFunction (ctx, b64, "b64", 1));
	// r2cmd deprecate . we have r2.cmd already same for r2log
	JS_SetPropertyStr (ctx, global_obj, "r2cmd", JS_NewCFunction (ctx, r2cmd, "r2cmd", 1));
	JS_SetPropertyStr (ctx, global_obj, "r2log", JS_NewCFunction (ctx, r2log, "r2log", 1));
	JS_SetPropertyStr (ctx, global_obj, "write", JS_NewCFunction (ctx, js_write, "write", 1));
	JS_SetPropertyStr (ctx, global_obj, "flush", JS_NewCFunction (ctx, js_flush, "flush", 1));
	JS_SetPropertyStr (ctx, global_obj, "print", JS_NewCFunction (ctx, js_print, "print", 1));
	eval (ctx, "setTimeout = (x,y) => x();");
	eval (ctx, "function dump(x) {"
		"if (typeof x==='object' && Object.keys(x)[0] != '0') { for (let k of Object.keys(x)) { console.log(k);}} else "
		"if (typeof x==='number'&& x > 0x1000){console.log(R.hex(x));}else"
		"{console.log((typeof x==='string')?x:JSON.stringify(x, null, 2));}"
		"}");
	eval (ctx, "var console = { log:print, error:print, debug:print };");
	eval (ctx, "r2.cmdj = (x) => JSON.parse(r2.cmd(x));");
	eval (ctx, "r2.call = (x) => r2.cmd('\"\"' + x);");
	eval (ctx, "r2.callj = (x)=> JSON.parse(r2.call(x));");
	eval (ctx, "var global = globalThis; var G = globalThis;");
	eval (ctx, js_require_qjs);
	eval (ctx, "var exports = {};");
	eval (ctx, "G.r2pipe = {open: function(){ return R.r2;}};");
	eval (ctx, "G.R2Pipe=() => R.r2;");
	if (!r_sys_getenv_asbool ("R2_DEBUG_NOPAPI")) {
		eval (ctx, js_r2papi_qjs);
		eval (ctx, "R=G.R=new R2Papi(r2);");
	} else {
		eval (ctx, "R=r2;");
	}
}

static JSContext *JS_NewCustomContext(JSRuntime *rt) {
	JSContext *ctx = JS_NewContext (rt);
	// JSContext *ctx = JS_NewContextRaw (rt);
	if (!ctx) {
		return NULL;
	}
#if CONFIG_BIGNUM
	JS_AddIntrinsicBigFloat (ctx);
	JS_AddIntrinsicBigDecimal (ctx);
	JS_AddIntrinsicOperators (ctx);
	JS_EnableBignumExt (ctx, true);
#endif
	register_helpers (ctx);
	return ctx;
}

static bool eval(JSContext *ctx, const char *code) {
	if (R_STR_ISEMPTY (code)) {
		return false;
	}
	bool wantRaw = strstr (code, "termInit(");
	if (wantRaw) {
		r_cons_set_raw (true);
	}
	int flags = JS_EVAL_TYPE_GLOBAL; //  | JS_EVAL_TYPE_MODULE; //  | JS_EVAL_FLAG_STRICT;
	if (*code == '-') {
		flags = JS_EVAL_TYPE_GLOBAL | JS_EVAL_TYPE_MODULE; //  | JS_EVAL_FLAG_STRICT;
		code++;
	}
	JSValue v = JS_Eval (ctx, code, strlen (code), "-", flags);
	if (JS_IsException (v)) {
		js_std_dump_error (ctx);
		JSValue e = JS_GetException (ctx);
		r2qjs_dump_obj (ctx, e);
	}
	eval_jobs (ctx);
	if (wantRaw) {
		r_cons_set_raw (false);
	}
	// restore raw console
	JS_FreeValue (ctx, v);
	return true;
}

static bool lang_quickjs_run(RLangSession *s, const char *code, int len) {
	r_return_val_if_fail (s && s->plugin_data && code, false);
	QjsPluginData *pd = s->plugin_data;
	return eval (pd->qc.ctx, code);
}

static bool lang_quickjs_file(RLangSession *s, const char *file) {
	r_return_val_if_fail (s && s->plugin_data && file, false);

	QjsPluginData *pd = s->plugin_data;
	QjsContext *qctx = &pd->qc;
	bool rc = false;
	char *code = r_file_slurp (file, NULL);
	if (code) {
		int loaded = r2qjs_loader (qctx->ctx, code);
		if (loaded == 1) {
			rc = true;
		} else if (loaded == -1) {
			// Error loading the file
			return false;
		} else {
			// not a package
			rc = eval (qctx->ctx, code) == 0;
			free (code);
			rc = true;
		}
	}
	return rc;
}

static bool init(RLangSession *ls) {
	if (ls == NULL) {
		// when ls is null means that we want to check if we can use it
		return true;
	}

	if (ls->plugin_data) {
		R_LOG_ERROR ("qjs lang plugin already loaded");
		return false;
	}

	JSRuntime *rt = JS_NewRuntime ();
	if (!rt) {
		return false;
	}

	JSContext *ctx = JS_NewCustomContext (rt);
	if (!ctx) {
		JS_FreeRuntime (rt);
		return false;
	}

	QjsPluginData *pd = R_NEW0 (QjsPluginData);
	if (!pd) {
		JS_FreeContext (ctx);
		JS_FreeRuntime (rt);
		return false;
	}

	RCore *core = ls->lang->user;
	plugin_manager_init (&pd->pm, core, rt);

	JSValue func = JS_NewBool (ctx, false); // fake function
	QjsContext *qc = &pd->qc;
	qc->ctx = ctx;
	qc->func = func;
	r2qjs_modules (ctx);
	JS_SetRuntimeOpaque (rt, pd);  // expose pd to all qjs native functions in R2

	ls->plugin_data = pd;
	return true;
}

static bool fini(RLangSession *s) {
	r_return_val_if_fail (s && s->plugin_data, false);

	QjsPluginData *pd = s->plugin_data;

	plugin_manager_fini (&pd->pm);

	QjsContext *qctx = &pd->qc;
	JS_FreeContext (qctx->ctx);
	qctx->ctx = NULL;

	free (pd);
	s->plugin_data = NULL;

	// XXX do we also remove core qjs plugin here?
	return true;
}

static RLangPlugin r_lang_plugin_qjs = {
	.name = "qjs",
	.ext = "qjs",
	.license = "MIT",
	.desc = "JavaScript extension language using QuickJS",
	.run = lang_quickjs_run,
	.run_file = lang_quickjs_file,
	.init = init,
	.fini = fini,
};

#if !CORELIB
RLibStruct radare_plugin = {
	.type = R_LIB_TYPE_LANG,
	.data = &r_lang_plugin_qjs,
	.version = R2_VERSION
};
#endif
