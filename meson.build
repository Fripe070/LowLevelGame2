project('lowlevelgame', 'cpp',
        default_options: [
            'cpp_std=c++23',
            'default_library=static',
            'warning_level=3',
        ])

sdl2_dep = dependency('sdl2')
imgui_dep = dependency('imgui')
glew_dep = dependency('glew')
glm_dep = dependency('glm')

dependencies = [sdl2_dep, imgui_dep, glew_dep, glm_dep]

if host_machine.system() == 'windows'
  sdl2_main_dep = dependency('sdl2main')
  dependencies += [sdl2_main_dep]
endif

sources = [
    'src/main.cpp',
    'src/shader.cpp',
    'src/gui.cpp',
    'src/logging.cpp',
    'src/utility.cpp',
]

exe = executable(
    'lowlevelgame', sources,
    # win_subsystem: 'windows',  # Supresses stdout on Windows
    include_directories: include_directories('src', 'include'),
    dependencies: dependencies,
    cpp_args: ['-std=c++23']
)

test('basic', exe)
