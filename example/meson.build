incdir = include_directories(
	'../include',
	'../submodule/spirea/include'
)

example_rc = windows.compile_resources( 'example.rc', depend_files: ['example.manifest'] )

executable( 'hello_world', 'hello_world.cpp', example_rc, include_directories: incdir )
executable( 'scroll_bar', 'scroll_bar.cpp', example_rc, include_directories: incdir )
executable( 'attributes', 'attributes.cpp', example_rc, include_directories: incdir )