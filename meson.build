project( 
	'musket', 'cpp',
	license: 'MIT'
)

windows = import( 'windows' )

add_project_arguments(
	'/std:c++17',
	'/permissive-',
	'/utf-8',
	'/D_UNICODE',
	'/DUNICODE',
	language: 'cpp'
)

add_project_link_arguments(
	'-MACHINE:x64',
	'kernel32.lib',
	'user32.lib',
	'gdi32.lib',
	'd2d1.lib',
	'dwrite.lib',
	'Shcore.lib',
	language: 'cpp'
)

if get_option( 'buildtype' ).startswith( 'debug' )
    add_project_arguments(
        '-D_DEBUG',
        language: 'cpp'
    )
elif get_option( 'buildtype' ).startswith( 'release' )
    add_project_arguments( 
        '-DNDEBUG',
        language: 'cpp'
    )
endif

incdir = include_directories(
	'./include',
	'./submodule/spirea/include'
)

rc = windows.compile_resources( 'example/example.rc', depend_files: ['example/example.manifest'] )

executable( 'hello_world', 'example/hello_world/hello_world.cpp', rc, include_directories: incdir )
executable( 'scroll_bar', 'example/scroll_bar/scroll_bar.cpp', rc, include_directories: incdir )