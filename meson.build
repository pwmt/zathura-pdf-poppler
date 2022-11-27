project('zathura-pdf-poppler', 'c',
  version: '0.3.1',
  meson_version: '>=0.56',
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
zathura = dependency('zathura', version: '>=0.5.2', fallback: ['zathura', 'zathura_dependency'])
girara = dependency('girara-gtk3', fallback: ['girara', 'girara_dependency'])
glib = dependency('glib-2.0')
poppler = dependency('poppler-glib', version: '>=0.18')

build_dependencies = [zathura, girara, glib, poppler]

if get_option('plugindir') == ''
  if zathura.type_name() == 'pkgconfig'
    plugindir = zathura.get_variable(pkgconfig: 'plugindir')
  else
    zathura_subproject = subproject('zathura')
    plugindir = zathura_subproject.get_variable('plugindir')
  endif
else
  plugindir = get_option('plugindir')
endif

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
  '-fvisibility=hidden'
]
flags = cc.get_supported_arguments(flags)

sources = files(
  'zathura-pdf-poppler/attachments.c',
  'zathura-pdf-poppler/document.c',
  'zathura-pdf-poppler/forms.c',
  'zathura-pdf-poppler/image.c',
  'zathura-pdf-poppler/index.c',
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
