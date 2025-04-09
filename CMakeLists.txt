cmake_minimum_required(VERSION 3.10)
project(PigeonEngine VERSION 1.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g")

file(GLOB_RECURSE SRC_FILES
    src/*.cpp
)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
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

# ADD TINYFILEDIALOGS LIBRARY HERE
add_library(tinyfiledialogs STATIC ${CMAKE_SOURCE_DIR}/src/tinyfiledialogs.c)
target_include_directories(tinyfiledialogs PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Add the main executable
add_executable(${PROJECT_NAME} ${SRC_FILES})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    tinyfiledialogs    
    glad
    imgui
    ${GLFW_LIB}
    OpenGL::GL
    assimp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external/glad/include
    ${CMAKE_SOURCE_DIR}/external/imgui-docking/imgui
    ${CMAKE_SOURCE_DIR}/external/imgui-docking/imgui/backends
    ${ASSIMP_INCLUDE_DIRS}
)
