#include <cstddef>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "IMGUIManager.h"
#include <glm/fwd.hpp>
#include <iostream>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <stb_image.h>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include "imstb_textedit.h"
#include "ObjectRendererManager.h"
#include "tinyfiledialogs.h"

// Forward declarations (if needed)
void addCube(GLFWwindow* window);

// Enum for Object Types
enum ObjectType {
    PARTICLE,
    CUBE,
    MODEL_OBJ,
    IMAGE
};

// Selected Object Type (default to CUBE)
ObjectType selectedType = CUBE;

// Collision Box
bool enableCollisionBox = false;

// Object Transform
float objectSize[3]   = { 1.0f, 1.0f, 1.0f };
float objectRotation[3] = { 0.0f, 0.0f, 0.0f };

// Object Color (RGB)
float objectColor[3]    = { 1.0f, 1.0f, 1.0f };

// Transparency (0.0 = Opaque, 1.0 = Transparent)
float objectTransparency = 0.0f;

// Material / Lighting
float objectShininess   = 32.0f;
float objectReflectivity= 0.5f;
float lightIntensity    = 1.0f;

// Physics
int   selectedPhysicsType = 0; // Index into the physics options array

// Model Path
char modelPath[256] = "";

const char* filePath;
// Simple color struct
struct Color {
    float r, g, b;
};

// Configuration to store newly created objects
struct ObjectConfig {
    ObjectType type;
    bool       hasCollisionBox;
    float      size[3];
    float      Position[3];
    float      rotation[3];
    Color      color;  // Now uses named struct
    float      transparency;
    float      shininess;
    float      reflectivity;
    float      lightIntensity;
    int        physicsType;
    std::string modelPath;
    const char* filepath;
};

std::vector<Cube> cubes;              // All cubes in the scene
std::vector<Image> images; //images storage
std::vector<Particle> paricles; //Particle storage
std::vector<ObjectConfig> createdObjects; // Stores all created object configs

// --------------------------------------------------------------------------
// Initializes ImGui (call once in your main initialization)
void initIMGUI(GLFWwindow* window)
{
    if (ImGui::GetCurrentContext() != nullptr)
        return;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}


static int selectedObjectIndex = -1;   // Which object is currently selected in the list
static char sceneName[64] = "Untitled Scene";

void CreationManager(GLFWwindow* window,
                     Shader &shader,
                     Camera &camera,
                     int scrwidth,
                     int scrheight,
                     double &mouseX,
                     double &mouseY,
                     bool &ishovering,
                     bool &isMoving)
{
    // Begin ImGui window
    ImGui::Begin("🎨 Pigeon Engine - Object Creator & Editor");

    // ------------------------------------------------
    // SCENE MANAGEMENT (Simple placeholders)
    // ------------------------------------------------
    ImGui::Text("🌐 Scene Management");
    ImGui::Separator();
    ImGui::InputText("Scene Name", sceneName, IM_ARRAYSIZE(sceneName));
    ImGui::SameLine();
    if (ImGui::Button("💾 Save"))
    {
        // TODO: Implement a Save Scene function (serialize objects to a file).
        std::cout << "Saving scene named: " << sceneName << std::endl;
    }
    ImGui::SameLine();
    if (ImGui::Button("📂 Load"))
    {
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
    ImGui::Text("🔹 Select Object Type:");
    const char* objectTypes[] = { "Particle", "Cube", "3D Model (OBJ)", "Image" };
    int currentItem = static_cast<int>(selectedType);
    if (ImGui::Combo("##ObjectType", &currentItem, objectTypes, IM_ARRAYSIZE(objectTypes)))
    {
        selectedType = static_cast<ObjectType>(currentItem);
    }

    // Collision Box Toggle
    ImGui::Checkbox("📦 Enable Collision Box", &enableCollisionBox);
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Enable this to add a collision box for physics calculations.");

    // Object Size
    ImGui::Text("📏 Size:");
    ImGui::DragFloat3("##Size", objectSize, 0.1f, 0.1f, 50.0f, "%.2f");

    // Rotation
    ImGui::Text("🔄 Rotation:");
    ImGui::DragFloat3("##Rotation", objectRotation, 0.1f, -360.0f, 360.0f, "%.1f");

    // Object Color Picker
    ImGui::Text("🎨 Color:");
    ImGui::ColorEdit3("##Color", objectColor);

    // Transparency Slider
    ImGui::Text("🌫 Transparency:");
    ImGui::SliderFloat("##Transparency", &objectTransparency, 0.0f, 1.0f, "%.2f");

    // Physics Options
    ImGui::Text("🛠 Physics Properties:");
    const char* physicsOptions[] = { "None", "Rigid Body", "Soft Body", "Static" };
    ImGui::Combo("##PhysicsType", &selectedPhysicsType, physicsOptions, IM_ARRAYSIZE(physicsOptions));
    if (selectedType == IMAGE)
    {
        ImGui::Text("📁 Image Path:");
        ImGui::InputText("##ImagePath", modelPath, IM_ARRAYSIZE(modelPath));

        ImGui::SameLine();
        if (ImGui::Button("📂 Browse"))
        {
          const char* filter[] = { "*.png", "*.jpg", "*.jpeg", "*.bmp" };
          filePath = tinyfd_openFileDialog(
            "Select an Image",     // aTitle
            "",                    // aDefaultPathAndFile
            4,                     // aNumOfFilterPatterns
            filter,                // aFilterPatterns
            "Image Files",         // aSingleFilterDescription
            0                      // aAllowMultipleSelects (0 = single file)
);

          if (filePath)
          {
            strncpy(modelPath, filePath, IM_ARRAYSIZE(modelPath));
          }
        }
    }
    // Model Path Input (only if 3D Model is selected)
    if (selectedType == MODEL_OBJ)
    {
        ImGui::Text("📁 Model Path:");
        ImGui::InputText("##ModelPath", modelPath, IM_ARRAYSIZE(modelPath));

        ImGui::SameLine();
        if (ImGui::Button("📂 Browse"))
        {
            // TODO: Implement your file dialog
        }
    }

    // Material Settings
    ImGui::Text("🛠 Material Properties:");
    ImGui::SliderFloat("Shininess", &objectShininess, 1.0f, 128.0f, "%.1f");
    ImGui::SliderFloat("Reflectivity", &objectReflectivity, 0.0f, 1.0f, "%.2f");

    // Lighting Settings
    ImGui::Text("💡 Lighting:");
    ImGui::SliderFloat("Intensity", &lightIntensity, 0.0f, 10.0f, "%.1f");

    // Separator before creation button
    ImGui::Separator();

    // Create Object Button
    if (ImGui::Button("➕ Create Object", ImVec2(200, 40)))
    {
        // Build the object configuration
        ObjectConfig newObj;
        newObj.type            = selectedType;
        newObj.hasCollisionBox = enableCollisionBox;
        memcpy(newObj.size,      objectSize,     sizeof(objectSize));
        memcpy(newObj.rotation,  objectRotation, sizeof(objectRotation));
        newObj.color           = { objectColor[0], objectColor[1], objectColor[2] };
        newObj.transparency    = objectTransparency;
        newObj.shininess       = objectShininess;
        newObj.reflectivity    = objectReflectivity;
        newObj.lightIntensity  = lightIntensity;
        newObj.physicsType     = selectedPhysicsType;
        newObj.modelPath       = modelPath;
        newObj.filepath = filePath;
        // Store this config
        createdObjects.push_back(newObj);

        // If it's a cube, create a Cube instance
        if (selectedType == CUBE)
        {
            Cube newCube;
            newCube.r = newObj.color.r;
            newCube.g = newObj.color.g;
            newCube.b = newObj.color.b;
            newCube.Alpha = newObj.transparency;
            newCube.Rotation = glm::vec3(newObj.rotation[0], newObj.rotation[1], newObj.rotation[2]);

            // Assign a position. Hard-coded at (5,5,5) or something more interesting:
            newCube.Position = glm::vec3(
                5.0f + (float)cubes.size() * 1.5f, 
                1.0f, 
                -5.0f
            );
            newCube.size = glm::vec3(objectSize[0], objectSize[1], objectSize[2]);

            std::cout << "Cube created at: "
                      << newCube.Position.x << ", "
                      << newCube.Position.y << ", "
                      << newCube.Position.z << std::endl;

            // Add it to our list of cubes
            cubes.push_back(newCube);
        }
        else if(selectedType == IMAGE)
        {
          Image newImage;
          newImage.Position = glm::vec3(newObj.Position[0], newObj.Position[1], newObj.Position[2]);
         // newImage.Rotation = glm::vec3(newObj.rotation[0], newObj.rotation[1], newObj.rotation[2]);
          newImage.imagePath = filePath;
          newImage.loadImage();
          images.push_back(newImage);
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    // ------------------------------------------------
    // OBJECT LIST & SELECTION
    // ------------------------------------------------
    ImGui::Text("📋 Created Objects (%d)", (int)createdObjects.size());

    // We can put the list of objects in a child window or just inline
    ImGui::BeginChild("ObjectList", ImVec2(0, 100), true);
    for (int i = 0; i < (int)createdObjects.size(); i++)
    {
        char label[64];
        snprintf(label, 64, "Object %d (%s)", i,
                 (createdObjects[i].type == CUBE) ? "Cube" : "Other");

        if (ImGui::Selectable(label, (selectedObjectIndex == i)))
        {
            selectedObjectIndex = i;
        }
    }
    ImGui::EndChild();

    // ------------------------------------------------
    // EDIT THE SELECTED OBJECT
    // ------------------------------------------------

    if (selectedObjectIndex >= 0 && selectedObjectIndex < (int)createdObjects.size())
    {
        ObjectConfig &obj = createdObjects[selectedObjectIndex];
        ImGui::Separator();
        ImGui::Text("Editing Object: %d", selectedObjectIndex);

        // Edit size
        ImGui::DragFloat3("Edit Size", obj.size, 0.1f, 0.1f, 50.0f, "%.2f");
        // Edit rotation
        ImGui::DragFloat3("Edit Rotation", obj.rotation, 0.1f, -360.0f, 360.0f, "%.1f");
        ImGui::DragFloat3("Edit Position", obj.Position, 0.1f,0.1f,50.0f,"%.2f");
        // Edit color
        float tempColor[3] = { obj.color.r, obj.color.g, obj.color.b };
        ImGui::ColorEdit3("Edit Color", tempColor);
        obj.color.r = tempColor[0];
        obj.color.g = tempColor[1];
        obj.color.b = tempColor[2];

        if(obj.type == IMAGE && selectedObjectIndex < (int)images.size())
        {
          images[selectedObjectIndex].Position = glm::vec3(obj.Position[0],obj.Position[1],obj.Position[2]);
          images[selectedObjectIndex].imagePath = filePath;
        }
        if (obj.type == CUBE && selectedObjectIndex < (int)cubes.size())
        {
            cubes[selectedObjectIndex].r     = obj.color.r;
            cubes[selectedObjectIndex].g     = obj.color.g;
            cubes[selectedObjectIndex].b     = obj.color.b;
            cubes[selectedObjectIndex].size  = glm::vec3(obj.size[0], obj.size[1], obj.size[2]);
            cubes[selectedObjectIndex].Alpha = obj.transparency;
            cubes[selectedObjectIndex].Rotation = glm::vec3(obj.rotation[0], obj.rotation[1], obj.rotation[2]);
            cubes[selectedObjectIndex].Position = glm::vec3(obj.Position[0], obj.Position[1],obj.Position[2]);
        }
        

        // Optionally, more edits: transparency, shininess, etc.
        ImGui::SliderFloat("Edit Transparency", &obj.transparency, 0.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Edit Shininess", &obj.shininess, 1.0f, 128.0f, "%.1f");
        ImGui::SliderFloat("Edit Reflectivity", &obj.reflectivity, 0.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Edit Intensity", &obj.lightIntensity, 0.0f, 10.0f, "%.1f");

        // Delete button
        if (ImGui::Button("❌ Delete Object"))
        {
            createdObjects.erase(createdObjects.begin() + selectedObjectIndex);

            if (obj.type == CUBE && selectedObjectIndex < (int)cubes.size())
            {
                cubes.erase(cubes.begin() + selectedObjectIndex);
            }
            selectedObjectIndex = -1;
        }
    }

    for (Image& image : images)
    {
      image.render(camera, scrwidth, scrheight);
    }
    for (Cube& cube : cubes)
    {
        cube.render(shader, camera, scrwidth, scrheight,
                    window, mouseX, mouseY, ishovering, isMoving);
    }

    ImGui::End();
}
