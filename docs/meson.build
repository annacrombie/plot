scdoc_required = get_option('build_docs') == 'enabled'
scdoc = dependency(
	'scdoc',
	native: true,
	required: scdoc_required,
	version: '>=1.9.0'
)

if scdoc.found()
	scdoc_prog = find_program(
		scdoc.get_pkgconfig_variable('scdoc'),
		native: true
	)
	sh = find_program('sh', native: true)
	mandir = get_option('mandir')

	custom_target(
		'plot.1',
		input: 'plot.1.scd',
		output: 'plot.1',
		command: [sh, '-c', scdoc_prog.path() + ' < @INPUT@'],
		capture: true,
		install: true,
		install_dir: '@0@/man1'.format(mandir)
	)
endif
