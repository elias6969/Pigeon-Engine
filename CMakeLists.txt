cmake_minimum_required(VERSION 3.10)
project(PigeonEngine VERSION 1.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(OpenGL_GL_PREFERENCE GLVND)

file(GLOB_RECURSE SRC_FILES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

# Engine include directories are organized by responsibility. The category
# folders are kept on the include path so existing includes like "Shader.h"
# continue to work while the files live in clearer folders.
set(ENGINE_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/animation
    ${CMAKE_SOURCE_DIR}/include/core
    ${CMAKE_SOURCE_DIR}/include/dialogue
    ${CMAKE_SOURCE_DIR}/include/effects
    ${CMAKE_SOURCE_DIR}/include/models
    ${CMAKE_SOURCE_DIR}/include/particles
    ${CMAKE_SOURCE_DIR}/include/procedural
    ${CMAKE_SOURCE_DIR}/include/rendering
    ${CMAKE_SOURCE_DIR}/include/ui
)

# Include directories
include_directories(${ENGINE_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/external)
include_directories(${CMAKE_SOURCE_DIR}/external/glad/include)
include_directories(${CMAKE_SOURCE_DIR}/external/imgui-docking/imgui)
include_directories(${CMAKE_SOURCE_DIR}/external/imgui-docking/imgui/backends)

# GLAD library
add_library(glad STATIC ${CMAKE_SOURCE_DIR}/external/glad/src/glad.c)

# ImGui library
file(GLOB IMGUI_SOURCES
    ${CMAKE_SOURCE_DIR}/external/imgui-docking/imgui/*.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui-docking/imgui/backends/imgui_impl_glfw.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui-docking/imgui/backends/imgui_impl_opengl3.cpp
)
add_library(imgui STATIC ${IMGUI_SOURCES})

find_package(OpenGL REQUIRED)
find_package(assimp REQUIRED)

# JSON serialization used by the editor scene/object save-load code.
find_package(nlohmann_json CONFIG QUIET)
if(NOT nlohmann_json_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        nlohmann_json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

# Find or build GLFW
find_package(glfw3 QUIET)
if(NOT glfw3_FOUND)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory(${CMAKE_SOURCE_DIR}/external/glfw)
    set(GLFW_LIB glfw)
else()
    set(GLFW_LIB glfw)
endif()

# Add the main executable
add_executable(${PROJECT_NAME} ${SRC_FILES})
target_compile_definitions(${PROJECT_NAME} PRIVATE
    PIGEON_ASSET_DIR="${CMAKE_SOURCE_DIR}/assets"
)


find_package(Freetype REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSYSTEMD REQUIRED IMPORTED_TARGET libsystemd)

target_link_libraries(PigeonEngine PRIVATE
    Freetype::Freetype
    PkgConfig::LIBSYSTEMD
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    glad
    imgui
    ${GLFW_LIB}
    OpenGL::GL
    assimp
    nlohmann_json::nlohmann_json
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${ENGINE_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/external/glad/include
    ${CMAKE_SOURCE_DIR}/external/imgui-docking/imgui
    ${CMAKE_SOURCE_DIR}/external/imgui-docking/imgui/backends
    ${ASSIMP_INCLUDE_DIRS}
)
