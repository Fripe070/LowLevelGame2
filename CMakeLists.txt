cmake_minimum_required(VERSION 3.31)
project(lowlevelgame
    LANGUAGES CXX
)

# TODO: Linting warnings and strict fail if found in my own code?

set(CMAKE_CXX_STANDARD 23)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "" FORCE)

include(FetchContent)

FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.32.8
    GIT_PROGRESS ON GIT_SHALLOW
)
FetchContent_MakeAvailable(SDL2)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_PROGRESS ON GIT_SHALLOW
)
FetchContent_MakeAvailable(glm)

#set(GLEW_STATIC ON)
set(glew-cmake_BUILD_SHARED OFF CACHE INTERNAL "" FORCE)
set(glew-cmake_BUILD_STATIC ON CACHE INTERNAL "" FORCE)
set(glew-ONLY_LIBS ON CACHE INTERNAL "" FORCE)
FetchContent_Declare(
    glew
    GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
    #    GIT_REPOSITORY https://github.com/nigels-com/glew.git
    #    GIT_TAG glew-2.2.0
    GIT_PROGRESS ON GIT_SHALLOW
)
FetchContent_MakeAvailable(glew)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.6-docking
    GIT_PROGRESS ON GIT_SHALLOW
)
FetchContent_MakeAvailable(imgui)
add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)
# Link imgui with SDL2 and OpenGL
target_link_libraries(imgui PUBLIC SDL2::SDL2 libglew_static)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.15.1
    GIT_PROGRESS ON GIT_SHALLOW
)
FetchContent_MakeAvailable(spdlog)

set(ASSIMP_INSTALL                          OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS                      OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT   OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT   OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER               ON CACHE BOOL "" FORCE)
FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG v5.2.5
    GIT_PROGRESS ON GIT_SHALLOW
)
FetchContent_MakeAvailable(assimp)

set(TINYGLTF_USE_CPP14   ON  CACHE INTERNAL "" FORCE)
set(TINYGLTF_HEADER_ONLY ON  CACHE INTERNAL "" FORCE)
set(TINYGLTF_INSTALL     OFF CACHE INTERNAL "" FORCE)
FetchContent_Declare(
    tinygltf
    GIT_REPOSITORY https://github.com/syoyo/tinygltf.git
    GIT_TAG v2.9.6
    GIT_PROGRESS ON GIT_SHALLOW
)
FetchContent_MakeAvailable(tinygltf)

#region Generate C++ header files from static resources

# Add a custom command to generate the header files from the static resources
set(BINS2H_INPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources/static")
set(BINS2H_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/static_resources")
file(MAKE_DIRECTORY ${BINS2H_OUTPUT_DIR})

file(GLOB BINARY_FILES "${BINS2H_INPUT_DIR}/*")
set(GENERATED_HEADERS "")
foreach(BINARY_FILE ${BINARY_FILES})
    get_filename_component(FILENAME ${BINARY_FILE} NAME) # Gets the filename without extension
    list(APPEND GENERATED_HEADERS "${BINS2H_OUTPUT_DIR}/${FILENAME}.h")
endforeach()

add_executable(bins2h tools/bins2h.cpp)
add_custom_command(
    OUTPUT ${GENERATED_HEADERS}
    COMMAND bins2h "${BINS2H_INPUT_DIR}" "${BINS2H_OUTPUT_DIR}/"
    DEPENDS bins2h ${BINARY_FILES}
    COMMENT "Generating C++ header files from static resources"
)
add_custom_target(GenerateBinaryHeaders ALL DEPENDS ${GENERATED_HEADERS})

#endregion


set(LLG2_SOURCES
    src/main.cpp
    src/engine/run.cpp
    src/engine/util/logging.cpp
    src/engine/util/file.cpp
    src/engine/resources/shader.cpp
    src/engine/resources/texture.cpp
    src/engine/resources/scene.cpp
    src/engine/resources/mesh.cpp
    src/engine/render/overlay.cpp
    src/engine/render/frame_buffer.cpp
    src/engine/resources/resource_manager.cpp

    src/game/game.cpp
    src/game/camera_utils.cpp
    src/game/player.cpp
    src/game/skybox.cpp
    src/game/gui.cpp
)

add_executable(lowlevelgame)

target_sources(lowlevelgame PRIVATE ${LLG2_SOURCES})

target_include_directories(lowlevelgame PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_include_directories(lowlevelgame PRIVATE ${assimp_SOURCE_DIR}/include)

# Depend on and include the generated binary headers
add_dependencies(lowlevelgame GenerateBinaryHeaders)
target_include_directories(lowlevelgame PRIVATE ${BINS2H_OUTPUT_DIR})

target_link_libraries(lowlevelgame
    PUBLIC
    SDL2::SDL2
    imgui
    glm::glm
    libglew_static
    spdlog::spdlog
    assimp
)
if(WIN32)
    target_link_libraries(lowlevelgame PRIVATE SDL2::SDL2main)
endif()
