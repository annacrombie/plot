project(
	'plot',
	'c',
	version: '0.4.0',
	license: 'GPL-3.0-only',
	meson_version: '>=0.44.1',
	default_options: [
		'c_std=c99',
		'warning_level=3',
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

git = find_program('git', required: false)
if git.found()
	git_rev_parse = run_command(git, 'rev-parse', '--short', '@')
	git_sha = git_rev_parse.stdout().strip()
else
	git_sha = 'unknown'
endif

version_info = configuration_data()
version_info.set('version', meson.project_version())
version_info.set('vcs_tag', git_sha)

libm = cc.find_library('m', required: true)
install_headers()

subdir('include')
subdir('lib')
subdir('cli')
subdir('examples')

if get_option('test')
	subdir('tests')
endif

if get_option('build_docs') != 'disabled'
	subdir('docs')
endif
