project('lowlevelgame', 'cpp',
        default_options: ['default_library=static', 'warning_level=3'])

sdl2_dep = dependency('sdl2')
imgui_dep = dependency('imgui')
glew_dep = dependency('glew')
dependencies = [sdl2_dep, imgui_dep, glew_dep]

if host_machine.system() == 'windows'
  sdl2_main_dep = dependency('sdl2main')
  dependencies += [sdl2_main_dep]
endif

sources = [
    'src/main.cpp',
    'src/shader.cpp',
]

exe = executable('lowlevelgame', sources,
#                 win_subsystem: 'windows',  # Supresses stdout on Windows
                 include_directories: include_directories('src'),
                 dependencies: dependencies)

test('basic', exe)
