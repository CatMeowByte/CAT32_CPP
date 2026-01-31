project(
 'CAT-32',
 'cpp',
 default_options: ['cpp_std=c++11'],
)

sources = [
# kernel
'CAT32.cpp',

# core
'core/interpreter.cpp',
'core/memory.cpp',
'core/module.cpp',
'core/opcode.cpp',
'core/utility.cpp',

# library
'library/sdl.cpp',

# module
'module/audio.cpp',
'module/button.cpp',
'module/filesystem.cpp',
'module/string.cpp',
'module/video.cpp',
]

executable(
 'CAT-32',
 sources,
 include_directories: include_directories('.'),
 dependencies: dependency('sdl3'),
)