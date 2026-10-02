# Engine Overview

This project is organized as a small OpenGL engine with editor-facing systems in
`src/` and their public interfaces in `include/`.

## Core Layout

- `src/app/main.cpp` is intentionally tiny: it creates `Application` and calls
  `Application::Run()`.
- `src/core/Application.cpp` owns the engine lifecycle: window creation, OpenGL
  setup, resource initialization, update, render, ImGui, and shutdown.
- `src/core/Callbacks.cpp` owns GLFW callbacks and frame input routing.
- `include/` contains the engine headers used across systems. Keep shared types
  in the matching category folder when more than one source file needs them.
- `src/ui/` contains the ImGui editor panels, project scene save/load actions,
  and the ImGui-based asset file browser.
- `src/models/` and `include/models/` handle model loading and model rendering
  state.
- `include/models/Model.h` imports model files through Assimp and converts them
  into engine `Mesh` objects.
- `include/models/mesh.h` owns GPU buffer setup and mesh draw calls.
- `include/rendering/` stores render primitives, shader helpers, texture helpers,
  and OpenGL diagnostics.
- `assets/Shaders/` stores GLSL programs.
- `assets/models/` stores model assets and their texture folders.
- `external/` stores vendored third-party code. Avoid engine changes in this
  folder unless updating a dependency.
- `docs/FOLDER_SYSTEM.md` is the detailed source/header placement guide.

## Runtime Flow

The executable starts in `src/app/main.cpp`:

```cpp
#include "Application.h"

int main() {
  Application app;
  return app.Run();
}
```

`Application::Run()` calls:

- `Init()` for GLFW, GLAD, engine resources, and ImGui.
- `Update()` once per frame for timing and input.
- `Render()` once per frame for ImGui, scene rendering, and swap/poll.
- `Shutdown()` after the window closes.

Use `Application` for top-level engine systems. Use `Callbacks` for GLFW input
and resize callbacks.

## Editor Debug Panel

The `Pigeon Engine Debug` ImGui panel shows:

- FPS, frame time, and delta time.
- Window and framebuffer size.
- Mouse position and cursor capture state.
- Camera position, front vector, yaw, pitch, zoom, and speed.
- Editor scene counts for objects, cubes, images, particles, models, and water
  grids.

Asset browsing uses the vendored ImGui file browser, so it works on Arch/Hyprland
without requiring `zenity`, `kdialog`, or other native dialog tools.

## 3D Model Loading

Models are loaded with Assimp and converted into one or more `Mesh` objects.
The loader currently expects:

- Triangulated mesh data, generated automatically during import.
- UVs in the first texture coordinate channel.
- Optional normals, tangents, bitangents, and bone weights.
- Textures either embedded in the model file or stored beside the model using
  paths from the source asset/material file.

Supported texture slots map to shader uniform names used by `Mesh::Draw()`:

- Diffuse maps: `texture_diffuse1`, `texture_diffuse2`, ...
- Specular maps: `texture_specular1`, `texture_specular2`, ...
- Normal maps: `texture_normal1`, `texture_normal2`, ...
- Height maps: `texture_height1`, `texture_height2`, ...

When adding new model shaders, keep those uniform names unless you also update
`Mesh::Draw()`.

## Asset Guidelines

- Prefer `glTF`/`GLB` for new animated or PBR-ready assets.
- Prefer `OBJ` for simple static meshes.
- Keep textures in the same folder as the model, or use embedded textures for
  portable assets.
- Runtime asset paths are rooted through `PIGEON_ASSET_DIR`, which CMake sets to
  the project `assets/` folder. The engine should run from either the project
  root or the `build/` directory.
- If a model looks black or flat, verify the shader supports the texture slots
  the model is using and that lighting uniforms are being set.
- If animation does not move, load the model with `animated = true` and make sure
  the animation file uses the same skeleton names as the mesh.

## Organization Rules

- Keep file movement small and intentional. Follow `docs/FOLDER_SYSTEM.md` when
  adding new source, header, asset, or documentation files.
- Put rendering data ownership close to the class that uploads or draws it.
- Keep editor-specific state in manager classes, not in low-level mesh/model
  types.
- Document new public headers with the asset or render assumptions they depend
  on.
