project('lowlevelgame', 'c',
        default_options: 'default_library=static')

sdl2_dep = dependency('sdl2')
  dependencies = [sdl2_dep]

if host_machine.system() == 'windows'
  sdl2_main_dep = dependency('sdl2main')
  dependencies += sdl2_main_dep
endif

exe = executable('lowlevelgame', 'lowlevelgame.c',
           win_subsystem: 'windows',
           dependencies: dependencies)

test('basic', exe)
