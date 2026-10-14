# Pigeon Engine

A Linux-native **C++23 / OpenGL 4.1** engine with a live scene editor and a
screen-space dialogue overlay.

You compose a 3D scene in the same process you play it: cubes, models, images,
particles, water, a skybox. Dialogue is not ImGui. It is a charcoal panel drawn
in pixels over the viewport — speaker, body, portrait, location background —
authored as objects and played back with a typewriter.

## What it is

Pigeon Engine is a small engine, not a framework dump. One executable owns:

| Layer | Role |
| --- | --- |
| 3D viewport | OpenGL scene, free-fly camera, grid, skybox |
| Editor | Dear ImGui docking: create, transform, browse, save, play |
| Overlay | `DialogueSystem` + `FontManager`: visual-novel panel and FONT objects |

The loop is short on purpose:

1. Place meshes and images in the world.
2. Author dialogue lines (speaker, copy, portrait, optional background).
3. Hit **Play Scene** and watch sequence, typewriter, hold, and portrait bob.

Game-facing UI stays in OpenGL overlay code. ImGui is editor chrome only.

## Status

| Area | State |
| --- | --- |
| Scene editor (create / edit / delete) | Done |
| JSON scene save / load (`Scenes/`) | Done |
| Assimp models + stb_image textures | Done |
| Native Linux file picker (XDG portal) | Done |
| Dialogue overlay, portraits, backgrounds | Done |
| Typewriter playback + speaking order | Done |
| Material / lighting editor | In progress |
| Choice UI, flags, story graph | Specified, not built |

## Features

### Editor

- Create **Particle, Cube, Model, Image, Water, Dialogue, Font** objects while the scene runs.
- World transforms for 3D objects; screen-space XY for dialogue and font overlays.
- Colour, transparency, shininess, reflectivity, light intensity.
- Named scenes written as JSON under `Scenes/`, including dialogue fields and a scene background path.
- Debug dock: FPS, frame time, camera, object counts.
- **Play Scene / Stop** walks dialogue objects in sequence order.

### Dialogue overlay

- Printed-matter panel: fill `#22272B`, hairline border, contact shadow, 2px speaker accent bar.
- **IBM Plex Sans** for readable body text; **Irish Grover** only for novelty `FONT` objects.
- Portraits (PNG/JPEG) on the left or right, overlap into the panel, optional speaking bob.
- Full-screen **location backgrounds**: scene default, plus per-line override with inherit.
- Typewriter, hold-after-line, pulsing continue tick when a line is fully revealed.
- Speaking Order list to reorder lines without changing object identity.

### Rendering and content

- OpenGL 4.1 core, GLAD, GLM transforms.
- Models via Assimp: OBJ, glTF, GLB, FBX (Normal / Rainbow / Light, wireframe).
- Textures via stb_image (PNG, JPEG, BMP, TGA, GIF, HDR, and related formats).
- Particle emitters in the editor, plus a multi-effect system (basic, noise, colour cycle).
- Water grid, skybox cubemap, voxel / Voronoi experiments under `src/procedural/`.
- Skeletal animation types exist (`animation`, `animator`, `bone`) for animated models.

### Linux file picker

Browse uses the **XDG Desktop Portal FileChooser** over the session bus (`libsystemd` sd-bus).
No Qt, no GTK, no in-engine file explorer. The editor keeps rendering while the
dialog is open; cancel leaves the existing path unchanged. Image filters include
PNG, JPEG, WEBP, BMP, TGA, GIF, HDR, and other engine-supported types. The
desktop chooser can show thumbnails when the DE supports it.

## Quick start

```sh
cmake -S . -B build
cmake --build build -j
./build/PigeonEngine
```

Run from `build/` or the repo root. CMake sets `PIGEON_ASSET_DIR` to `assets/`.

### Dependencies (Linux)

- C++23 compiler, CMake 3.10+, pkg-config
- OpenGL, GLFW 3 (system or vendored), Assimp, FreeType
- **libsystemd** (portal FileChooser)
- nlohmann/json (fetched by CMake if missing)

After adding files under `src/`, re-run CMake so the recursive source glob refreshes.

## Controls

| Input | Action |
| --- | --- |
| W A S D | Move camera |
| Mouse | Look (when captured) |
| Scroll | Zoom |
| Left Shift | Faster move |
| F | Release cursor for the editor |
| G | Recapture cursor |
| Esc | Quit |

Press **F** before clicking ImGui. **G** puts you back in the viewport.

## Project layout

```
src/app/          entry (main.cpp)
src/core/         Application, callbacks, paths, camera input
src/rendering/    shader helpers, primitives, textures
src/models/       Assimp / editor model manager
src/dialogue/     overlay playback and JSON
src/ui/           ImGui editor, portal picker, FontManager
src/particles/    particle systems
src/procedural/   voxels, Voronoi
src/effects/      standalone effects
include/<same>/   public headers
assets/           shaders, textures, fonts, models, cubemap
docs/             engine notes (Obsidian-friendly hub included)
```

`Application::Run()` owns the window, GL, ImGui, update, and render.
`CreationManager` is the object editor. `DialogueSystem::Render` draws after
the 3D pass and before ImGui.

## Documentation

- [Pigeon Engine](docs/Pigeon%20Engine.md) — full hub (architecture, editor, dialogue, palette). Written for Obsidian; still readable on GitHub.
- [Engine Overview](docs/ENGINE_OVERVIEW.md) — runtime flow, model texture slots, asset guidelines.
- [Folder System](docs/FOLDER_SYSTEM.md) — where new source, headers, and assets go.
- [Dialogue Story System](docs/DIALOGUE_STORY_SYSTEM.md) — planned graph (choices, flags). Sequential play in `DialogueSystem` stays as it is until that lands.

## Stack

| Layer | Choice |
| --- | --- |
| Language | C++23 |
| Window / input | GLFW |
| GL loader | GLAD (OpenGL 4.1 core) |
| Math | GLM |
| Editor UI | Dear ImGui (docking) |
| Models | Assimp |
| Images | stb_image |
| Fonts | FreeType |
| JSON | nlohmann/json |
| File picker | XDG Desktop Portal + sd-bus |
| Build | CMake |

## License

See [LICENSE](LICENSE). Fonts under `assets/fonts/` keep their own OFL / Apache files.
