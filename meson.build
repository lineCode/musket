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
	'/bigobj',
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

if get_option( 'buildtype' ).startswith( 'release' )
    add_project_arguments( 
        '-DNDEBUG',
        language: 'cpp'
    )
else
    add_project_arguments(
        '-D_DEBUG',
        language: 'cpp'
    )
endif

if get_option( 'build_examples' )
	subdir( 'example' )
endif
