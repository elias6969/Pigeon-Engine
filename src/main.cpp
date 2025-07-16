// ================================
// System and Library Includes
// ================================
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>  // OpenGL function loader
#include <GLFW/glfw3.h> // GLFW window and input handling

// GLM - Mathematics for 3D Graphics
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

// IMGUI - GUI Library
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>
#include <imguiThemes.h>

// Image Loading
#include <stb_image.h>

// Standard Libraries
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

// ================================
// Custom Modules
// ================================
#include "BoundingBox.h" // Bounding box collision detection
#include "Camera.h"      // Camera system
#include "Cube.h"        // Cube rendering system
#include "GeoManager.h"
#include "Grid.h"         // Grid rendering
#include "IMGUIManager.h" // IMGUI manager for UI handling
#include "Image.h"        // Image rendering system
#include "Model.h"        // 3D model representation
#include "Particle.h"     // Particle system
#include "ParticleManager.h"
#include "Shader.h" // Shader management
#include "SkyBox.h" // Skybox rendering
#include "Utils.h"  // Utility functions and global helpers
#include "Variables.h"
#include "WindowModule.h" // Window management module
#include "animator.h"     // Animation management
#include "filemanager.h"  // File management system
#include "modelLoader.h"  // 3D model loader
#include "Voxel.h"

// ================================
// Function Declarations
// ================================
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void processInput(GLFWwindow *window, float deltaTime);
void LoadAnimationInThread(Animation *animation, Animator *animator,
                           const std::string &animationPath, Model *model);

// ================================
// Global Settings and State
// ================================
int SCR_WIDTH = 1200;
int SCR_HEIGHT = 1000;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

bool ishovering = false;
bool isOutcamera = false;
bool isMoving = false;
double mouseX = 0.0, mouseY = 0.0;

int ParticleAmount = 100000;
int particlespeed = 10;
bool showImGuiDemo = false;
bool Collided = false;
bool isRender = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// ================================
// Main Function
// ================================
int main() {
  // ---------------------------
  // Initialize GLFW and Configure OpenGL
  // ---------------------------
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // Create the GLFW window
  GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Pigeon Engine",
                                        nullptr, nullptr);
  if (!window) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // Initialize GLAD
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  // Enable depth testing
  glEnable(GL_DEPTH_TEST);

  // ---------------------------
  // Initialize Resources and Custom Modules
  // ---------------------------
  PathManager pathmanagerctr;

  // Instantiate custom classes
  Cube cube;
  Grid grid;
  SkyBox skybox;
  Particle particle;
  TransparentWindow windowManager;
  CharacterModel playerManager;
  Image imageManager;
  geoData geometryManager;
  Model playerModel;
  MesmerizingParticleSystem newParticleSystem(5000);
  MultiEffectParticleSystem multiParticles(5000);

  // Shader declarations
  Shader cubeShader;
  Shader skyboxshader;
  Shader particleShader;
  Shader PlayerShader;
  Shader boxShader;

  OpenGLState opengl = {{0.1f, 0.2f, 0.3f, 1.0f}, true};
  // Initialize modules
  windowManager.init();
  grid.setupGrid();
  grid.setupGridWater();
  // newParticleSystem.init();
  multiParticles.init(ParticleEffectMode::NOISE_DISTORTION);
  skybox.texturebufferLoading(skyboxshader);

  VoxelChunk voxelchunks(100,16,100);
  voxelchunks.loadCubes();

  // ---------------------------
  // Initialize ImGui (if enabled)
  // ---------------------------
#if REMOVE_IMGUI == 0
  ImGui::CreateContext();
  imguiThemes::embraceTheDarkness();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
  io.FontGlobalScale = 1.0f;
  ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
  ImGuiStyle &style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    style.Colors[ImGuiCol_WindowBg].w = 0.f;
    style.Colors[ImGuiCol_DockingEmptyBg].w = 0.f;
  }
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 330");
#endif

  camera.Position = loadPosition();
  // ---------------------------
  // Main Render Loop
  // ---------------------------
  while (!glfwWindowShouldClose(window)) {
    // Calculate delta time
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // Process input events
    processInput(window, deltaTime);

    // Start a new ImGui frame (if enabled)
#if REMOVE_IMGUI == 0
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
#endif

    // ---------------------------
    // ImGui Debug Window
    // ---------------------------
#if REMOVE_IMGUI == 0
    {
      ImGuiIO &io = ImGui::GetIO();
      ImGui::Begin("Pigeon Engine Debug");
      ImGui::Text("FPS: %.1f", io.Framerate);
      ImGui::Text("Screen Dimension: (%u, %u)", PathManager::SCR_WIDTH,
                  PathManager::SCR_HEIGHT);
      ImGui::Text("Mouse Position: (%.2f, %.2f)", mouseX, mouseY);
      ImGui::Checkbox("Hovering Cube", &ishovering);
      ImGui::Checkbox("Out Camera Mode", &isOutcamera);
      ImGui::Checkbox("Moving", &isMoving);
      ImGui::Checkbox("Collided with Cube", &Collided);
      if(ImGui::Button("Save Position")) {
        savePosition(camera.Position);
      }
      ImGui::End();
    }
#endif

    // ---------------------------
    // Render Scene
    // ---------------------------
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ScreenSizeConfiguration(window, SCR_WIDTH, SCR_HEIGHT);
    CreationManager(window, cubeShader, camera, SCR_WIDTH, SCR_HEIGHT, mouseX,
                    mouseY, ishovering, isMoving);
    grid.renderGrid(camera, window);
    windowManager.render(camera, window);
    stateGame(opengl);
    skybox.renderSkybox(skyboxshader, SCR_WIDTH, SCR_HEIGHT, window, camera);
    voxelchunks.draw(camera, window);

    // ---------------------------
    // Render ImGui on Top (if enabled)
    // ---------------------------
#if REMOVE_IMGUI == 0
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      GLFWwindow *backup_current_context = glfwGetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      glfwMakeContextCurrent(backup_current_context);
    }
#endif

    // Swap buffers and poll events
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // ---------------------------
  // Cleanup and Exit
  // ---------------------------
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}

// ================================
// Callback and Helper Functions
// ================================

// Threaded animation loading function
void LoadAnimationInThread(Animation *animation, Animator *animator,
                           const std::string &animationPath, Model *model) {
  animation->LoadAnimation(animationPath, model);
  animator->loadAnimator(animation);
  std::cout << "Animation loaded in thread!" << std::endl;
}

// Process input events
void processInput(GLFWwindow *window, float deltaTime) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  // Adjust camera speed based on shift key
  camera.SPEED =
      (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ? 10.0f : 2.5f;

  // Toggle cursor mode for out-camera view
  if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !isOutcamera) {
    isOutcamera = true;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }
  if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && isOutcamera) {
    isOutcamera = false;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  }

  // Movement controls
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camera.ProcessKeyboard(FORWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera.ProcessKeyboard(BACKWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    camera.ProcessKeyboard(LEFT, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    camera.ProcessKeyboard(RIGHT, deltaTime);

  if (isOutcamera)
    glfwGetCursorPos(window, &mouseX, &mouseY);
}

// Resize callback to adjust viewport and screen dimensions
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  if (height == 0)
    return;
  glViewport(0, 0, width, height);
  SCR_WIDTH = width;
  SCR_HEIGHT = height;
}

// Mouse movement callback for camera control
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn) {
  if (isOutcamera)
    return;

  float xpos = static_cast<float>(xposIn);
  float ypos = static_cast<float>(yposIn);

  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos; // Reversed: y-coordinates go from bottom to top

  lastX = xpos;
  lastY = ypos;

  camera.ProcessMouseMovement(xoffset, yoffset);
}
