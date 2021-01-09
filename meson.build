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

scdoc = dependency('scdoc', native: true, required: false, version: '>=1.9.0')

if scdoc.found()
	scdoc_prog = find_program(scdoc.get_pkgconfig_variable('scdoc'), native: true)
	sh = find_program('sh', native: true)
	mandir = get_option('mandir')

	custom_target(
		'plot.1',
		input: 'docs/plot.1.scd',
		output: 'plot.1',
		command: [
			sh, '-c', '@0@ < @INPUT@ > @1@'.format(
				scdoc_prog.path(),
				join_paths(meson.current_build_dir(), 'plot.1'))
		],
		install: true,
		install_dir: '@0@/man@1@'.format(mandir, 1)
	)
else
	warning('scdoc not found, no man pages will be installed')
endif
