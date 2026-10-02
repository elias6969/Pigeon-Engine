# Folder System

This project keeps engine code grouped by responsibility. Use this guide when
adding new files so the engine stays easy to navigate.

## Top-Level Folders

- `assets/` - runtime assets loaded by the engine.
- `assets/Shaders/` - GLSL shader files.
- `assets/models/` - 3D model files, model-specific textures, and model
  licenses.
- `assets/fonts/` - font files and emoji/font source material.
- `docs/` - engine documentation and contributor notes.
- `external/` - vendored third-party libraries. Do not place engine code here.
- `include/` - public engine headers, grouped by system.
- `src/` - engine implementation files, grouped by system.

## Source Folders

- `src/app/` - application entry points and top-level runtime wiring.
  - Current file: `main.cpp`
- `src/core/` - shared engine services and global runtime helpers.
  - Current files: `Application.cpp`, `Callbacks.cpp`, `filemanager.cpp`,
    `Variables.cpp`
- `src/rendering/` - render helpers, texture handling, geometry drawing, and
  OpenGL diagnostics.
  - Current files: `ObjectRenderer.cpp`, `textureManager.cpp`,
    `OpenGLerrorreporting.cpp`, `geoDrawer.cpp`, `BoundingBox.cpp`
- `src/models/` - 3D model loading/rendering managers.
  - Current file: `ModelManager.cpp`
- `src/animation/` - animation implementations when animation code moves out of
  headers.
- `src/ui/` - ImGui/editor code and UI file-dialog integration.
  - Current files: `ObjectGenerationIMGUI.cpp`, `tinyfiledialogs.c`
  - The active editor browser uses `external/imgui-docking/imgui/imfilebrowser.h`.
    `tinyfiledialogs` is kept as legacy/vendor code but is no longer linked into
    the main engine target.
- `src/particles/` - particle system implementations.
  - Current file: `ParticleManager.cpp`
- `src/procedural/` - voxel, Voronoi, terrain, water, and generated geometry
  experiments.
  - Current files: `Voronoi.cpp`, `VoxelChunk.cpp`, `testVoxel.cpp`
- `src/effects/` - visual effects that are not core renderer systems.
  - Current file: `bird.cpp`

## Header Folders

- `include/core/` - core engine headers.
  - `Application.h`, `Callbacks.h`, `Camera.h`, `Variables.h`, `Utils.h`,
    `WindowModule.h`, `filemanager.h`
- `include/rendering/` - renderer-facing headers and primitive render objects.
  - `Shader.h`, `textureManager.h`, `vao_manager.h`,
    `openglErrorReporting.h`, `Cube.h`, `Sphere.h`, `pyramid.h`, `Grid.h`,
    `SkyBox.h`, `Image.h`, `GeoManager.h`, `BoundingBox.h`, `ScreenEffect.h`
- `include/models/` - model import, mesh data, and Assimp helpers.
  - `Model.h`, `mesh.h`, `modelLoader.h`, `assimp_glm_helpers.h`
- `include/animation/` - skeletal animation and bone data.
  - `animation.h`, `animator.h`, `animdata.h`, `bone.h`
- `include/ui/` - editor UI and dialog headers.
  - `IMGUIManager.h`, `tinyfiledialogs.h`
- `include/particles/` - particle types and managers.
  - `Particle.h`, `ParticleManager.h`
- `include/procedural/` - generated/procedural world systems.
  - `Voronoi.h`, `Voxel.h`, `VoxelTest.h`
- `include/effects/` - standalone visual effects.
  - `bird.h`

## Where New Files Go

- New window, input, camera, path, or global configuration code goes in
  `core`.
- New top-level runtime orchestration belongs in `Application`, not in
  `src/app/main.cpp`.
- New GLFW callbacks or input routing belongs in `Callbacks`.
- New OpenGL draw helpers, primitives, materials, textures, or shader wrappers
  go in `rendering`.
- New model import, mesh, material, or 3D asset code goes in `models`.
- New skeletal animation, keyframe, bone, or animator code goes in `animation`.
- New ImGui panels, editor tools, or file-picker code goes in `ui`.
- New particles, emitters, particle materials, or simulation controls go in
  `particles`.
- New voxel, terrain, procedural mesh, water, Voronoi, or generation experiments
  go in `procedural`.
- New special visual effects that are not reusable renderer primitives go in
  `effects`.
- New GLSL files go in `assets/Shaders/`.
- New model assets go in `assets/models/<model-name>/`.
- New engine documentation goes in `docs/`.

## Build Notes

CMake adds each `include/<category>/` folder to the include path. That means
existing code can still include headers by filename, for example:

```cpp
#include "Shader.h"
#include "Model.h"
```

When adding new source files under `src/`, re-run CMake before building so the
recursive source glob refreshes:

```sh
cmake -S . -B build
cmake --build build
```
