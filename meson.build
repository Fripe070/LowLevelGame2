project('lowlevelgame', 'cpp',
        default_options : [
            'cpp_std=c++23',
            'default_library=static',
            'warning_level=3',
        ])

sdl2_dep = dependency('sdl2')
imgui_dep = dependency('imgui')
glew_dep = dependency('glew')
glm_dep = dependency('glm')

assimp_dep = dependency('assimp', method: 'pkg-config', required: false)  # required: false is to make meson not die when it can't find assimp
if not assimp_dep.found()
    cmake = import('cmake')
    assimp_opt = cmake.subproject_options()
    assimp_opt.add_cmake_defines({
        'BUILD_SHARED_LIBS' : false,
        'CMAKE_BUILD_TYPE' : get_option('buildtype') == 'debug' ? 'Debug' : 'Release',
        'ASSIMP_BUILD_TESTS' : false,
        'ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT' : false,  # We won't be exporting
        'ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT' : false,  # There are like a million of them

        'ASSIMP_BUILD_OBJ_IMPORTER' : true,
    })
    assimp_dep = cmake.subproject('assimp', options : assimp_opt).dependency('assimp')
endif

dependencies = [sdl2_dep, imgui_dep, glew_dep, glm_dep, assimp_dep]

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
    'src/scene_loader.cpp',
    'src/camera.cpp',
]

exe = executable(
    'lowlevelgame', sources,
    # win_subsystem: 'windows',  # Supresses stdout on Windows
    include_directories : include_directories('src', 'include'),
    dependencies : dependencies,
    cpp_args : ['-std=c++23']
)

test('basic', exe)
