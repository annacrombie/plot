src = files([
	'main.c',
	'opts.c',
	'animate.c',
])

plot = executable(
	'plot',
	sources: src,
	dependencies: [libm],
	link_with: [libplot],
	include_directories: include,
	install: true
)
