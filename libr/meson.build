use_gpl = not get_option('nogpl')
user_plugins = get_option('plugins').split(',')
no_user_plugins = get_option('plugins') == ''

esil_plugins = [ 'dummy' ]
asm_plugins = [ 'null' ]
anal_plugins = [ 'null' ]
arch_plugins = [ 'null',
  # 'arc',
  'pyc',
  'lh5801',
  'ebc',
  'msp430',
  'h8300',
  'sh',
  'sh_cs',
  'x86_nz',
  'alpha',
  'pic',
  'bpf',
  'bpf_cs',
  'lm32',
  'mcs96',
  'ws',
  'arm',
  '6502',
  'or1k',
  'dis',
  'evm',
  'nios2',
  'snes',
  'v850',
  'any_as',
  'any_vasm',
  'jdh8',
  'kvx',
  'xtensa',
  'xap',
  'mcore',
  'riscv',
  'riscv_cs',
  'i4004',
  'rsp',
  'propeller',
  'amd29k',
  'vax',
  'tricore',
  'tricore_cs',
  'cris',
  'cr16',
  'bf',
]
bin_plugins = [ 'any' ]
bin_ldr_plugins = [ 'ldr_linux' ]
bin_xtr_plugins = [ 'xtr_sep64' ]
io_plugins = [ 'malloc', 'xattr', 'fd', 'default', 'null', 'rbuf', 'r2pipe' , 'r2pipe']
fs_plugins = [ 'r2', 'posix', 'io', 'zip' ]
bp_plugins = [ 'bf' ]
crypto_plugins = [ 'xor', 'punycode' ]
core_plugins = [ 'a2f' ]
egg_plugins = [ 'xor' ]
debug_plugins = [ 'null', 'io' ]
lang_plugins = [ 'lib', 'pipe' ]
parse_plugins = [ 'att2intel' ]

if get_option('nogpl')
  user_plugins += ['nogrub']
endif

# This must be splitted in meson.builds for each module
# meson split is results in 1 empty element when splitting an empty string :facepalm:
if not no_user_plugins
  if user_plugins.contains('apple') or user_plugins.contains('macos') or user_plugins.contains('ios')
    io_plugins += [
      'mach',
    ]
    bin_plugins += [
      'mach0', 'mach064',
      'dyldcache',
      'xnu_kernelcache',
    ]
    bin_xtr_plugins += [
      'xtr_dyldcache',
      'xtr_fatmach0',
      'xtr_xalz',
      'xtr_sep64'
    ]
  endif
  if user_plugins.contains('android')
    bin_plugins += [
      'elf', 'elf64',
      'art', 'nso', 'nro'
    ]
    bin_ldr_plugins += [
      'ldr_linux'
    ]
  endif
  if user_plugins.contains('x86')
    arch_plugins += [ 'x86_nz' ]
    anal_plugins += [ 'x86_cs' ]
    bp_plugins += [ 'x86' ]
    parse_plugins += [ 'x86_pseudo' ]
  endif
  if user_plugins.contains('web')
    io_plugins += [ 'http' ]
    io_plugins += [ 'r2web' ]
  endif
  if user_plugins.contains('arm')
    anal_plugins += []
    arch_plugins += [ 'arm', 'arm_cs'  ]
    bp_plugins += [ 'arm' ]
    parse_plugins += [ 'arm_pseudo' ]
  endif
  if user_plugins.contains('v850')
    arch_plugins += [ 'v810', 'v850' ]
    parse_plugins += [ 'v850_pseudo' ]
  endif
  if user_plugins.contains('wasm')
    bin_plugins += [ 'wasm' ]
    arch_plugins += [ 'wasm' ]
    parse_plugins += [ 'wasm_pseudo' ]
  endif
  if user_plugins.contains('sparc')
    arch_plugins += [ 'sparc_cs' ]
    bp_plugins += [ 'sparc' ]
    # missing parse_plugins += [ 'sparc_pseudo' ]
  endif
  if user_plugins.contains('mips')
    arch_plugins += [ 'mips_cs' ]
    if use_gpl
      arch_plugins += [ 'mips_gnu' ]
    endif
    bp_plugins += [ 'mips' ]
    parse_plugins += [ 'mips_pseudo' ]
  endif
  if user_plugins.contains('dalvik')
    bin_plugins += [ 'dex' ]
    arch_plugins += [ 'dalvik' ]
    parse_plugins += [ 'dalvik_pseudo' ]
  endif
  if user_plugins.contains('loongarch')
    arch_plugins += [ 'loongarch_gnu' ]
  endif
  if user_plugins.contains('elf')
    bin_plugins += [ 'elf', 'elf64' ]
  endif
  if user_plugins.contains('pe')
    bin_plugins += [ 'pe', 'pe64', 'coff', 'xcoff64', 'mz', 'ne' ]
    bin_xtr_plugins += [ 'xtr_pemixed' ]
  endif
endif

# else
if no_user_plugins

anal_plugins += [
  'x86_cs',
]

arch_plugins += [
  '8051',
  'avr',
  'dalvik',
  'arm_cs',
  'java',
  'chip8',
  'wasm',
  'i8080',
  'mips_cs',
  '6502_cs',
  'loongarch_gnu',
  's390_cs',
  'm68k_cs',
  'm680x_cs',
  'xcore_cs',
  'tms320',
  'sparc_cs',
  'ppc_cs',
  'gb',
]

if no_user_plugins
  if use_gpl
    arch_plugins += [
      'loongarch_gnu',
      'arm_gnu',
      'z80',
      'pdp11',
      'ppc_gnu',
      'm68k_gnu',
# 'hppa_gnu',
      'sparc_gnu',
      'lanai',
      's390_gnu',
      'mips_gnu',
    ]
  endif
endif

if get_option('use_v35')
  arch_plugins += ['arm_v35']
endif

bin_plugins += [
  'art',
  'avr',
  'bf',
  's390',
  'bflt',
  'bios',
  'bootimg',
  'cgc',
  'coff',
  'dex',
  'dis',
  'wasm',
  'dmp64',
  'dol',
  'dyldcache',
  'elf',
  'elf64',
  'fs',
  'java',
  'le',
  'mach0',
  'mach064',
  'mbn',
  'mdmp',
  'menuet',
  'mz',
  'ne',
  'nes',
  'nin3ds',
  'ninds',
  'ningb',
  'ningba',
  'nro',
  'nso',
  'omf',
  'qnx',
  'p9',
  'pe',
  'pe64',
  'pebble',
  'prg',
  'psxexe',
  'rel',
  'sfc',
  'smd',
  'sms',
  'symbols',
  'te',
  'vsf',
  'xbe',
  'xcoff64',
  'xnu_kernelcache',
  'z64',
  'zimg',
  'xtac'
]

bin_ldr_plugins += [
  'ldr_linux'
]

bin_xtr_plugins += [
  'xtr_dyldcache',
  'xtr_fatmach0',
  'xtr_xalz',
  'xtr_sep64',
]

bp_plugins += [
  'arm',
  'bf',
  'mips',
  'ppc',
  'sh',
  'x86'
]

core_plugins += [
  'a2f',
  'java'
]

crypto_plugins += [
  'aes',
  'aes_cbc',
  'aes_wrap',
  'base64',
  'base91',
  'blowfish',
  'cps2',
  'des',
  'rc2',
  'rc4',
  'rc6',
  'rol',
  'ror',
  'rot',
  'serpent',
]

debug_plugins += [
  'bf',
  'bochs',
  'esil',
  'gdb',
  'io',
  'native',
  'null',
  'qnx',
  'rap',
  'winkd'
]

egg_plugins += [
  'exec',
  #'shya',
]

fs_plugins += [
  'io',
  'posix',
  'r2',
]

if not get_option('nogpl')
  fs_plugins += [
    'cpio', 'ext2', 'fat', 'fb', 'hfs',
    'hfsplus', 'iso9660', 'jfs', 'minix',
    'ntfs', 'reiserfs', 'sfs', 'tar',
    'udf', 'ufs', 'ufs2', 'xfs'
  ]
endif

io_plugins += [
  'ar',
  'fd',
  'reg',
  'bfdbg',
  'bochs',
  'debug',
  'gdb',
  'gprobe',
  'gzip',
  'http',
  'ihex',
  'mach',
  'mmap',
  'procpid',
  'ptrace',
  'qnx',
  'r2k',
  'rap',
  'rbuf',
  'self',
  'shm',
  'sparse',
  'socket',
  'tcpslurp',
  'winkd',
  'winedbg',
  'zip'
]

if host_machine.system() == 'windows'
  io_plugins += [
    'windbg',
    'w32dbg',
    'w32',
  ]
  debug_plugins += 'windbg'
endif

lang_plugins += [
  'v',
  'lib',
  'spp',
  'rust',
  'go',
  'zig',
  's',
  'asm',
  'c',
  'vala'
]

if host_machine.system() != 'windows'
  lang_plugins += [
    'pipe',
    'cpipe'
  ]
endif

parse_plugins += [
  '6502_pseudo',
  'bpf_pseudo',
  'evm_pseudo',
  'arm_pseudo',
  'avr_pseudo',
  'chip8_pseudo',
  'dalvik_pseudo',
  'm68k_pseudo',
  'mips_pseudo',
  'ppc_pseudo',
  'riscv_pseudo',
  'sh_pseudo',
  'tms320_pseudo',
  'v850_pseudo',
  'x86_pseudo',
  'z80_pseudo',
  'wasm_pseudo'
]

endif

include_files = [
  'include/r2naked.h',
  'include/rvc.h',
  'include/r_agraph.h',
  'include/r_anal.h',
  'include/r_arch.h',
  'include/r_esil.h',
  'include/r_asm.h',
  'include/r_bin.h',
  'include/r_bin_dwarf.h',
  'include/r_bind.h',
  'include/r_bp.h',
  'include/r_cmd.h',
  'include/r_codemeta.h',
  'include/r_config.h',
  'include/r_cons.h',
  'include/r_core.h',
  'include/r_crypto.h',
  'include/r_debug.h',
  'include/r_drx.h',
  'include/r_egg.h',
  'include/r_endian.h',
  'include/r_flag.h',
  'include/r_fs.h',
  'include/r_getopt.h',
  'include/r_hash.h',
  'include/r_heap_glibc.h',
  'include/r_heap_jemalloc.h',
  'include/r_io.h',
  'include/r_lang.h',
  'include/r_lib.h',
  'include/r_list.h',
  'include/r_magic.h',
  'include/r_main.h',
  'include/r_parse.h',
  'include/r_pdb.h',
  'include/r_qrcode.h',
  'include/r_reg.h',
  'include/r_regex.h',
  'include/r_search.h',
  'include/r_sign.h',
  'include/r_skiplist.h',
  'include/r_socket.h',
  'include/r_syscall.h',
  'include/r_th.h',
  'include/r_types.h',
  'include/r_types_base.h',
  'include/r_types_overflow.h',
  'include/r_util.h',
  'include/r_vector.h',
]
install_headers(include_files, install_dir: r2_incdir)

r_util_files = [
  'include/r_util/bplist.h',
  'include/r_util/pj.h',
  'include/r_util/r_alloc.h',
  'include/r_util/r_ascii_table.h',
  'include/r_util/r_asn1.h',
  'include/r_util/r_assert.h',
  'include/r_util/r_axml.h',
  'include/r_util/r_base36.h',
  'include/r_util/r_base64.h',
  'include/r_util/r_base91.h',
  'include/r_util/r_big.h',
  'include/r_util/r_bitmap.h',
  'include/r_util/r_buf.h',
  'include/r_util/r_ctypes.h',
  'include/r_util/r_debruijn.h',
  'include/r_util/r_diff.h',
  'include/r_util/r_event.h',
  'include/r_util/r_file.h',
  'include/r_util/r_graph.h',
  'include/r_util/r_graph_drawable.h',
  'include/r_util/r_hex.h',
  'include/r_util/r_idpool.h',
  'include/r_util/r_intervaltree.h',
  'include/r_util/r_itv.h',
  'include/r_util/r_json.h',
  'include/r_util/r_log.h',
  'include/r_util/r_mem.h',
  'include/r_util/r_name.h',
  'include/r_util/r_new_rbtree.h',
  'include/r_util/r_num.h',
  'include/r_util/r_panels.h',
  'include/r_util/r_pkcs7.h',
  'include/r_util/r_pool.h',
  'include/r_util/r_print.h',
  'include/r_util/r_protobuf.h',
  'include/r_util/r_punycode.h',
  'include/r_util/r_queue.h',
  'include/r_util/r_range.h',
  'include/r_util/r_rbtree.h',
  'include/r_util/r_ref.h',
  'include/r_util/r_sandbox.h',
  'include/r_util/r_signal.h',
  'include/r_util/r_spaces.h',
  'include/r_util/r_stack.h',
  'include/r_util/r_str.h',
  'include/r_util/r_str_constpool.h',
  'include/r_util/r_str_util.h',
  'include/r_util/r_strbuf.h',
  'include/r_util/r_strpool.h',
  'include/r_util/r_sys.h',
  'include/r_util/r_table.h',
  'include/r_util/r_time.h',
  'include/r_util/r_token.h',
  'include/r_util/r_tree.h',
  'include/r_util/r_uleb128.h',
  'include/r_util/r_utf16.h',
  'include/r_util/r_utf32.h',
  'include/r_util/r_utf8.h',
  'include/r_util/r_w32.h',
  'include/r_util/r_w32dw.h',
  'include/r_util/r_x509.h',
  'include/r_util/r_xml.h'
]
install_headers(r_util_files, install_dir: join_paths(r2_incdir, 'r_util'))

r_crypto_files = [
  'include/r_crypto/r_aes.h',
  'include/r_crypto/r_des.h',
  'include/r_crypto/r_sm4.h'
]
install_headers(r_crypto_files, install_dir: join_paths(r2_incdir, 'r_crypto'))

r_anal_files = [
  'include/r_anal/op.h',
  'include/r_anal/hint.h',
]
install_headers(r_anal_files, install_dir: join_paths(r2_incdir, 'r_anal'))

r_esil_files = [
  'include/r_esil.h',
]
install_headers(r_esil_files, install_dir: join_paths(r2_incdir, 'r_esil'))

sflib_common_files = [
  join_paths('include/sflib/common/sftypes.h'),
  join_paths('include/sflib/common/sfsocketcall.h')
]

sflib_arch = [
  'darwin-arm-64',
  'darwin-x86-32',
  'darwin-x86-64',
  'freebsd-x86-32',
  'linux-arm-32',
  'linux-arm-64',
  'linux-x86-32',
  'linux-x86-64',
]

sflib_arch_files = [
  'sflib.h',
  'sfsyscall.h',
  'sfsysnr.h'
]

install_headers(sflib_common_files, install_dir: join_paths(r2_incdir, 'sflib', 'common'))
foreach arch : sflib_arch
  foreach file : sflib_arch_files
    dst = join_paths('include', 'sflib', arch, file)
    install_headers(dst, install_dir: join_paths(r2_incdir, 'sflib', arch))
  endforeach
endforeach

sdb_readlink_script = '''#script
import os
import sys
infile = sys.argv[1]
if os.name == 'nt' and os.path.getsize(infile) <= 64:
    with open(infile) as sdb_txt:
        fname = sdb_txt.readline()
    if fname.endswith('.sdb.txt'):
        infile = fname
else:
    infile = os.path.basename(infile)
print(infile)
'''

sdb_readlink_cmd = [
  py3_exe,
  '-c',
  sdb_readlink_script
]
