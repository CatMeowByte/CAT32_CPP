project('CAT-32', 'cpp',
 default_options: [
  'cpp_std=c++11'
 ]
)

sources = run_command('find', '.', '-name', '*.cpp', '!', '-path', './build/*', check: true).stdout().strip().split('\n')

executable('CAT-32', sources,
 include_directories: include_directories('.'),
 dependencies: dependency('sdl3')
)