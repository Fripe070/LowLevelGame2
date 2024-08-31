project('lowlevelgame', 'cpp',
        default_options: ['default_library=static', 'warning_level=3'])

sdl2_dep = dependency('sdl2')
imgui_dep = dependency('imgui')
dependencies = [sdl2_dep, imgui_dep]

if host_machine.system() == 'windows'
  sdl2_main_dep = dependency('sdl2main')
  dependencies += sdl2_main_dep
endif

exe = executable('lowlevelgame', 'src/main.cpp',
           win_subsystem: 'windows',
           dependencies: dependencies)

test('basic', exe)
