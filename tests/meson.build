foreach t: [
	'basic.sh',
	'follow.sh',
	'valgrind.sh',
]
	test(
		t,
		find_program(t),
		args: [plot],
		workdir: meson.current_build_dir()
	)
endforeach
