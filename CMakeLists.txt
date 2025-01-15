cmake_minimum_required(VERSION 3.10)

# Project name and version
project(PigeonEngine VERSION 1.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)

# Add source files
file(GLOB_RECURSE SRC_FILES
    src/*.cpp
)

# Add include directory for your headers
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party)

# Include third_party directories
set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third_party)

include_directories(${THIRD_PARTY_DIR}/glad/include)
include_directories(${THIRD_PARTY_DIR}/imgui)
include_directories(${THIRD_PARTY_DIR}/imgui/backends)

# Add glad
add_library(glad STATIC ${THIRD_PARTY_DIR}/glad/src/glad.c)

# Add ImGui with backends
file(GLOB IMGUI_SOURCES
    ${THIRD_PARTY_DIR}/imgui/*.cpp
    ${THIRD_PARTY_DIR}/imgui/*.h
    ${THIRD_PARTY_DIR}/imgui/backends/imgui_impl_glfw.cpp
    ${THIRD_PARTY_DIR}/imgui/backends/imgui_impl_opengl3.cpp
)
add_library(imgui STATIC ${IMGUI_SOURCES})

# Find OpenGL
find_package(OpenGL REQUIRED)
find_package(assimp REQUIRED)
# Find GLFW (system-installed or in third_party)
find_package(glfw3 QUIET)
if(NOT glfw3_FOUND)
    # If not found, use GLFW from third_party
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

    add_subdirectory(${THIRD_PARTY_DIR}/glfw)
    set(GLFW_LIB glfw)
else()
    set(GLFW_LIB glfw)
endif()

# Add executable
add_executable(${PROJECT_NAME} ${SRC_FILES})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    glad
    imgui
    ${GLFW_LIB}
    OpenGL::GL
    assimp
)

# Add include directories for the target
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${THIRD_PARTY_DIR}/glad/include
    ${THIRD_PARTY_DIR}/imgui
    ${THIRD_PARTY_DIR}/imgui/backends
    ${ASSIMP_INCLUDE_DIRS} 
)
