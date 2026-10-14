#include "IMGUIManager.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <glad/glad.h>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

#include <algorithm>
#include <chrono>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <stb_image.h>
#include <string>
#include <thread>
#include <vector>

#include "Cube.h"
#include "DialogueEditorUI.h"
#include "DialogueSystem.h"
#include "Grid.h"
#include "Image.h"
#include "Particle.h"
#include "Shader.h"
#include "SkyBox.h"
#include "Utils.h"
#include "WindowModule.h"
#include "fontManager.h"
#include "imstb_textedit.h"
#include "modelLoader.h"
#include "PortalFileChooser.h"

// Enum for Object Types
enum ObjectType {
  PARTICLE,
  CUBE,
  MODEL_OBJ,
  IMAGE,
  WATER_GRID,
  DIALOUGE,
  FONT
};

// Enum Modes
ModelRenderMode renderMode;

// Selected Object Type (default to CUBE)
ObjectType selectedType = CUBE;

// Collision Box
bool enableCollisionBox = false;
bool RenderParticle = true;

int amount = 0;
float particleSpeed;
float particleHeight;
float modelShaderHeight = 0.0f;
float modelMaxheight, modelminHeight;

// Object Transform
float objectSize[3] = {1.0f, 1.0f, 1.0f};
float objectRotation[3] = {0.0f, 0.0f, 0.0f};

// Object Color (RGB)
float objectColor[3] = {1.0f, 1.0f, 1.0f};

// Transparency (0.0 = Opaque, 1.0 = Transparent)
float objectTransparency = 0.0f;

// Material / Lighting
float objectShininess = 32.0f;
float objectReflectivity = 0.5f;
float lightIntensity = 1.0f;

// Physics
int selectedPhysicsType = 0; // Index into the physics options array

// Model Path
// char modelPath[256] = "";
char modelPath[256] = "";

const char *filePath;

float defaultPosition[3] = {0.0f, 0.0f, 0.0f};
glm::vec3 LightPosition = glm::vec3(1.0f);

glm::vec3 ParticlePosition;
// Simple color struct
struct Color {
  float r, g, b;
};

// Configuration for the water Grid
float amplitude;
float speedGrid;
float frequency;
float sizeGrid;
float spacing;

LOD lod;

bool isSaveScene = false;

// Configuration to store newly created objects
struct ObjectConfig {
  ObjectType type;
  int vectorIndex;
  bool hasCollisionBox;
  float size[3];
  float Position[3];
  float rotation[3];
  Color color; // Now uses named struct
  float transparency;
  float shininess;
  float reflectivity;
  float lightIntensity;
  int physicsType;
  int amount;
  float height;
  float Speed;
  std::string modelPath;
  const char *filepath;
  float Gridamplitude;
  float Gridspeed;
  float Gridfrequency;
  float Gridsize;
  float Gridspacing;
  float modelSize;
  bool isWireFrame;
  DialogueLine dialogue;
};

std::vector<Cube> cubes;         // All cubes in the scene
std::vector<Image> images;       // images storage
std::vector<Particle> particles; // Particle storage
std::vector<CharacterModel> ModelManagers;
std::vector<Grid> grids;
std::vector<Shader> shaders;
std::vector<ObjectConfig> createdObjects; // Stores all created object configs

enum class EditorFileDialogTarget {
  None,
  AssetPath,
  SceneLoad,
  PortraitCreate,
  PortraitEdit,
  BackgroundCreate,
  BackgroundEdit,
  SceneBackground
};

static EditorFileDialogTarget editorFileDialogTarget =
    EditorFileDialogTarget::None;

static void
OpenEditorFileBrowser(EditorFileDialogTarget target, const std::string &title,
                      const std::vector<std::string> &filters,
                      const std::filesystem::path &startPath = {}) {
  if (PortalFileChooser::Instance().IsBusy())
    return;
  if (!PortalFileChooser::Instance().Open(title, filters, startPath))
    return;
  editorFileDialogTarget = target;
}

static void CopySelectedPathToEditorInput(const std::filesystem::path &path) {
  const std::string selectedPath = path.string();
  std::snprintf(modelPath, IM_ARRAYSIZE(modelPath), "%s", selectedPath.c_str());
  filePath = modelPath;
}

static int selectedObjectIndex = -1;
static char sceneName[64] = "Untitled Scene";
static char sceneBackground[512] = "";

static void loadSceneFromPath(const std::filesystem::path &path);

static void SyncDialogueLine(ObjectConfig &obj) {
  obj.dialogue.position = glm::vec2(obj.Position[0], obj.Position[1]);
  obj.dialogue.bodyColor =
      glm::vec3(obj.color.r, obj.color.g, obj.color.b);
  obj.dialogue.isDialogue = obj.type == DIALOUGE;
  if (obj.vectorIndex >= 0)
    GetDialogueSystem().UpdateLine(obj.vectorIndex, obj.dialogue);
}

static void PullDialogueSequences() {
  for (ObjectConfig &obj : createdObjects) {
    if (obj.type != DIALOUGE || obj.vectorIndex < 0)
      continue;
    const DialogueLine *line = GetDialogueSystem().GetLine(obj.vectorIndex);
    if (line)
      obj.dialogue.sequence = line->sequence;
  }
}

static int FindDialogueObject(int lineIndex) {
  for (int i = 0; i < static_cast<int>(createdObjects.size()); ++i) {
    if (createdObjects[i].type == DIALOUGE &&
        createdObjects[i].vectorIndex == lineIndex)
      return i;
  }
  return -1;
}

static void DrawSpeakingOrder() {
  const std::vector<int> order = GetDialogueSystem().DialogueOrder();
  if (order.empty())
    return;

  ImGui::Text("Speaking Order");
  ImGui::Separator();
  ImGui::TextDisabled("Who speaks, then how long before the next speaker.");

  for (int slot = 0; slot < static_cast<int>(order.size()); ++slot) {
    const DialogueLine *line = GetDialogueSystem().GetLine(order[slot]);
    if (!line)
      continue;

    const int objectIndex = FindDialogueObject(order[slot]);
    const std::string speaker =
        line->speaker.empty() ? "Narrator" : line->speaker;
    std::string preview = line->text;
    if (preview.size() > 42)
      preview = preview.substr(0, 42) + "...";

    ImGui::PushID(slot);
    const std::string label =
        std::to_string(slot + 1) + ". " + speaker + ": " + preview;
    if (ImGui::Selectable(label.c_str(), objectIndex == selectedObjectIndex))
      selectedObjectIndex = objectIndex;

    if (slot > 0 && ImGui::SmallButton("Up")) {
      GetDialogueSystem().MoveDialogue(slot, -1);
      PullDialogueSequences();
    }
    if (slot > 0)
      ImGui::SameLine();
    if (slot + 1 < static_cast<int>(order.size()) && ImGui::SmallButton("Down")) {
      GetDialogueSystem().MoveDialogue(slot, 1);
      PullDialogueSequences();
    }

    if (slot + 1 < static_cast<int>(order.size()) && objectIndex >= 0) {
      float transition = createdObjects[objectIndex].dialogue.holdAfterLine;
      if (ImGui::SliderFloat("Transition", &transition, 0.0f, 8.0f,
                             "then %.2f s")) {
        createdObjects[objectIndex].dialogue.holdAfterLine = transition;
        SyncDialogueLine(createdObjects[objectIndex]);
      }
    }
    ImGui::PopID();
  }
  ImGui::Spacing();
}

static void RenderEditorFileBrowser() {
  PortalFileChooser &chooser = PortalFileChooser::Instance();
  chooser.Poll();

  std::filesystem::path selectedPath;
  bool cancelled = false;
  if (!chooser.ConsumeCompletion(selectedPath, cancelled))
    return;

  if (cancelled || selectedPath.empty()) {
    editorFileDialogTarget = EditorFileDialogTarget::None;
    return;
  }

  switch (editorFileDialogTarget) {
  case EditorFileDialogTarget::AssetPath:
    CopySelectedPathToEditorInput(selectedPath);
    break;
  case EditorFileDialogTarget::SceneLoad:
    loadSceneFromPath(selectedPath);
    break;
  case EditorFileDialogTarget::PortraitCreate:
    DialogueCreationDraft().portraitPath = selectedPath.string();
    break;
  case EditorFileDialogTarget::PortraitEdit:
    if (selectedObjectIndex >= 0 &&
        selectedObjectIndex < static_cast<int>(createdObjects.size())) {
      ObjectConfig &obj = createdObjects[selectedObjectIndex];
      if (obj.type == FONT || obj.type == DIALOUGE)
        obj.dialogue.portraitPath = selectedPath.string();
    }
    break;
  case EditorFileDialogTarget::BackgroundCreate:
    DialogueCreationDraft().backgroundPath = selectedPath.string();
    break;
  case EditorFileDialogTarget::BackgroundEdit:
    if (selectedObjectIndex >= 0 &&
        selectedObjectIndex < static_cast<int>(createdObjects.size())) {
      ObjectConfig &obj = createdObjects[selectedObjectIndex];
      if (obj.type == FONT || obj.type == DIALOUGE)
        obj.dialogue.backgroundPath = selectedPath.string();
    }
    break;
  case EditorFileDialogTarget::SceneBackground:
    std::snprintf(sceneBackground, IM_ARRAYSIZE(sceneBackground), "%s",
                  selectedPath.string().c_str());
    GetDialogueSystem().SetSceneBackground(sceneBackground);
    break;
  case EditorFileDialogTarget::None:
    break;
  }

  editorFileDialogTarget = EditorFileDialogTarget::None;
}

// --------------------------------------------------------------------------
// Initializes ImGui (call once in your main initialization)
void initIMGUI(GLFWwindow *window) {
  if (ImGui::GetCurrentContext() != nullptr)
    return;

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 330");
}

void UpdateManager(GLFWwindow *window, Camera &camera) {}

void CreationManager(GLFWwindow *window, Shader &shader, Camera &camera,
                     int scrwidth, int scrheight, double &mouseX,
                     double &mouseY, bool &ishovering, bool &isMoving) {
  // Begin ImGui window
  ImGui::Begin("Pigeon Engine - Object Creator & Editor");

  // ------------------------------------------------
  // SCENE MANAGEMENT (Simple placeholders)
  // ------------------------------------------------
  ImGui::Text("Scene Management");
  ImGui::Separator();
  ImGui::InputText("Scene Name", sceneName, IM_ARRAYSIZE(sceneName));
  ImGui::SameLine();
  if (ImGui::Button("Save")) {
    // TODO: Implement a Save Scene function (serialize objects to a file).
    std::cout << "Saving scene named: " << sceneName << std::endl;
    // isSaveScene = true;
    saveScene();
  }
  ImGui::SameLine();
  if (ImGui::Button("Load")) {
    // TODO: Implement a Load Scene function (deserialize from a file).
    std::cout << "Loading scene into: " << sceneName << std::endl;
    loadScene();
  }
  ImGui::SameLine();
  if (GetDialogueSystem().IsPlaying()) {
    if (ImGui::Button("Stop"))
      GetDialogueSystem().Stop();
    ImGui::SameLine();
    ImGui::TextDisabled("Playing dialogue...");
  } else if (ImGui::Button("Play Scene")) {
    GetDialogueSystem().Play();
  }

  ImGui::InputText("Scene Background", sceneBackground,
                   IM_ARRAYSIZE(sceneBackground));
  ImGui::SameLine();
  if (ImGui::Button("Browse##SceneBackground")) {
    OpenEditorFileBrowser(EditorFileDialogTarget::SceneBackground,
                          "Select a Scene Background",
                          PortalFileChooser::ImageExtensions());
  }

  GetDialogueSystem().SetSceneBackground(sceneBackground);

  ImGui::Separator();
  ImGui::Spacing();

  // ------------------------------------------------
  // OBJECT CREATION
  // ------------------------------------------------
  ImGui::Text("🛠 Object Creation");
  ImGui::Separator();

  // Object Type Selection
  ImGui::Text("Select Object Type:");
  const char *objectTypes[] = {"Particle", "Cube",  "3D Model (OBJ)",
                               "Image",    "Water", "Dialouge",
                               "Font"};
  int currentItem = static_cast<int>(selectedType);
  if (ImGui::Combo("##ObjectType", &currentItem, objectTypes,
                   IM_ARRAYSIZE(objectTypes))) {
    selectedType = static_cast<ObjectType>(currentItem);
  }

  // Collision Box Toggle
  ImGui::Checkbox("Enable Collision Box", &enableCollisionBox);
  ImGui::SameLine();
  ImGui::TextDisabled("(?)");
  if (ImGui::IsItemHovered())
    ImGui::SetTooltip(
        "Enable this to add a collision box for physics calculations.");

  // Object Size
  ImGui::Text("Size:");
  ImGui::DragFloat3("##Size", objectSize, 0.1f, 0.1f, 50.0f, "%.2f");

  // Rotation
  ImGui::Text("Rotation:");
  ImGui::DragFloat3("##Rotation", objectRotation, 0.1f, -360.0f, 360.0f,
                    "%.1f");

  // Object Color Picker
  ImGui::Text("Color:");
  ImGui::ColorEdit3("##Color", objectColor);

  // Transparency Slider
  ImGui::Text("Transparency:");
  ImGui::SliderFloat("##Transparency", &objectTransparency, 0.0f, 1.0f, "%.2f");

  // Physics Options
  ImGui::Text("Physics Properties:");
  const char *physicsOptions[] = {"None", "Rigid Body", "Soft Body", "Static"};
  ImGui::Combo("##PhysicsType", &selectedPhysicsType, physicsOptions,
               IM_ARRAYSIZE(physicsOptions));

  // IMAGE chooser
  if (selectedType == IMAGE) {
    ImGui::Text("Image Path:");
    ImGui::InputText("##ImagePath", modelPath, IM_ARRAYSIZE(modelPath));

    ImGui::SameLine();
    if (ImGui::Button("Browse")) {
      OpenEditorFileBrowser(EditorFileDialogTarget::AssetPath,
                            "Select an Image",
                            PortalFileChooser::ImageExtensions());
    }
  }
  if (selectedType == CUBE) {
    ImGui::Text("Render type:");
    const char *modeNames[] = {"Normal", "light"};
    ImGui::Text("Texture Path:");
    ImGui::InputText("##TexturePath", modelPath, IM_ARRAYSIZE(modelPath));
    ImGui::SameLine();
    if (ImGui::Button("Browse")) {
      OpenEditorFileBrowser(EditorFileDialogTarget::AssetPath,
                            "Select a Texture",
                            PortalFileChooser::ImageExtensions());
    }
  }

  if (selectedType == FONT || selectedType == DIALOUGE) {
    DrawDialogueCreationFields(
        selectedType == DIALOUGE,
        []() {
          OpenEditorFileBrowser(EditorFileDialogTarget::PortraitCreate,
                                "Select a Portrait",
                                PortalFileChooser::ImageExtensions());
        },
        []() {
          OpenEditorFileBrowser(EditorFileDialogTarget::BackgroundCreate,
                                "Select a Background",
                                PortalFileChooser::ImageExtensions());
        });
  }

  if (selectedType == MODEL_OBJ) {
    const char *modeNames[] = {"Normal", "Rainbow", "Light"};
    int currentModeIndex = static_cast<int>(renderMode);

    if (ImGui::Combo("Render Mode", &currentModeIndex, modeNames,
                     IM_ARRAYSIZE(modeNames))) {
      renderMode = static_cast<ModelRenderMode>(currentModeIndex);
    }
    ImGui::InputFloat("Shader Height max height", &modelMaxheight);
    ImGui::InputFloat("Shader height min height", &modelminHeight);
    ImGui::Text("Model Path:");
    ImGui::InputText("##Model Path", modelPath, IM_ARRAYSIZE(modelPath));

    ImGui::SameLine();
    if (ImGui::Button("Browse")) {
      OpenEditorFileBrowser(EditorFileDialogTarget::AssetPath,
                            "Select a 3D Model",
                            PortalFileChooser::ModelExtensions());
    }
  }

  if (selectedType == PARTICLE) {
    // ObjectConfig NewObj;
    ImGui::Text("Particle Values");
    // ImGui::Text("Particle Amount");
    // ImGui::DragInt("Particle Amount", &amount);

    ImGui::InputInt("Particle Amount", &amount);
    // ImGui::SliderInt("Particle Amount", &amount, 0, 10000);
    ImGui::SliderFloat("Speed", &particleSpeed, 0.0f, 10.0f);
    ImGui::SliderFloat("height", &particleHeight, 0.0f, 100.0f);
    // ImGui::DragFloat3("Edit Position", NewObj.Position, 0.1f, -50.0f, 50.0f,
    // "%.2f");

    ImGui::Text("Texture Path:");
    ImGui::InputText("##Texture Path", modelPath, IM_ARRAYSIZE(modelPath));

    ImGui::SameLine();
    if (ImGui::Button("Browse")) {
      OpenEditorFileBrowser(EditorFileDialogTarget::AssetPath,
                            "Select a Particle Texture",
                            PortalFileChooser::ImageExtensions());
    }
  }
  if (selectedType == WATER_GRID) {
    ImGui::Text("Render type:");
    const char *modeNames[] = {"Basic", "Medium", "Detailed"};
    int currentModeIndex = static_cast<int>(lod);
    if (ImGui::Combo("Render Mode", &currentModeIndex, modeNames,
                     IM_ARRAYSIZE(modeNames))) {
      lod = static_cast<LOD>(currentModeIndex);
    }
    ImGui::SliderFloat("Amplitude", &amplitude, 0.0f, 10.0f);
    ImGui::SliderFloat("speed", &speedGrid, 0.0f, 10.0f);
    ImGui::SliderFloat("frequency", &frequency, 0.0f, 10.0f);
    ImGui::SliderFloat("size", &sizeGrid, 0.0f, 500.0f);
    ImGui::SliderFloat("spacing", &spacing, 0.0f, 100.0f);
  }
  // Material Settings
  ImGui::Text("🛠 Material Properties:");
  ImGui::SliderFloat("Shininess", &objectShininess, 1.0f, 128.0f, "%.1f");
  ImGui::SliderFloat("Reflectivity", &objectReflectivity, 0.0f, 1.0f, "%.2f");

  // Lighting Settings
  ImGui::Text("Lighting:");
  ImGui::SliderFloat("Intensity", &lightIntensity, 0.0f, 10.0f, "%.1f");

  // Separator before creation button
  ImGui::Separator();
  //---------------------------------------------------------------
  //---------------- CREATE BUTTON OBJECTS ------------------------
  //---------------------------------------------------------------
  if (ImGui::Button("➕ Create Object", ImVec2(200, 40))) {
    // Build the base object configuration
    ObjectConfig newObj;
    newObj.type = selectedType;
    newObj.hasCollisionBox = enableCollisionBox;
    memcpy(newObj.size, objectSize, sizeof(objectSize));
    memcpy(newObj.rotation, objectRotation, sizeof(objectRotation));
    memcpy(newObj.Position, defaultPosition, sizeof(newObj.Position));
    newObj.color = {objectColor[0], objectColor[1], objectColor[2]};
    newObj.transparency = objectTransparency;
    newObj.shininess = objectShininess;
    newObj.reflectivity = objectReflectivity;
    newObj.lightIntensity = lightIntensity;
    newObj.physicsType = selectedPhysicsType;
    newObj.modelPath = modelPath;
    newObj.filepath = filePath;
    newObj.amount = amount;
    // Grid water values
    newObj.Gridamplitude = amplitude;
    newObj.Gridfrequency = frequency;
    newObj.Gridsize = sizeGrid;
    newObj.Gridspacing = spacing;
    newObj.Gridspeed = speedGrid;
    newObj.isWireFrame = false;

    int newIndex = -1;

    if (selectedType == CUBE) {
      Cube newCube;
      newCube.r = newObj.color.r;
      newCube.g = newObj.color.g;
      newCube.b = newObj.color.b;
      newCube.Alpha = newObj.transparency;
      newCube.Rotation =
          glm::vec3(newObj.rotation[0], newObj.rotation[1], newObj.rotation[2]);
      newCube.Position =
          glm::vec3(newObj.Position[0], newObj.Position[1], newObj.Position[2]);
      newCube.size = glm::vec3(objectSize[0], objectSize[1], objectSize[2]);
      newCube.texturePath = newObj.filepath;

      std::cout << "Cube created at: " << newCube.Position.x << ", "
                << newCube.Position.y << ", " << newCube.Position.z
                << std::endl;

      newCube.loadCube();
      cubes.push_back(newCube);
      newIndex = static_cast<int>(cubes.size()) - 1;
    } else if (selectedType == IMAGE) {
      Image newImage;
      newImage.Position =
          glm::vec3(newObj.Position[0], newObj.Position[1], newObj.Position[2]);
      newImage.Rotation =
          glm::vec3(newObj.rotation[0], newObj.rotation[1], newObj.rotation[2]);
      newImage.imagePath = newObj.filepath;
      newImage.loadImage();

      images.push_back(newImage);
      newIndex = static_cast<int>(images.size()) - 1;
      std::cout << "DEBUG: images.size() now = " << images.size() << std::endl;
    } else if (selectedType == PARTICLE) {
      Particle newParticle;
      newParticle.Position =
          glm::vec3(newObj.Position[0], newObj.Position[1], newObj.Position[2]);
      newParticle.texturePath = newObj.filepath;
      newParticle.Height = newObj.height;
      newParticle.ParticleAmount = amount;
      newParticle.Speed = newObj.Speed;
      newParticle.InitParticle();

      particles.push_back(newParticle);
      newIndex = static_cast<int>(particles.size()) - 1;
    } else if (selectedType == MODEL_OBJ) {
      CharacterModel newModel;
      newModel.ModelPath = modelPath;
      newModel.modelSize = newObj.modelSize;
      newModel.vHeight = modelShaderHeight;
      newModel.isWire = newObj.isWireFrame;
      //  newModel.ModelPosition = glm::vec3(newObj.Position[0],
      //  newObj.Position[1], newObj.Position[2]);
      newModel.currentRenderMode = renderMode;
      switch (renderMode) {
      case ModelRenderMode::NORMAL:
        std::cout << "IMGUI::RENDER::MODE::NORMAL" << std::endl;
        break;
      case ModelRenderMode::RAINBOW:
        std::cout << "IMGUI::RENDER::MODE::RAINBOW" << std::endl;
        break;
      case ModelRenderMode::LIGHT:
        std::cout << "IMGUI::RENDER::MODE::LIGHT" << std::endl;
        break;
      case ModelRenderMode::COUNT:
        break;
      }
      newModel.IMGUIinitializeModelRenderingSystem();
      ModelManagers.push_back(newModel);
      newIndex = static_cast<int>(ModelManagers.size()) - 1;
    } else if (selectedType == WATER_GRID) {
      Grid newGrid;
      newGrid.spacing = spacing;
      newGrid.size = sizeGrid;
      newGrid.frequency = frequency;
      newGrid.amplitude = amplitude;
      newGrid.speed = speedGrid;

      if (lod == LOD::BASIC) {
        newGrid.setupGridWater();
      }
      grids.push_back(newGrid);
      newIndex = static_cast<int>(grids.size()) - 1;
    } else if (selectedType == FONT || selectedType == DIALOUGE) {
      newObj.dialogue = DialogueCreationDraft();
      newObj.dialogue.isDialogue = selectedType == DIALOUGE;
      if (selectedType == DIALOUGE) {
        newObj.color = {newObj.dialogue.bodyColor.r, newObj.dialogue.bodyColor.g,
                        newObj.dialogue.bodyColor.b};
      } else {
        newObj.dialogue.bodyColor =
            glm::vec3(objectColor[0], objectColor[1], objectColor[2]);
      }
      if (selectedType == FONT) {
        newObj.Position[0] = 100.0f;
        newObj.Position[1] = 400.0f;
      } else {
        newObj.Position[0] = 80.0f;
        newObj.Position[1] = 40.0f;
      }
      newObj.dialogue.position =
          glm::vec2(newObj.Position[0], newObj.Position[1]);
      newIndex = GetDialogueSystem().AddLine(newObj.dialogue);
      if (const DialogueLine *stored = GetDialogueSystem().GetLine(newIndex))
        newObj.dialogue.sequence = stored->sequence;
      if (selectedType == DIALOUGE)
        DialogueCreationDraft().text.clear();
    }

    newObj.vectorIndex = newIndex;
    // Now we push the final newObj into createdObjects
    createdObjects.push_back(newObj);
  }

  ImGui::Spacing();
  ImGui::Separator();

  // ------------------------------------------------
  // OBJECT LIST & SELECTION
  // ------------------------------------------------
  ImGui::Text("Created Objects (%d)", (int)createdObjects.size());

  ImGui::BeginChild("ObjectList", ImVec2(0, 100), true);
  for (int i = 0; i < (int)createdObjects.size(); i++) {
    char label[64];
    const char *typeLabel;
    switch (createdObjects[i].type) {
    case CUBE:
      typeLabel = "Cube";
      break;
    case IMAGE:
      typeLabel = "Image";
      break;
    case PARTICLE:
      typeLabel = "Particle";
      break;
    case MODEL_OBJ:
      typeLabel = "Model";
      break;
    case FONT:
      typeLabel = "Font";
      break;
    case DIALOUGE:
      typeLabel = "Dialogue";
      break;
    default:
      typeLabel = "Other";
      break;
    }

    snprintf(label, 64, "Object %d (%s)", i, typeLabel);

    if (ImGui::Selectable(label, (selectedObjectIndex == i))) {
      selectedObjectIndex = i;
    }
  }
  ImGui::EndChild();

  DrawSpeakingOrder();

  // ------------------------------------------------
  // EDIT THE SELECTED OBJECT
  // ------------------------------------------------
  if (selectedObjectIndex >= 0 &&
      selectedObjectIndex < (int)createdObjects.size()) {
    ObjectConfig &obj = createdObjects[selectedObjectIndex];
    if (isSaveScene) {
      // saveScene();
    }
    ImGui::Separator();
    ImGui::Text("Editing Object: %d", selectedObjectIndex);

    // --- Transform Controls ---
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
      if (obj.type == FONT || obj.type == DIALOUGE) {
        ImGui::DragFloat2("Screen Position", obj.Position, 1.0f, 0.0f, 4000.0f,
                          "%.0f");
        ImGui::TextDisabled("Origin is bottom-left of the viewport.");
      } else {
        ImGui::DragFloat3("Position", obj.Position, 0.1f, -50.0f, 50.0f,
                          "%.2f");
        ImGui::DragFloat3("Rotation", obj.rotation, 0.1f, -360.0f, 360.0f,
                          "%.1f");
        ImGui::DragFloat3("Scale", obj.size, 0.1f, 0.1f, 50.0f, "%.2f");
        if (ImGui::Button("Save Position")) {
          glm::vec3 newPos(obj.Position[0], obj.Position[1], obj.Position[2]);
          savePosition(newPos);
        }
        if (ImGui::Button("Load Position")) {
          loadPosition();
        }
      }
    }
    ImGui::Spacing();

    // --- Appearance Controls ---
    if (ImGui::CollapsingHeader("Appearance", ImGuiTreeNodeFlags_DefaultOpen)) {
      // Color and Transparency
      float tempColor[3] = {obj.color.r, obj.color.g, obj.color.b};
      ImGui::ColorEdit3("Color", tempColor);
      obj.color = {tempColor[0], tempColor[1], tempColor[2]};
      ImGui::SliderFloat("Transparency", &obj.transparency, 0.0f, 1.0f, "%.2f");

      // Material
      ImGui::SliderFloat("Shininess", &obj.shininess, 1.0f, 128.0f, "%.1f");
      ImGui::SliderFloat("Reflectivity", &obj.reflectivity, 0.0f, 1.0f, "%.2f");
    }
    ImGui::Spacing();

    // --- Lighting & Physics ---
    if (ImGui::CollapsingHeader("Lighting & Physics",
                                ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::SliderFloat("Light Intensity", &obj.lightIntensity, 0.0f, 10.0f,
                         "%.1f");
      ImGui::InputInt("Particle Amount", &obj.amount);
      ImGui::SliderFloat("Particle Speed", &obj.Speed, 0.0f, 10.0f);
      ImGui::SliderFloat("Particle Height", &obj.height, 0.0f, 100.0f);
    }
    ImGui::Spacing();

    // --- Model Settings ---
    if (obj.type == MODEL_OBJ && ImGui::CollapsingHeader("Model Settings", 0)) {
      ImGui::InputFloat("Model Size", &obj.modelSize);
      ImGui::InputFloat("Shader Max Height", &modelMaxheight);
      ImGui::InputFloat("Shader Min Height", &modelminHeight);
      ImGui::Checkbox("Wire Frame", &obj.isWireFrame);
    }
    ImGui::Spacing();

    // --- Water Grid Settings ---
    if (obj.type == WATER_GRID &&
        ImGui::CollapsingHeader("Water Grid Settings", 0)) {
      ImGui::SliderFloat("Amplitude", &obj.Gridamplitude, 0.0f, 10.0f);
      ImGui::SliderFloat("Speed", &obj.Gridspeed, 0.0f, 10.0f);
      ImGui::SliderFloat("Frequency", &obj.Gridfrequency, 0.0f, 10.0f);
      ImGui::SliderFloat("Size", &obj.Gridsize, 0.0f, 10.0f);
      ImGui::SliderFloat("Spacing", &obj.Gridspacing, 0.0f, 10.0f);
    }
    ImGui::Spacing();

    if ((obj.type == FONT || obj.type == DIALOUGE) &&
        ImGui::CollapsingHeader("Text Settings",
                                ImGuiTreeNodeFlags_DefaultOpen)) {
      DrawDialogueEditFields(
          obj.dialogue, obj.type == DIALOUGE,
          []() {
            OpenEditorFileBrowser(EditorFileDialogTarget::PortraitEdit,
                                  "Select a Portrait",
                                  PortalFileChooser::ImageExtensions());
          },
          []() {
            OpenEditorFileBrowser(EditorFileDialogTarget::BackgroundEdit,
                                  "Select a Background",
                                  PortalFileChooser::ImageExtensions());
          });
    }
    ImGui::Spacing();
    int idx = obj.vectorIndex;
    if (obj.type == IMAGE && idx >= 0 && idx < (int)images.size()) {
      images[idx].r = obj.color.r;
      images[idx].g = obj.color.g;
      images[idx].b = obj.color.b;
      images[idx].Alpha = obj.transparency;
      images[idx].Rotation =
          glm::vec3(obj.rotation[0], obj.rotation[1], obj.rotation[2]);
      images[idx].Position =
          glm::vec3(obj.Position[0], obj.Position[1], obj.Position[2]);
    } else if (obj.type == MODEL_OBJ && idx >= 0 &&
               idx < (int)ModelManagers.size()) {
      ModelManagers[idx].ModelPosition =
          glm::vec3(obj.Position[0], obj.Position[1], obj.Position[2]);
      ModelManagers[idx].ModelRotation =
          glm::vec3(obj.rotation[0], obj.rotation[1], obj.rotation[2]);
      ModelManagers[idx].modelSize = obj.modelSize;
      ModelManagers[idx].maxHeight = modelMaxheight;
      ModelManagers[idx].isWire = obj.isWireFrame;
    } else if (obj.type == CUBE && idx >= 0 && idx < (int)cubes.size()) {
      cubes[idx].r = obj.color.r;
      cubes[idx].g = obj.color.g;
      cubes[idx].b = obj.color.b;
      cubes[idx].Alpha = obj.transparency;
      cubes[idx].Rotation =
          glm::vec3(obj.rotation[0], obj.rotation[1], obj.rotation[2]);
      cubes[idx].Position =
          glm::vec3(obj.Position[0], obj.Position[1], obj.Position[2]);
      LightPosition = cubes[idx].Position;
      cubes[idx].size = glm::vec3(obj.size[0], obj.size[1], obj.size[2]);
    } else if (obj.type == PARTICLE && idx >= 0 &&
               idx < (int)particles.size()) {
      // particles[idx].Speed = obj.Speed;
      // particles[idx].Height = obj.height;
      // particles[idx].Position = glm::vec3(obj.Position[0], obj.Position[1],
      // obj.Position[2]);
      particles[idx].ParticleAmount = amount;
      // cubes[idx].texturePath = obj.filepath;
    } else if (obj.type == WATER_GRID && idx >= 0 && idx < (int)grids.size()) {
      grids[idx].frequency = obj.Gridfrequency;
      grids[idx].amplitude = obj.Gridamplitude;
      grids[idx].speed = obj.Gridspeed;
      grids[idx].spacing = obj.Gridspacing;
      grids[idx].size = obj.Gridsize;
    } else if (obj.type == FONT || obj.type == DIALOUGE) {
      SyncDialogueLine(obj);
    }

    // -------------------
    // Delete button
    // -------------------
    if (ImGui::Button("Delete Object")) {
      const ObjectType deletedType = obj.type;
      const int deletedIndex = idx;
      createdObjects.erase(createdObjects.begin() + selectedObjectIndex);

      if (deletedType == IMAGE && deletedIndex >= 0 &&
          deletedIndex < (int)images.size()) {
        images.erase(images.begin() + deletedIndex);
        std::cout << "DELETED::IMAGE" << std::endl;
      }
      if (deletedType == PARTICLE && deletedIndex >= 0 &&
          deletedIndex < (int)particles.size()) {
        particles.erase(particles.begin() + deletedIndex);
        std::cout << "DELETED::PARTICLE" << std::endl;
      }
      if (deletedType == MODEL_OBJ && deletedIndex >= 0 &&
          deletedIndex < (int)ModelManagers.size()) {
        ModelManagers.erase(ModelManagers.begin() + deletedIndex);
        std::cout << "DELETED::MODEL" << std::endl;
      }

      if (deletedType == CUBE && deletedIndex >= 0 &&
          deletedIndex < (int)cubes.size()) {
        cubes.erase(cubes.begin() + deletedIndex);
      }

      if (deletedType == WATER_GRID && deletedIndex >= 0 &&
          deletedIndex < (int)grids.size()) {
        grids.erase(grids.begin() + deletedIndex);
      }
      if (deletedType == FONT || deletedType == DIALOUGE)
        GetDialogueSystem().RemoveLine(deletedIndex);
      selectedObjectIndex = -1;
    }
  }

  // ------------------------------------------------
  // RENDER OBJECTS
  // ------------------------------------------------
  for (Particle particle : particles) {
    particle.renderParticles(camera, RenderParticle, window);
  }
  for (Image &image : images) {
    image.render(camera);
  }
  for (Cube &cube : cubes) {
    cube.render(camera, window, mouseX, mouseY, ishovering, isMoving);
  }
  for (CharacterModel &models : ModelManagers) {
    models.IMGUIRenderModel(camera, window, scrwidth, scrheight, LightPosition);
  }
  for (Grid &gridsWat : grids) {
    gridsWat.renderGridWater(camera, window);
  }

  int previewLine = -1;
  if (selectedObjectIndex >= 0 &&
      selectedObjectIndex < static_cast<int>(createdObjects.size()) &&
      createdObjects[selectedObjectIndex].type == DIALOUGE) {
    previewLine = createdObjects[selectedObjectIndex].vectorIndex;
  }
  GetDialogueSystem().SetPreviewLine(previewLine);

  RenderEditorFileBrowser();
  ImGui::End();
}

void RenderEditorText(FontManager &fontManager, int scrwidth, int scrheight) {
  GetDialogueSystem().Render(fontManager, scrwidth, scrheight);
}

void stateGame(OpenGLState &opengl) {

  nlohmann::json j;
  // if(std::filesystem::exists(path)
  j["clearColor"] = {opengl.clearColor[0], opengl.clearColor[1],
                     opengl.clearColor[2], opengl.clearColor[3]};
  j["DepthTest"] = opengl.depthTestEnabled;
  std::ofstream outFile("opengl_state.json");
  outFile << j.dump(4);
  outFile.close();
}

EditorSceneStats GetEditorSceneStats() {
  EditorSceneStats stats;
  stats.objects = static_cast<int>(createdObjects.size());
  stats.cubes = static_cast<int>(cubes.size());
  stats.images = static_cast<int>(images.size());
  stats.particles = static_cast<int>(particles.size());
  stats.models = static_cast<int>(ModelManagers.size());
  stats.waterGrids = static_cast<int>(grids.size());
  return stats;
}

static std::string SceneFileName() {
  std::string name = sceneName;
  if (name.empty())
    name = "Untitled Scene";

  for (char &character : name) {
    if (character == '/' || character == '\\' || character == ':')
      character = '_';
  }

  return name + ".json";
}

void saveScene() {
  std::filesystem::create_directory("Scenes");

  nlohmann::json sceneJson;
  sceneJson["sceneName"] = sceneName;
  sceneJson["backgroundPath"] = sceneBackground;

  for (const auto &obj : createdObjects) {
    nlohmann::json objJson;
    objJson["type"] = obj.type;
    objJson["vectorIndex"] = obj.vectorIndex;
    objJson["hasCollisionBox"] = obj.hasCollisionBox;
    objJson["size"] = {obj.size[0], obj.size[1], obj.size[2]};
    objJson["position"] = {obj.Position[0], obj.Position[1], obj.Position[2]};
    objJson["rotation"] = {obj.rotation[0], obj.rotation[1], obj.rotation[2]};
    objJson["color"] = {obj.color.r, obj.color.g, obj.color.b};
    objJson["transparency"] = obj.transparency;
    objJson["shininess"] = obj.shininess;
    objJson["reflectivity"] = obj.reflectivity;
    objJson["lightIntensity"] = obj.lightIntensity;
    objJson["physicsType"] = obj.physicsType;
    objJson["amount"] = obj.amount;
    objJson["height"] = obj.height;
    objJson["Speed"] = obj.Speed;
    objJson["modelPath"] = obj.modelPath;
    objJson["Gridamplitude"] = obj.Gridamplitude;
    objJson["Gridspeed"] = obj.Gridspeed;
    objJson["Gridfrequency"] = obj.Gridfrequency;
    objJson["Gridsize"] = obj.Gridsize;
    objJson["Gridspacing"] = obj.Gridspacing;
    objJson["modelSize"] = obj.modelSize;
    objJson["isWireFrame"] = obj.isWireFrame;
    WriteDialogueJson(objJson, obj.dialogue);

    sceneJson["objects"].push_back(objJson);
  }

  const std::filesystem::path savePath =
      std::filesystem::path("Scenes") / SceneFileName();
  std::ofstream outFile(savePath);
  if (outFile.is_open()) {
    outFile << sceneJson.dump(4);
    outFile.close();
    std::cout << "Scene saved to: " << savePath.string() << std::endl;
  } else {
    std::cerr << "Failed to save scene.\n";
  }
}

void loadScene() {
  std::filesystem::create_directory("Scenes");
  OpenEditorFileBrowser(EditorFileDialogTarget::SceneLoad, "Load Scene",
                        PortalFileChooser::SceneExtensions(), "Scenes");
}

static void loadSceneFromPath(const std::filesystem::path &path) {
  std::ifstream inFile(path);
  if (!inFile.is_open()) {
    std::cerr << "Failed to open file: " << path.string() << std::endl;
    return;
  }

  nlohmann::json sceneJson;
  inFile >> sceneJson;
  inFile.close();

  if (sceneJson.contains("sceneName")) {
    strncpy(sceneName, sceneJson["sceneName"].get<std::string>().c_str(),
            sizeof(sceneName));
  }

  sceneBackground[0] = '\0';
  if (sceneJson.contains("backgroundPath")) {
    strncpy(sceneBackground,
            sceneJson["backgroundPath"].get<std::string>().c_str(),
            sizeof(sceneBackground) - 1);
    sceneBackground[sizeof(sceneBackground) - 1] = '\0';
  }

  // Clear current scene data
  createdObjects.clear();
  cubes.clear();
  images.clear();
  particles.clear();
  ModelManagers.clear();
  grids.clear();
  GetDialogueSystem().Clear();
  GetDialogueSystem().SetSceneBackground(sceneBackground);

  // Rebuild each object
  for (const auto &objJson : sceneJson["objects"]) {
    ObjectConfig newObj;

    newObj.type = static_cast<ObjectType>(objJson["type"].get<int>());
    newObj.vectorIndex = objJson["vectorIndex"];
    newObj.hasCollisionBox = objJson["hasCollisionBox"];

    for (int i = 0; i < 3; ++i) {
      newObj.size[i] = objJson["size"][i];
      newObj.Position[i] = objJson["position"][i];
      newObj.rotation[i] = objJson["rotation"][i];
    }

    newObj.color = {objJson["color"][0], objJson["color"][1],
                    objJson["color"][2]};

    newObj.transparency = objJson["transparency"];
    newObj.shininess = objJson["shininess"];
    newObj.reflectivity = objJson["reflectivity"];
    newObj.lightIntensity = objJson["lightIntensity"];
    newObj.physicsType = objJson["physicsType"];
    newObj.amount = objJson["amount"];
    newObj.height = objJson["height"];
    newObj.Speed = objJson["Speed"];
    newObj.modelPath = objJson["modelPath"];
    newObj.filepath =
        newObj.modelPath.c_str(); // pointer for legacy compatibility
    newObj.Gridamplitude = objJson["Gridamplitude"];
    newObj.Gridspeed = objJson["Gridspeed"];
    newObj.Gridfrequency = objJson["Gridfrequency"];
    newObj.Gridsize = objJson["Gridsize"];
    newObj.Gridspacing = objJson["Gridspacing"];
    newObj.modelSize = objJson["modelSize"];
    newObj.isWireFrame = objJson["isWireFrame"];
    ReadDialogueJson(objJson, newObj.dialogue);
    newObj.dialogue.position =
        glm::vec2(newObj.Position[0], newObj.Position[1]);
    newObj.dialogue.bodyColor =
        glm::vec3(newObj.color.r, newObj.color.g, newObj.color.b);
    newObj.dialogue.isDialogue = newObj.type == DIALOUGE;

    int newIndex = -1;

    // Object reconstruction logic
    switch (newObj.type) {
    case CUBE: {
      Cube cube;
      cube.r = newObj.color.r;
      cube.g = newObj.color.g;
      cube.b = newObj.color.b;
      cube.Alpha = newObj.transparency;
      cube.Position =
          glm::vec3(newObj.Position[0], newObj.Position[1], newObj.Position[2]);
      cube.Rotation =
          glm::vec3(newObj.rotation[0], newObj.rotation[1], newObj.rotation[2]);
      cube.size = glm::vec3(newObj.size[0], newObj.size[1], newObj.size[2]);
      cube.texturePath = newObj.filepath;
      cube.loadCube();

      cubes.push_back(cube);
      newIndex = (int)cubes.size() - 1;
      break;
    }

    case IMAGE: {
      Image image;
      image.imagePath = newObj.filepath;
      image.Position =
          glm::vec3(newObj.Position[0], newObj.Position[1], newObj.Position[2]);
      image.Rotation =
          glm::vec3(newObj.rotation[0], newObj.rotation[1], newObj.rotation[2]);
      image.loadImage();

      images.push_back(image);
      newIndex = (int)images.size() - 1;
      break;
    }

    case PARTICLE: {
      Particle particle;
      particle.Position =
          glm::vec3(newObj.Position[0], newObj.Position[1], newObj.Position[2]);
      particle.texturePath = newObj.filepath;
      particle.Height = newObj.height;
      particle.ParticleAmount = newObj.amount;
      particle.Speed = newObj.Speed;
      particle.InitParticle();

      particles.push_back(particle);
      newIndex = (int)particles.size() - 1;
      break;
    }

    case MODEL_OBJ: {
      CharacterModel model;
      model.ModelPath = newObj.modelPath.c_str();
      model.modelSize = newObj.modelSize;
      model.vHeight = modelShaderHeight;
      model.isWire = newObj.isWireFrame;
      model.ModelPosition =
          glm::vec3(newObj.Position[0], newObj.Position[1], newObj.Position[2]);
      model.ModelRotation =
          glm::vec3(newObj.rotation[0], newObj.rotation[1], newObj.rotation[2]);
      model.currentRenderMode =
          renderMode; // You could load from JSON if needed

      model.IMGUIinitializeModelRenderingSystem();

      ModelManagers.push_back(model);
      newIndex = (int)ModelManagers.size() - 1;
      break;
    }

    case WATER_GRID: {
      Grid grid;
      grid.amplitude = newObj.Gridamplitude;
      grid.speed = newObj.Gridspeed;
      grid.frequency = newObj.Gridfrequency;
      grid.size = newObj.Gridsize;
      grid.spacing = newObj.Gridspacing;

      if (lod == LOD::BASIC) {
        grid.setupGridWater();
      }

      grids.push_back(grid);
      newIndex = (int)grids.size() - 1;
      break;
    }

    case FONT:
    case DIALOUGE: {
      newIndex = GetDialogueSystem().AddLine(newObj.dialogue);
      if (const DialogueLine *stored = GetDialogueSystem().GetLine(newIndex))
        newObj.dialogue.sequence = stored->sequence;
      break;
    }
    }

    newObj.vectorIndex = newIndex;
    createdObjects.push_back(newObj);
  }

  std::cout << "Scene loaded from: " << path.string() << " with "
            << createdObjects.size() << " objects.\n";
}

void savePosition(const glm::vec3 &position) {
  nlohmann::json j;
  j["position"] = {position.x, position.y, position.z};

  std::ofstream outFile("object_position.json");
  if (outFile.is_open()) {
    outFile << j.dump(4);
    outFile.close();
    std::cout << "Position::saved\n";
  }
}

glm::vec3 loadPosition() {
  std::ifstream inFile("object_position.json");
  if (!inFile.is_open()) {
    std::cerr << "No saved position found. Using default." << std::endl;
    return glm::vec3(0.0f);
  }

  nlohmann::json j;
  inFile >> j;

  glm::vec3 pos(0.0f);
  if (j.contains("position") && j["position"].is_array()) {
    pos.x = j["position"][0];
    pos.y = j["position"][1];
    pos.z = j["position"][2];
  }
  return pos;
}
