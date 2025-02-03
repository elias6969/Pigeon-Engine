#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "IMGUIManager.h"
#include <iostream>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <stb_image.h>
#include <string>
#include <vector>
#include <stb_image.h>
#include <chrono>
#include <thread>
#include "imstb_textedit.h"

// Enum for Object Types
enum ObjectType { PARTICLE, CUBE, MODEL_OBJ, IMAGE };

// Selected Object Type
ObjectType selectedType = CUBE;  // Default to Cube

// Boolean for Collision Box Toggle
bool enableCollisionBox = false;

// Object Transform Properties
float objectSize[3] = {1.0f, 1.0f, 1.0f};   // Default size (X, Y, Z)
float objectRotation[3] = {0.0f, 0.0f, 0.0f}; // Default rotation (X, Y, Z)

// Object Color (RGB)
float objectColor[3] = {1.0f, 1.0f, 1.0f};  // Default to white

// Transparency (0.0 = Fully Opaque, 1.0 = Fully Transparent)
float objectTransparency = 0.0f;  

// Material Properties
float objectShininess = 32.0f;    // Default shininess
float objectReflectivity = 0.5f;  // Default reflectivity

// Lighting Properties
float lightIntensity = 1.0f;  // Default light intensity

// Physics Type Selection
int selectedPhysicsType = 0;  // Default to "None" (index in the physicsOptions array)

// Model Path String
char modelPath[256] = "";  // Empty string for now

struct Color {
    float r, g, b;
};

struct ObjectConfig {
    ObjectType type;
    bool hasCollisionBox;
    float size[3];
    float rotation[3];
    Color color;  // ✅ Use the named struct
    float transparency;
    float shininess;
    float reflectivity;
    float lightIntensity;
    int physicsType;
    std::string modelPath;
};
// Vector to Store Created Objects
std::vector<ObjectConfig> createdObjects;

void initIMGUI(GLFWwindow* window)
{
    if(ImGui::GetCurrentContext() != nullptr)
      return;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void AddEmojiRanges(ImVector<ImWchar>& ranges)
{
    // Manually specify the UTF-32 values for emojis
    ranges.push_back(0x1F3A8); // 🎨
    ranges.push_back(0x1F6E0); // 🛠
    ranges.push_back(0x1F539); // 🔹
    ranges.push_back(0x1F4E6); // 📦
    ranges.push_back(0x1F55F); // 📏
    ranges.push_back(0x1F504); // 🔄
    ranges.push_back(0x1F3A8); // 🎨 (again, duplicate example)
    ranges.push_back(0x1F32B); // 🌫
    ranges.push_back(0x1F6E0); // 🛠 (again, duplicate)
    ranges.push_back(0x1F4A1); // 💡
    ranges.push_back(0x2795); // ➕
    ranges.push_back(0x1F4CB); // 📋
}
void CreationManager()
{
    ImGui::Begin("🎨 Pigeon Engine - Object Creator");

    ImGui::Text("🛠 Object Configuration");
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

    // Model Path Input (For 3D Model)
    if (selectedType == MODEL_OBJ)
    {
        ImGui::Text("📁 Model Path:");
        ImGui::InputText("##ModelPath", modelPath, IM_ARRAYSIZE(modelPath));

        ImGui::SameLine();
        if (ImGui::Button("📂 Browse"))
        {
            // TODO: Add file dialog function to select a file
        }
    }

    // Material Settings
    ImGui::Text("🛠 Material Properties:");
    ImGui::SliderFloat("Shininess", &objectShininess, 1.0f, 128.0f, "%.1f");
    ImGui::SliderFloat("Reflectivity", &objectReflectivity, 0.0f, 1.0f, "%.2f");

    // Lighting Settings
    ImGui::Text("💡 Lighting:");
    ImGui::SliderFloat("Intensity", &lightIntensity, 0.0f, 10.0f, "%.1f");

    // Create Object Button
    ImGui::Separator();
    if (ImGui::Button("➕ Create Object", ImVec2(200, 40)))
    {
        ObjectConfig newObj;
        newObj.type = selectedType;
        newObj.hasCollisionBox = enableCollisionBox;
        memcpy(newObj.size, objectSize, sizeof(objectSize));
        memcpy(newObj.rotation, objectRotation, sizeof(objectRotation));
        newObj.color = { objectColor[0], objectColor[1], objectColor[2]};
        newObj.transparency = objectTransparency;
        newObj.shininess = objectShininess;
        newObj.reflectivity = objectReflectivity;
        newObj.lightIntensity = lightIntensity;
        newObj.physicsType = selectedPhysicsType;
        newObj.modelPath = modelPath;
        createdObjects.push_back(newObj);
    }

    // Created Objects List
    ImGui::Separator();
    ImGui::Text("📋 Created Objects: %d", (int)createdObjects.size());

    ImGui::End();
}
