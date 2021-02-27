src = files([
	'data_pipe.c',
	'data_proc.c',
	'display.c',
	'file_input.c',
	'log.c',
	'plot.c',
	'util.c',
]) + [configure_file(
	configuration: version_info,
	input: files('version.c.in'),
	output: 'version.c'
)]

libplot = library(
	'plot',
	sources: src,
	dependencies: [libm],
	include_directories: include,
	install: true
)
