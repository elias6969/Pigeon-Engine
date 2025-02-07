#PigeonEngine

PigeonEngine is a **lightweight** and **modular** graphics engine built for **Linux**, primarily **Ubuntu**. It provides a **custom editor** that allows users to create and manipulate objects easily by specifying file paths for **textures** and **3D models**. The engine is currently in development, with ongoing improvements, including a **real-time text editor** for adjusting lighting and materials.

PigeonEngine is designed to combine elements of **Blender** and **Unity**, but with a streamlined workflow and performance-focused design.

---

## 🚀 Features

- **Modern OpenGL Rendering** – Optimized real-time rendering.
- **Custom Engine Editor** – Create, manage, and manipulate objects in an intuitive UI.
- **Real-Time Asset Loading** – Import **2D textures** and **3D models** by providing a file path.
- **Lighting & Material Editing** – *(In Development)* Real-time modifications to lighting and material properties.
- **Cross-Platform Codebase** – Primarily supports **Ubuntu**, but designed for future portability.
- **Mathematics & Transformations** – Uses **GLM** for efficient vector and matrix operations.

---

## 📦 Dependencies

Before building PigeonEngine, ensure the following dependencies are installed:

- [**Assimp**](https://github.com/assimp/assimp) – Model file loading and parsing.
- [**GLM**](https://github.com/g-truc/glm) – Mathematics library for transformations.
- [**GLFW**](https://www.glfw.org/) – Windowing and input handling.
- [**GLAD**](https://github.com/Dav1dde/glad) – OpenGL function loader.
- [**stb_image**](https://github.com/nothings/stb) – Image loading for textures.
- **CMake** (for building the project).
- **C++17** or newer compiler.

---

## 🔧 Installation

### 1️⃣ Install Dependencies (Ubuntu)
```sh
sudo apt update
sudo apt install cmake g++ libassimp-dev libglfw3-dev

Other dependencies (GLM, GLAD, stb_image) can be included directly within the project if not available via package manager.
2️⃣ Clone the Repository

git clone https://github.com/your-username/PigeonEngine.git
cd PigeonEngine

3️⃣ Build the Engine

mkdir build && cd build
cmake ..
make

4️⃣ Run PigeonEngine

./bin/PigeonEngine

This will launch the engine editor where you can create and manipulate objects in a 3D environment.
🎮 Usage
🖼️ Loading Models and Textures

To create objects, simply provide the file path to an image or 3D model in the editor. Supported formats include:

    Models: .obj, .fbx, .dae, .gltf, etc. (via Assimp).
    Textures: .png, .jpg, .bmp, etc. (handled by stb_image).

🔄 Transformations & Object Manipulation

    Move, rotate, and scale objects within the editor.
    Modify lighting and materials (real-time editing feature in progress).

📝 Example Code

A minimal example of setting up a PigeonEngine window:

#include <PigeonEngine.h>

int main() {
    PigeonEngine::Initialize();  

    while (!PigeonEngine::ShouldClose()) {
        PigeonEngine::PollEvents();

        PigeonEngine::BeginRender();
        // Draw objects here...
        PigeonEngine::EndRender();
    }

    PigeonEngine::Shutdown();
    return 0;
}

🛠️ Development Roadmap
✅ Short-Term Goals

Custom-built editor with basic scene management.
Object creation from file paths (models/textures).
Real-time material and lighting editor (In Progress).
Improved UI/UX for object manipulation.

    Scene-saving and serialization.

🔮 Long-Term Vision

    Advanced Material System – Node-based material editing.
    Physics Integration – Basic collision detection and physics.
    Animation System – Skeletal animation and keyframe-based interpolation.
    Expanded Platform Support – Compatibility testing for other Linux distros.

🤝 Contributing

Contributions are welcome! If you'd like to help improve PigeonEngine:

    Fork the Repository.
    Open an Issue – Report bugs or request features.
    Submit a Pull Request – Contributions to performance, rendering, or editor features are highly valued.

For major contributions, please create an issue first to discuss the implementation plan.
📜 License

PigeonEngine is released under the MIT License, allowing free use and modification. See the LICENSE file for details.
🎉 Acknowledgments

    OpenGL Community – For resources, tutorials, and continuous improvements in rendering.
    Third-Party Libraries – Assimp, GLM, GLFW, GLAD, stb_image for enabling essential functionality.

For any questions or feedback, feel free to open an issue or contribute to the repository. 🚀


---

### ✅ **GitHub-Optimized Markdown:**
- **Properly formatted** code blocks (`sh` for shell, `cpp` for C++ for syntax highlighting).
- **Headers, lists, and emojis** to improve readability.
- **Bullet points & checklists** for development progress tracking.
- **Links to dependencies** for easy access.

This **README.md** will look clean and professional on GitHub. You can copy-paste it directly into your repository. Let me know if you need any adjustments! 🚀
