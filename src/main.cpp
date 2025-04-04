// OpenGL and GLFW
#include <glad/glad.h>          // OpenGL function loader
#include <GLFW/glfw3.h>         // GLFW window and input handling

// GLM - Mathematics for 3D Graphics
#include <glm/glm.hpp>          // Core GLM functionality
#include <glm/vec3.hpp>         // Vector operations
#include <glm/mat4x4.hpp>       // Matrix operations
#include <glm/ext/matrix_float4x4.hpp> // Extended matrix operations
#include <glm/fwd.hpp>          // Forward declarations

// IMGUI - GUI Library
#include <imgui.h>              // Core IMGUI functionality
#include <backends/imgui_impl_glfw.h>   // GLFW bindings for IMGUI
#include <backends/imgui_impl_opengl3.h> // OpenGL bindings for IMGUI
#include <imguiThemes.h>        // Custom themes for IMGUI

// Image Loading
#include <stb_image.h>          // STB Image for texture loading (included twice, removing redundancy)

// Standard Libraries
#include <iostream>             // Standard input/output stream
#include <string>               // String manipulation
#include <chrono>               // Time utilities
#include <thread>               // Multithreading utilities

// Custom Modules
#include "Shader.h"             // Shader management
#include "Camera.h"             // Camera system
#include "Cube.h"               // Cube rendering system
#include "Particle.h"           // Particle system
#include "SkyBox.h"             // Skybox rendering
#include "WindowModule.h"       // Window management module
#include "Grid.h"               // Grid rendering
#include "Image.h"              // Image rendering system
#include "Utils.h"              // Utility functions and global helpers
#include "filemanager.h"        // File management system
#include "modelLoader.h"        // 3D model loader
#include "Model.h"              // 3D model representation
#include "animator.h"           // Animation management
#include "BoundingBox.h"        // Bounding box collision detection
#include "IMGUIManager.h"       // IMGUI manager for UI handling
#include "GeoManager.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow *window, float deltaTime);
void LoadAnimationInThread(Animation* animation, Animator* animator, const std::string& animationPath, Model* model);

// settings
int SCR_WIDTH = 1200;
int SCR_HEIGHT = 1000;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool ishovering = false;
bool isOutcamera = false;
bool isMoving = false;
double mouseX=0.0, mouseY = 0.0;
int ParticleAmount = 100000;
int particlespeed = 10;

bool showImGuiDemo = false; // To toggle ImGui demo window
bool Collided = false;
bool isRender = true;

// timing
float deltaTime = 0.0f;    
float lastFrame = 0.0f;


int main()
{
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Pigeon Engine", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    // configure global opengl state
    glEnable(GL_DEPTH_TEST);

    VirtualFileSystem vfs("../resources/");
    std::string resources = vfs.getFullPath("models/");

    //Classes
    Cube cube;
    Grid grid;
    SkyBox skybox;
    Particle particle;
    TransparentWindow windowManager;  
    CharacterModel playerManager;
    Image imageManager;
    geoData geometryManager;

    //Models
    Model playerModel;

    //Shaders declarations
    Shader cubeShader;
    Shader skyboxshader;
    Shader particleShader;
    Shader PlayerShader;
    Shader boxShader;
    //Inilialization
    windowManager.init();
    //grid.size = 10.0f, grid.spacing = 0.5f;
    grid.setupGrid();
    //grid.size = 5.0f;
    //grid.spacing = 2.5f;
    //grid.setupGridWater();
    skybox.texturebufferLoading(skyboxshader);

    //geometryManager.initGeometry();

#pragma region imgui
#if REMOVE_IMGUI == 0
	ImGui::CreateContext();
	imguiThemes::embraceTheDarkness();

	ImGuiIO &io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         

	io.FontGlobalScale = 1.0f; 
  ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(0.0f, 1.0f, 1.0f, 1.0f); 
	ImGuiStyle &style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		//style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 0.f;
		style.Colors[ImGuiCol_DockingEmptyBg].w = 0.f;
	}

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
#endif
#pragma endregion

    glm::vec3 cubeposition = glm::vec3(0.0f, 0.0f, 0.0f);
    
    // Variables for sticky corners
    static int corner = 0; // 0: top-left, 1: top-right, 2: bottom-left, 3: bottom-right

    const float snapDistance = 20.0f;
    float height = 0.0f;

    float shaderheight = 3.0f;



    bool renderDistance = true;

    int counter = 0;

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window, deltaTime);
        
	    #pragma region imgui
		    ImGui_ImplOpenGL3_NewFrame();
		    ImGui_ImplGlfw_NewFrame();
		    ImGui::NewFrame();
		    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
	    #pragma endregion

        // ImGui window
        ImGui::Begin("Pigeon Engine Debug");

        // Display FPS
        ImGui::Text("FPS: %.1f", io.Framerate);
        ImGui::Text("Mouse Position: (%.2f, %.2f)", mouseX, mouseY);
        // Controls for toggling features
        ImGui::Checkbox("Hovering Cube", &ishovering);
        ImGui::Checkbox("Out Camera Mode", &isOutcamera);
        ImGui::Checkbox("Moving", &isMoving);
        ImGui::Checkbox("Collided with Cube", &Collided);
        ImGui::SliderFloat("Amplitude", &grid.amplitude, 0.0f, 10.0f);
        ImGui::SliderFloat("speed", &grid.speed, 0.0f, 10.0f);
        ImGui::SliderFloat("frequency", &grid.frequency, 0.0f, 10.0f);
        ImGui::SliderFloat("size", &grid.size, 0.0f, 10.0f);
        ImGui::SliderFloat("spacing", &grid.spacing, 0.0f, 10.0f);
        ImGui::End();
        // render
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ScreenSize(SCR_WIDTH, SCR_HEIGHT);
        geometryManager.RenderGeo();
        CreationManager(window, cubeShader, camera, SCR_WIDTH, SCR_HEIGHT, mouseX, mouseY, ishovering, isMoving);
        //grid.renderGridWater(camera, window);
        grid.renderGrid(camera, window);
        skybox.renderSkybox(skyboxshader, SCR_WIDTH, SCR_HEIGHT, window, camera);
        windowManager.render(camera, window);
        //particle.renderParticles(camera, SCR_WIDTH, SCR_HEIGHT, isRender, window);

	      #pragma region imgui
		      ImGui::Render();
		      int display_w, display_h;
		      glfwGetFramebufferSize(window, &display_w, &display_h);
		      glViewport(0, 0, display_w, display_h);
		      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		      if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	      	{
	      		GLFWwindow *backup_current_context = glfwGetCurrentContext();
	      		ImGui::UpdatePlatformWindows();
	      		ImGui::RenderPlatformWindowsDefault();
	      		glfwMakeContextCurrent(backup_current_context);
	      	}
        #pragma endregion

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void LoadAnimationInThread(Animation* animation, Animator* animator, const std::string& animationPath, Model* model) {
    animation->LoadAnimation(animationPath, model);
    animator->loadAnimator(animation);
    std::cout << "Animation loaded in thread!" << std::endl;
}

void processInput(GLFWwindow* window, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    camera.SPEED = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ? 10.0f : 2.5f;
    
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !isOutcamera) {
        isOutcamera = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && isOutcamera) {
        isOutcamera = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.ProcessKeyboard(FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.ProcessKeyboard(LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.ProcessKeyboard(RIGHT, deltaTime);
    }

    if(isOutcamera)
      glfwGetCursorPos(window, &mouseX, &mouseY);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    if(height == 0) return;
    float aspectRatio = (float)width / (float)height;
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    if (isOutcamera) {
        return;
    }

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}
