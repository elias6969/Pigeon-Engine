PigeonEngine

PigeonEngine is a lightweight, real-time graphics engine designed for Linux, primarily Ubuntu. It aims to provide an efficient and modular solution for rendering 2D/3D graphics, featuring a custom-built editor that simplifies object creation by allowing users to specify file paths for textures and models. The project is currently under development, with ongoing improvements such as a real-time text editor for adjusting lighting and materials.

PigeonEngine is envisioned as a hybrid between Blender and Unity, offering a streamlined workflow while maintaining powerful rendering capabilities.
Features

    Modern OpenGL Rendering – Efficient rendering pipeline optimized for real-time applications.
    Custom Engine Editor – Create, manage, and manipulate objects in a visual interface.
    Real-Time Asset Loading – Import 2D textures and 3D models by simply providing a file path.
    Lighting and Material Editing – (In Development) A built-in text editor to allow real-time modification of lighting and material properties.
    Cross-Platform Codebase – Primarily supports Ubuntu, but portability is considered for future expansions.
    Mathematics & Transformations – Uses GLM for efficient vector and matrix operations.

Dependencies

Before building PigeonEngine, ensure the following dependencies are installed:

    Assimp – Model file loading and parsing.
    GLM – Mathematics library for vector/matrix transformations.
    GLFW – Windowing and input management.
    GLAD – OpenGL function loader.
    stb_image – Image loading for textures.
    CMake (if building manually).
    C++17 or newer compiler.

Installation
1. Install Dependencies (Ubuntu)

sudo apt update
sudo apt install cmake g++ libassimp-dev libglfw3-dev

Other dependencies (GLM, GLAD, stb_image) can be included directly within the project if not available via package manager.
2. Clone the Repository

git clone https://github.com/your-username/PigeonEngine.git
cd PigeonEngine

3. Build the Engine

mkdir build && cd build
cmake ..
make

4. Run PigeonEngine

./bin/PigeonEngine

This will launch the engine editor where you can create and manipulate objects in a 3D environment.
Usage
Loading Models and Textures

To create objects, simply provide the file path to an image or 3D model in the editor. Supported formats include:

    Models: .obj, .fbx, .dae, .gltf, and more via Assimp.
    Textures: .png, .jpg, .bmp, etc., handled by stb_image.

Transformations & Object Manipulation

    Move, rotate, and scale objects within the editor.
    Modify lighting and materials (real-time editing feature in progress).

Development Roadmap
Short-Term Goals

✅ Custom-built editor with basic scene management.
✅ Object creation from file paths (models/textures).
🔲 Real-time material and lighting editor.
🔲 Improved UI/UX for object manipulation.
🔲 Scene-saving and serialization.
Long-Term Vision

    Advanced Material System – Node-based material editing.
    Physics Integration – Basic collision detection and physics support.
    Animation System – Skeletal animation and keyframe-based interpolation.
    Expanded Platform Support – Testing and compatibility for other Linux distributions.

Contributing

Contributions are welcome! If you’d like to help improve PigeonEngine:

    Fork the Repository
    Open an Issue – Report bugs or request features.
    Submit a Pull Request – Contributions to performance, rendering, or editor features are highly valued.

For major contributions, please create an issue first to discuss the implementation plan.
License

PigeonEngine is released under the MIT License, allowing free use and modification. See the LICENSE file for more details.
