project('lowlevelgame', 'cpp',
        default_options : [
            'cpp_std=c++23',
            'default_library=static',
            'warning_level=3',
            'b_ndebug=if-release',
        ])

sdl2_dep = dependency('sdl2')
glew_dep = dependency('glew')
glm_dep = dependency('glm')
imgui_dep = dependency('imgui_docking')

assimp_dep = dependency('assimp', method: 'pkg-config', required: false)  # required: false is to make meson not die when it can't find assimp
if not assimp_dep.found()
    cmake = import('cmake')
    assimp_opt = cmake.subproject_options()
    assimp_opt.add_cmake_defines({
        'BUILD_SHARED_LIBS' : false,
        'CMAKE_BUILD_TYPE' : get_option('buildtype') == 'release' ? 'Release' : 'Debug',
        'ASSIMP_BUILD_TESTS' : false,
        'ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT' : false,  # We won't be exporting
        'ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT' : false,  # There are like a million of them

        'ASSIMP_BUILD_OBJ_IMPORTER' : true,
    })
    assimp_dep = cmake.subproject('assimp', options : assimp_opt).dependency('assimp')
endif


spdlog_options = ['default_library=static', 'compile_library=true', 'werror=false', 'tests=disabled', 'external_fmt=disabled', 'std_format=disabled']
spdlog_dep = dependency('spdlog', default_options: spdlog_options)

dependencies = [sdl2_dep, glew_dep, glm_dep, imgui_dep, assimp_dep, spdlog_dep]

if host_machine.system() == 'windows'
    sdl2_main_dep = dependency('sdl2main')
    dependencies += [sdl2_main_dep]
endif

sources = [
    'src/main.cpp',
    'src/engine/run.cpp',
    'src/engine/util/logging.cpp',
    'src/engine/util/file.cpp',
    'src/engine/resources/shader.cpp',
    'src/engine/resources/texture.cpp',
    'src/engine/resources/scene.cpp',
    'src/engine/resources/mesh.cpp',
    'src/engine/render/overlay.cpp',
    'src/engine/render/frame_buffer.cpp',
    'src/engine/resources/resource_manager.cpp',

    'src/game/game.cpp',
    'src/game/camera_utils.cpp',
    'src/game/player.cpp',
    'src/game/skybox.cpp',
    'src/game/gui.cpp',
]


bin2h = executable('bin2h', 'tools/bin2h.cpp')
static_binaries = [
    ['resources/static/error.png', 'error_png'],
    ['resources/static/error.obj', 'error_obj'],
    ['resources/static/error_shader.vert', 'error_shader_vert'],
    ['resources/static/error_shader.frag', 'error_shader_frag'],
]
foreach binary : static_binaries
    sources += custom_target(
        'embed_binary_' + binary[1],
        input: binary[0],
        output: binary[1] + '.h',
        command: [bin2h, '@INPUT@', '@OUTPUT@'],
    )
endforeach


exe = executable(
    'lowlevelgame', sources,
    include_directories : include_directories('src', 'include'),
    dependencies : dependencies,
    cpp_args : ['-std=c++23']
)
test('basic', exe)
