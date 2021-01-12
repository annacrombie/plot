project(
	'plot',
	'c',
	version: '0.3.0',
	license: 'GPL-3.0-only',
	meson_version: '>=0.44.1',
	default_options: [
		'c_std=c99',
		'warning_level=2',
	],
)


cc = meson.get_compiler('c')

add_project_arguments(cc.get_supported_arguments([
	'-Wundef',
	'-Wlogical-op',
	'-Wmissing-include-dirs',
	'-Wold-style-definition',
	'-Wpointer-arith',
	'-Winit-self',
	'-Wstrict-prototypes',
	'-Wimplicit-fallthrough=2',
	'-Wendif-labels',
	'-Wstrict-aliasing=2',
	'-Woverflow',

	'-Wno-missing-braces',
	'-Wno-missing-field-initializers',
	'-Wno-unused-parameter',
]), language: 'c')

deps = [
	cc.find_library('m', required : true),
]

src = files([
	'src/display.c',
	'src/follow.c',
	'src/input.c',
	'src/main.c',
	'src/opts.c',
	'src/plot.c',
	'src/util.c',
])

executable('plot', sources: src, dependencies: deps, install: true)

if get_option('build_docs') != 'disabled'
	subdir('docs')
endif
