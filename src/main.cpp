#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/fwd.hpp>
#include <iostream>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <stb_image.h>
#include <string>
#include <stb_image.h>
#include <chrono>
#include <thread>

#include "Shader.h"
#include "Camera.h"
#include "ObjectRendererManager.h"
#include "filemanager.h"
#include "modelLoader.h"
#include "Model.h"
#include "animator.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window, float deltaTime);
void LoadAnimationInThread(Animation* animation, Animator* animator, const std::string& animationPath, Model* model);

// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 1000;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool ishovering = false;
bool isOutcamera = false;
bool isMoving = false;
bool RenderParticle = false;
double mouseX=0.0, mouseY = 0.0;
int ParticleAmount = 100000;
int particlespeed = 10;

bool showImGuiDemo = false; // To toggle ImGui demo window

// timing
float deltaTime = 0.0f;    
float lastFrame = 0.0f;


void setWindowIcon(GLFWwindow* window) {
    int width, height, channels;

    // Get the full path to the image using your virtual file system
    VirtualFileSystem resources("../resources/");
    std::string imagepath = resources.getFullPath("Textures/");

    // Ensure imagepath ends with a '/' if needed
    if (!imagepath.empty() && imagepath.back() != '/')
        imagepath += '/';

    // Load the image
    unsigned char* image = stbi_load((imagepath + "Break.jpg").c_str(), &width, &height, &channels, 4);
    if (image) {
        // Set the icon
        GLFWimage icon;
        icon.width = width;
        icon.height = height;
        icon.pixels = image;
        glfwSetWindowIcon(window, 1, &icon);

        // Free the image memory
        stbi_image_free(image);
    } else {
        // Handle error gracefully
        std::cerr << "Failed to load icon image from: " << (imagepath + "Break.jpg") << std::endl;
    }
}


int main()
{
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Pigeon Engine!", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    setWindowIcon(window);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    glEnable(GL_DEPTH_TEST);

    //Classes
    Cube cube;
    Grid grid;
    SkyBox skybox;
    Particle particle;
    TransparentWindow windowManager;  
    CharacterModel playerManager;
    Image imageManager;

    //Models
    Model playerModel;

    //Shaders declarations
    Shader cubeShader;
    Shader gridShader;
    Shader skyboxshader;
    Shader particleShader;
    Shader PlayerShader;
    //Inilialization
    windowManager.init();
    cube.loadCube(cubeShader);
    grid.setupGrid(gridShader, 10.0f, 0.5f);
    particle.InitParticle(particleShader);
    imageManager.loadImage();
    skybox.texturebufferLoading(skyboxshader);

    // ImGui: setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg].w = 0.5f;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glm::vec3 cubeposition = glm::vec3(0.0f, 0.0f, 0.0f);
    
    // Variables for sticky corners
    static int corner = 0; // 0: top-left, 1: top-right, 2: bottom-left, 3: bottom-right

    const float snapDistance = 20.0f;
    float height = 0.0f;

    glm::vec3 mountainposition = glm::vec3(10.0f, 0.0f,0.0f);
    glm::vec3 housePosition = glm::vec3(10.0f, 0.0f, 10.0f);
    float shaderheight = 3.0f;


    VirtualFileSystem vfs("../resources/");
    std::string resources = vfs.getFullPath("models/");

    //playerManager.customModel(PlayerShader, playerModel, "guitarplaying/guitar.dae", "model.vs", "model.fs");
    //Animation danceanimation;
    //danceanimation.LoadAnimation((resources + "guitarplaying/guitar.dae"), &playerModel);
    //Animator animator;
    //animator.loadAnimator(&danceanimation);

    bool renderDistance = true;

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window, deltaTime);
        //animator.UpdateAnimation(deltaTime);

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // ImGui window
        ImGui::Begin("Pigeon Engine Debug");

        // Display FPS
        ImGui::Text("FPS: %.1f", io.Framerate);
        ImGui::Text("Mouse Position: (%.2f, %.2f)", mouseX, mouseY);
        // Controls for toggling features
        ImGui::Checkbox("Hovering Cube", &ishovering);
        ImGui::Checkbox("Out Camera Mode", &isOutcamera);
        ImGui::Checkbox("Moving", &isMoving);
        ImGui::Text("Soldier position: (%.2f, %.2f, %.2f)", housePosition.x, housePosition.y, housePosition.z);

        // Toggle demo window
        ImGui::Checkbox("Show ImGui Demo", &showImGuiDemo);
        if (showImGuiDemo)
          ImGui::ShowDemoWindow();

        ImGui::Separator();
        ImGui::Text("Window Snapping");
        ImVec2 OriginalWindowpos = ImGui::GetWindowPos();
        ImVec2 debugWindowPos = OriginalWindowpos;
        ImVec2 debugWindowSize = ImGui::GetWindowSize();

        if (debugWindowPos.x <= snapDistance) // Snap to left edge
          debugWindowPos.x = 0;
        if (debugWindowPos.x + debugWindowSize.x >= SCR_WIDTH - snapDistance) // Snap to right edge
          debugWindowPos.x = SCR_WIDTH - debugWindowSize.x;
        if (debugWindowPos.y <= snapDistance) // Snap to top edge
          debugWindowPos.y = 0;
        if (debugWindowPos.y + debugWindowSize.y >= SCR_HEIGHT - snapDistance) // Snap to bottom edge
           debugWindowPos.y = SCR_HEIGHT - debugWindowSize.y;

        if(OriginalWindowpos.x != debugWindowPos.x || OriginalWindowpos.y != debugWindowPos.y)
          ImGui::SetWindowPos(OriginalWindowpos);

        ImGui::End();

        ImGui::Begin("Particle");
        ImGui::Checkbox("Render Particle", &RenderParticle);
        ImGui::InputInt("Amount", &ParticleAmount);
        ImGui::InputFloat("shader height", &shaderheight);
        ImGui::Separator();
        ImGui::Text("Window Snapping");

        ImVec2 originalparticlewindowpos = ImGui::GetWindowPos();
        ImVec2 particleWindowPos = originalparticlewindowpos;
        ImVec2 particleWindowSize = ImGui::GetWindowSize();

        if (particleWindowPos.x <= snapDistance) // Snap to left edge
          particleWindowPos.x = 0;
        if (particleWindowPos.x + particleWindowSize.x >= SCR_WIDTH - snapDistance) // Snap to right edge
          particleWindowPos.x = SCR_WIDTH - particleWindowSize.x;
        if (particleWindowPos.y <= snapDistance) // Snap to top edge
          particleWindowPos.y = 0;
       if (particleWindowPos.y + particleWindowSize.y >= SCR_HEIGHT - snapDistance) // Snap to bottom edge
          particleWindowPos.y = SCR_HEIGHT - particleWindowSize.y;

       if(originalparticlewindowpos.x != particleWindowPos.x || originalparticlewindowpos.y != originalparticlewindowpos.y)
         ImGui::SetWindowPos(particleWindowPos);
        ImGui::End();
        
        // render
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        cube.render(cubeShader, cubeposition, camera, SCR_WIDTH, SCR_HEIGHT, window, mouseX, mouseY, ishovering, isMoving);
        grid.renderGrid(gridShader, camera, window);
        particle.renderParticles(particleShader, ParticleAmount, particlespeed, camera, SCR_WIDTH, SCR_HEIGHT, height, RenderParticle, window);
        skybox.renderSkybox(skyboxshader, SCR_WIDTH, SCR_HEIGHT, window, camera);
        windowManager.render(camera, window);

        glm::vec3 Distance = housePosition - camera.Position;
        float scalarDistance = glm::length(Distance); // Calculate the magnitude of the distance vector

        // Define reasonable thresholds
        float modelRenderDistance = 20.0f; // Beyond this, render the model
        float imageRenderDistance = 5.0f; // Within this, render the image

        if (scalarDistance >= modelRenderDistance) {
          //playerManager.customRenderModel(animator, camera, 1.1f, 1.0f, housePosition, SCR_WIDTH, SCR_HEIGHT, playerModel, PlayerShader);
        } else {
          imageManager.render(camera, housePosition, SCR_WIDTH, SCR_HEIGHT);
        }
        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

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

    if (glfwGetKey(window, glfwGetKeyScancode(GLFW_KEY_LEFT_SHIFT)) == GLFW_PRESS) {
      camera.MovementSpeed += 1.0f;
    }

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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
