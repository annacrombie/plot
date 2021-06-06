simple_example = executable(
	'simple',
	files(['simple.c']),
	dependencies: [libm],
	link_with: [libplot],
	include_directories: include,
	install: false
)

ncurses = dependency('curses', required: false)

if ncurses.found()
	simple_curses_example = executable(
		'simple_curses',
		files(['simple_curses.c']),
		dependencies: [libm, ncurses],
		link_with: [libplot],
		include_directories: include,
		install: false
	)
endif
