project('zathura-pdf-poppler', 'c',
  version: '0.2.9',
  meson_version: '>=0.47',
  default_options: 'c_std=c11'
)

version = meson.project_version()
version_array = version.split('.')

cc = meson.get_compiler('c')

prefix = get_option('prefix')
datadir = get_option('datadir')
metainfodir = join_paths(datadir, 'metainfo')
desktopdir = join_paths(datadir, 'applications')

# required dependencies
libzathura = dependency('libzathura', version: '>=0.0.1', fallback: ['libzathura', 'libzathura_dependency'])
glib = dependency('glib-2.0')
poppler = dependency('poppler-glib', version: '>=0.18')

build_dependencies = [libzathura, glib, poppler]

# defines
defines = [
  '-DVERSION_MAJOR=@0@'.format(version_array[0]),
  '-DVERSION_MINOR=@0@'.format(version_array[1]),
  '-DVERSION_REV=@0@'.format(version_array[2]),
  '-D_DEFAULT_SOURCE',
]

# compile flags
flags = [
  '-Wall',
  '-Wextra',
  '-pedantic',
  '-Werror=implicit-function-declaration',
  '-Werror=vla',
]
flags = cc.get_supported_arguments(flags)

# optional dependencies
cairo = dependency('cairo', required: get_option('enable-cairo'))
libfiu = dependency('libfiu', required: get_option('enable-libfiu'))

if cairo.found()
  build_dependencies += cairo
  defines += '-DHAVE_CAIRO'
endif

if libfiu.found()
  build_dependencies += libfiu
  defines += '-DWITH_LIBFIU'
endif

if get_option('plugindir') == ''
  if libzathura.type_name() == 'pkgconfig'
    plugindir = libzathura.get_pkgconfig_variable('plugindir')
  else
    libzathura_subproject = subproject('libzathura')
    plugindir = '/tmp'
  endif
else
  plugindir = get_option('plugindir')
endif

include_directories = [
  include_directories('.')
]

sources = files(
  'zathura-pdf-poppler/attachments.c',
  'zathura-pdf-poppler/annotations.c',
  'zathura-pdf-poppler/document.c',
  'zathura-pdf-poppler/form-fields.c',
  'zathura-pdf-poppler/image.c',
  'zathura-pdf-poppler/outline.c',
  'zathura-pdf-poppler/links.c',
  'zathura-pdf-poppler/meta.c',
  'zathura-pdf-poppler/page.c',
  'zathura-pdf-poppler/plugin.c',
  'zathura-pdf-poppler/render.c',
  'zathura-pdf-poppler/search.c',
  'zathura-pdf-poppler/select.c',
  'zathura-pdf-poppler/utils.c'
)

pdf = shared_module('pdf-poppler',
  sources,
  dependencies: build_dependencies,
  c_args: defines + flags,
  install: true,
  install_dir: plugindir
)

subdir('data')
subdir('tests')
