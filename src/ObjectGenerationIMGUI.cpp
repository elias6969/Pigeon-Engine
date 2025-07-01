#include "IMGUIManager.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <glad/glad.h>
#include <glm/fwd.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

#include <chrono>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <stb_image.h>
#include <string>
#include <thread>
#include <vector>

#include "Cube.h"
#include "Grid.h"
#include "Image.h"
#include "Particle.h"
#include "Shader.h"
#include "SkyBox.h"
#include "Utils.h"
#include "WindowModule.h"
#include "imstb_textedit.h"
#include "modelLoader.h"
#include "tinyfiledialogs.h"

// Enum for Object Types
enum ObjectType { PARTICLE, CUBE, MODEL_OBJ, IMAGE, WATER_GRID };

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
};

std::vector<Cube> cubes;         // All cubes in the scene
std::vector<Image> images;       // images storage
std::vector<Particle> particles; // Particle storage
std::vector<CharacterModel> ModelManagers;
std::vector<Grid> grids;
std::vector<Shader> shaders;
std::vector<ObjectConfig> createdObjects; // Stores all created object configs

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

static int selectedObjectIndex =
    -1; // Which object is currently selected in the list
static char sceneName[64] = "Untitled Scene";

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
  }
  ImGui::SameLine();
  if (ImGui::Button("Load")) {
    // TODO: Implement a Load Scene function (deserialize from a file).
    std::cout << "Loading scene into: " << sceneName << std::endl;
  }

  ImGui::Separator();
  ImGui::Spacing();

  // ------------------------------------------------
  // OBJECT CREATION
  // ------------------------------------------------
  ImGui::Text("🛠 Object Creation");
  ImGui::Separator();

  // Object Type Selection
  ImGui::Text("Select Object Type:");
  const char *objectTypes[] = {"Particle", "Cube", "3D Model (OBJ)", "Image",
                               "Water"};
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
      const char *filter[] = {"*.png", "*.jpg", "*.jpeg", "*.bmp"};
      filePath =
          tinyfd_openFileDialog("Select an Image", // aTitle
                                "",                // aDefaultPathAndFile
                                4,                 // aNumOfFilterPatterns
                                filter,            // aFilterPatterns
                                "Image Files",     // aSingleFilterDescription
                                0 // aAllowMultipleSelects (0 = single file)
          );

      if (filePath) {
        strncpy(modelPath, filePath, IM_ARRAYSIZE(modelPath));
      }
    }
  }
  if (selectedType == CUBE) {
    ImGui::Text("Render type:");
    const char *modeNames[] = {"Normal", "light"};
    ImGui::Text("Texture Path:");
    ImGui::InputText("##TexturePath", modelPath, IM_ARRAYSIZE(modelPath));
    ImGui::SameLine();
    if (ImGui::Button("Browse")) {
      const char *filter[] = {"*.png", "*.jpg", "*.jpeg", "*.bmp"};
      filePath =
          tinyfd_openFileDialog("Select an Image", // aTitle
                                "",                // aDefaultPathAndFile
                                4,                 // aNumOfFilterPatterns
                                filter,            // aFilterPatterns
                                "Image Files",     // aSingleFilterDescription
                                0 // aAllowMultipleSelects (0 = single file)
          );

      if (filePath) {
        strncpy(modelPath, filePath, IM_ARRAYSIZE(modelPath));
      }
    }
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
      const char *filter[] = {"*.obj", "*.glb", "*.gltf"};
      filePath = tinyfd_openFileDialog(
          "Select an OBJ/GLB Model", // aTitle
          "",                        // aDefaultPathAndFile
          3,                         // aNumOfFilterPatterns
          filter,                    // aFilterPatterns
          "OBJ/GLB/GLTF Files",      // aSingleFilterDescription
          0                          // aAllowMultipleSelects (0 = single file)
      );

      if (filePath) {
        strncpy(modelPath, filePath, IM_ARRAYSIZE(modelPath));
      }
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
      const char *filter[] = {"*.png", "*.jpg", "*.jpeg", "*.bmp"};
      filePath =
          tinyfd_openFileDialog("Select an Image", // aTitle
                                "",                // aDefaultPathAndFile
                                4,                 // aNumOfFilterPatterns
                                filter,            // aFilterPatterns
                                "Image Files",     // aSingleFilterDescription
                                0 // aAllowMultipleSelects (0 = single file)
          );

      if (filePath) {
        strncpy(modelPath, filePath, IM_ARRAYSIZE(modelPath));
      }
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

  // ------------------------------------------------
  // EDIT THE SELECTED OBJECT
  // ------------------------------------------------
  if (selectedObjectIndex >= 0 &&
      selectedObjectIndex < (int)createdObjects.size()) {
    ObjectConfig &obj = createdObjects[selectedObjectIndex];
    ImGui::Separator();
    ImGui::Text("Editing Object: %d", selectedObjectIndex);

    // --- Transform Controls ---
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::DragFloat3("Position", obj.Position, 0.1f, -50.0f, 50.0f, "%.2f");
      ImGui::DragFloat3("Rotation", obj.rotation, 0.1f, -360.0f, 360.0f,
                        "%.1f");
      ImGui::DragFloat3("Scale", obj.size, 0.1f, 0.1f, 50.0f, "%.2f");
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
    }

    // -------------------
    // Delete button
    // -------------------
    if (ImGui::Button("Delete Object")) {
      // Remove from createdObjects
      createdObjects.erase(createdObjects.begin() + selectedObjectIndex);

      // Also remove from the specific vector
      if (obj.type == IMAGE && idx >= 0 && idx < (int)images.size()) {
        images.erase(images.begin() + idx);
        std::cout << "DELETED::IMAGE" << std::endl;
      }
      if (obj.type == PARTICLE && idx >= 0 && idx < (int)particles.size()) {
        particles.erase(particles.begin() + idx);
        std::cout << "DELETED::PARTICLE" << std::endl;
      }
      if (obj.type == MODEL_OBJ && idx >= 0 &&
          idx < (int)ModelManagers.size()) {
        ModelManagers.erase(ModelManagers.begin() + idx);
        std::cout << "DELETED::MODEL" << std::endl;
      }

      if (obj.type == CUBE && idx >= 0 && idx < (int)cubes.size()) {
        cubes.erase(cubes.begin() + idx);
      }

      if (obj.type == WATER_GRID && idx >= 0 && idx < (int)grids.size()) {
        grids.erase(grids.begin() + idx);
      }
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

  ImGui::End();
}

void stateGame(OpenGLState &opengl) {

  nlohmann::json j;
  // if(std::filesystem::exists(path)
  j["clearColor"] = {opengl.clearColor[0], opengl.clearColor[1],
                     opengl.clearColor[2], opengl.clearColor[3],
                     opengl.clearColor[4]};
  j["DepthTest"] = opengl.depthTestEnabled;
  std::ofstream outFile("opengl_state.json");
  outFile << j.dump(4);
  outFile.close();
}
