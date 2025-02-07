# PigeonEngine

PigeonEngine is a lightweight **graphics engine** for **Linux** (primarily Ubuntu), designed for **real-time 3D rendering**. It features a **custom editor** that allows users to create and manipulate objects using simple file paths for textures and 3D models. 

The engine is currently under active development, with upcoming features including **a built-in text editor for real-time lighting and material adjustments**. PigeonEngine aims to combine the best aspects of **Blender** and **Unity**, offering a powerful yet streamlined experience.

---

## Features

- **Custom Engine Editor** – Create and modify objects in an interactive UI.
- **Real-Time Asset Loading** – Import **textures** and **3D models** via file paths.
- **Modern OpenGL Rendering** – Efficient and optimized rendering pipeline.
- **Lighting & Material Editing** *(In Progress)* – Real-time adjustments inside the engine.
- **Cross-Platform Codebase** – Designed with portability in mind (Ubuntu-first focus).
- **Mathematics & Transformations** – Powered by **GLM** for vector/matrix operations.

---

## Dependencies

Before building PigeonEngine, ensure you have the following installed:

- [**Assimp**](https://github.com/assimp/assimp) – 3D model loading.
- [**GLM**](https://github.com/g-truc/glm) – Mathematics library.
- [**GLFW**](https://www.glfw.org/) – Window management and input handling.
- [**GLAD**](https://github.com/Dav1dde/glad) – OpenGL function loader.
- [**stb_image**](https://github.com/nothings/stb) – Image loading.
- **CMake** – For building the project.
- **C++17** or newer compiler.

---

## Installation

### 1. Install Dependencies (Ubuntu)
```sh
sudo apt update
sudo apt install cmake g++ libassimp-dev libglfw3-dev

