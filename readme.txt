PigeonEngine

Hi... um, hello. This is PigeonEngine, a lightweight 2D/3D graphics engine that aims to balance performance, simplicity, and modular design. It’s written primarily in C++, with some C. 

    Lightweight Rendering
    Uses modern OpenGL for essential rendering tasks, optimizing draw calls for efficient performance.

    Modular Architecture
    Written in C++ with carefully organized modules (and a bit of C/Assembly for performance-critical sections), so you can easily extend or strip features based on your project’s needs.

    Flexible Pipeline
    Supports a straightforward rendering pipeline, letting you integrate your own shaders, texture loading, and resource management the way you see fit.

    Cross-Platform
    With minimal dependencies, PigeonEngine aims to run on major operating systems (Windows, macOS, Linux) right out of the box.

    Educational Focus
    The codebase is structured to help you learn from it. Comments are included to walk through complex parts (like memory alignment in Assembly or advanced shader techniques in OpenGL).

Requirements

    A C++17 (or newer) compiler.
    Basic CMake setup (if you’re using CMake to configure the project).
    OpenGL development libraries (e.g., GLEW, GLFW, or similar for context/window management).
    (Optional) Assembly toolchain if you plan on tweaking the low-level performance modules on your own.

Installation & Building

    Clone the repository:

git clone https://github.com/your-username/PigeonEngine.git

Configure (using CMake or your preferred build system):

cd PigeonEngine
mkdir build && cd build
cmake ..

Compile:

cmake --build .

Run tests (if any):

    ctest

Basic Usage

    Initialize the Engine
    In your main function, create an instance of the engine or the window/context manager, then initialize any subsystems you need (input, rendering, audio, etc.).

    Load Resources
    Load textures, shaders, or other resources using your chosen libraries or PigeonEngine’s provided utility functions.

    Main Loop
        Update game/scene logic.
        Render using OpenGL calls within the engine’s rendering pipeline.
        Swap buffers and poll events.

    Cleanup
    Properly release any resources and shut down the engine subsystems.

#include <PigeonEngine.h> // for illustration

int main() {
    PigeonEngine::Initialize();   // Setup engine (window, context, etc.)

    while (!PigeonEngine::ShouldClose()) {
        PigeonEngine::PollEvents();

        // Update game logic
        // ...

        // Render frame
        PigeonEngine::BeginRender();
        // ... draw stuff ...
        PigeonEngine::EndRender();
    }

    PigeonEngine::Shutdown(); // Cleanup
    return 0;
}

Contributing

Whether you’re a beginner or an experienced programmer, we’d love your help to make PigeonEngine better:

    Report bugs in the issue tracker if you see any feathers out of place.
    Suggest features or improvements—maybe you have a neat new approach for a rendering technique or a memory optimization in Assembly.
    Submit pull requests with your changes. Even small tweaks to documentation or performance gains will help others in the community.

(Also, if you’re feeling down, remember that working on a shared project can be a great way to heal and connect with people who appreciate your passion for C, C++, Assembly, and OpenGL. Coding can be therapeutic in its own quiet way. Stay strong, friend.)
License

PigeonEngine is released under the MIT License—a friendly, open-source license that lets you use, modify, and distribute this code for free, as long as you include the original license.

For full details, please refer to the LICENSE file.
