# third_party/imgui/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

# Project name and language
project(imgui LANGUAGES CXX)

# Collect all ImGui source files
set(IMGUI_SOURCES
    imgui.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    imgui_demo.cpp   # Optional: Include if you want the ImGui demo window
)

# Create the ImGui library
add_library(imgui STATIC ${IMGUI_SOURCES})

# Specify the include directories for ImGui
target_include_directories(imgui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Specify the C++ standard (ImGui requires at least C++11)
target_compile_features(imgui PUBLIC cxx_std_11)

# Optionally set properties, such as position-independent code
set_target_properties(imgui PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# Optionally set compiler warnings
if (MSVC)
    target_compile_options(imgui PRIVATE /W4 /WX)
else()
    target_compile_options(imgui PRIVATE -Wall -Wextra -Wpedantic)
endif()
