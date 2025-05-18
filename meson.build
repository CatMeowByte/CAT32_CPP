project('test_sdl', 'cpp')

sources = run_command('find', '.', '-name', '*.cpp', '!', '-path', './build/*', check: true).stdout().strip().split('\n')

executable('CAT32', sources,
  include_directories: include_directories('.'),
  dependencies: dependency('sdl3')
)
