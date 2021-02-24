src = files([
	'animate.c',
	'data_pipe.c',
	'data_proc.c',
	'display.c',
	'input.c',
	'log.c',
	'main.c',
	'opts.c',
	'plot.c',
	'util.c',
]) + [configure_file(
	configuration: version_info,
	input: files('version.c.in'),
	output: 'version.c'
)]

plot = executable(
	'plot',
	sources: src,
	dependencies: [libm],
	include_directories: include,
	install: true
)
