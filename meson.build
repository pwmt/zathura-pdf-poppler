project('zathura-pdf-poppler', 'c',
  version: '0.2.8',
  meson_version: '>=0.43',
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
zathura = dependency('zathura', version: '>=0.3.8')
girara = dependency('girara-gtk3')
glib = dependency('glib-2.0')
poppler = dependency('poppler-glib', version: '>=0.18')

build_dependencies = [zathura, girara, glib, poppler]

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
  '-Werror=vla'
]
flags = cc.get_supported_arguments(flags)

sources = files(
  'attachments.c',
  'document.c',
  'forms.c',
  'image.c',
  'index.c',
  'links.c',
  'meta.c',
  'page.c',
  'plugin.c',
  'render.c',
  'search.c',
  'select.c',
  'utils.c'
)

pdf = shared_module('pdf-poppler',
  sources,
  dependencies: build_dependencies,
  c_args: defines + flags,
  install: true,
  install_dir: zathura.get_pkgconfig_variable('plugindir')
)

subdir('data')
