project(
  'benchmark-harf',
  'cpp',
  version: '0.1.0',
  default_options: [
    'buildtype=release',
    'cpp_std=c++17',
    'warning_level=2',
  ],
)

cpp = meson.get_compiler('cpp')
fs = import('fs')

hb_root = get_option('hb_root')
hb_build = get_option('hb_build')
hr_root = get_option('hr_root')
hr_build = get_option('hr_build')

if hb_root == ''
  hb_root = meson.project_source_root() / '..' / 'hb'
endif
if hb_build == ''
  hb_build = hb_root / 'build'
endif
if hr_root == ''
  hr_root = meson.project_source_root() / '..' / 'hr'
endif
if hr_build == ''
  hr_build = hr_root / 'target' / 'release'
endif

foreach path : [hb_root, hb_build, hr_root, hr_build]
  if not fs.is_absolute(path)
    error('Source and build paths must be absolute: ' + path)
  endif
endforeach

hb_inc = hb_root / 'src'
hb_libdir = hb_build / 'src'
hr_inc = hr_root / 'harfrust-capi' / 'include'
hr_libdir = hr_build

if not fs.exists(hb_inc / 'hb.h')
  error('HarfBuzz header not found: ' + hb_inc / 'hb.h')
endif
if not fs.exists(hr_inc / 'hr-hb.h')
  error('HarfRust compatibility header not found: ' + hr_inc / 'hr-hb.h')
endif

hb_lib = cpp.find_library('harfbuzz', dirs: hb_libdir, required: true)
hr_lib = cpp.find_library('harfrust_c', dirs: hr_libdir, required: true)
glib_dep = dependency('glib-2.0')
benchmark_dep = dependency(
  'benchmark',
  fallback: ['google-benchmark', 'google_benchmark_dep'],
  default_options: ['tests=disabled'],
)

conf = configuration_data()
conf.set_quoted('HB_SOURCE_ROOT', hb_root)
configure_file(
  output: 'benchmark-config.hh',
  configuration: conf,
)

harfbuzz_core = static_library(
  'benchmark-core-harfbuzz',
  'benchmark-core.cc',
  cpp_args: [
    '-DBENCHMARK_HARFBUZZ',
    '-isystem' + hb_inc,
  ],
  dependencies: benchmark_dep,
  install: false,
)

harfrust_core = static_library(
  'benchmark-core-harfrust',
  'benchmark-core.cc',
  cpp_args: [
    '-DBENCHMARK_HARFRUST',
    '-isystem' + hr_inc,
  ],
  dependencies: benchmark_dep,
  install: false,
)

executable(
  'benchmark-harf',
  'benchmark-main.cc',
  dependencies: [
    benchmark_dep,
    glib_dep,
    hb_lib,
    hr_lib,
  ],
  link_whole: [harfbuzz_core, harfrust_core],
  build_rpath: hb_libdir + ':' + hr_libdir,
  install: false,
)
