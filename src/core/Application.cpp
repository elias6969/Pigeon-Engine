#include "Application.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <imgui.h>
#include <imguiThemes.h>

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

Application::Application() = default;

Application::~Application() { Shutdown(); }

int Application::Run() {
  if (!Init())
    return -1;

  while (!glfwWindowShouldClose(m_Window)) {
    Update();
    Render();
  }

  Shutdown();
  return 0;
}

bool Application::Init() {
  InitWindow();
  if (!m_Window)
    return false;

  if (!InitOpenGL())
    return false;

  InitEngineResources();
  InitImGui();

  m_Camera.Position = loadPosition();
  return true;
}

void Application::InitWindow() {
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  m_Window = glfwCreateWindow(m_ScreenWidth, m_ScreenHeight, "Pigeon Engine",
                              nullptr, nullptr);
  if (!m_Window) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return;
  }

  glfwMakeContextCurrent(m_Window);
  glfwSetWindowUserPointer(m_Window, this);
  glfwSetFramebufferSizeCallback(m_Window, Callbacks::FramebufferSize);
  glfwSetCursorPosCallback(m_Window, Callbacks::MouseMove);
  glfwSetScrollCallback(m_Window, Callbacks::Scroll);
  glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

bool Application::InitOpenGL() {
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    return false;
  }

  glEnable(GL_DEPTH_TEST);
  return true;
}

void Application::InitEngineResources() {
  m_WindowManager.init();
  m_Grid.setupGrid();
  m_Grid.setupGridWater();
  m_MultiParticles.init(ParticleEffectMode::NOISE_DISTORTION);
  m_Skybox.texturebufferLoading(m_SkyboxShader);

  m_Sphere = std::make_unique<Sphere>(1.0f, countfirst, countsecond);
  m_Pyramid = std::make_unique<Pyramid>(m_PyramidShader, 2.0f, 3.0f);
  m_Pyramid->position = glm::vec3(10.0f, 0.0f, 0.0f);
  m_Fontmanager = std::make_unique<FontManager>();
}

void Application::InitImGui() {
#if REMOVE_IMGUI == 0
  ImGui::CreateContext();
  imguiThemes::embraceTheDarkness();

  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;
  io.FontGlobalScale = 1.0f;

  ImGuiStyle &style = ImGui::GetStyle();
  style.Colors[ImGuiCol_Text] = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
  style.Colors[ImGuiCol_DockingEmptyBg].w = 0.0f;

  ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
  ImGui_ImplOpenGL3_Init("#version 330");
#endif
}

void Application::Shutdown() {
  if (!m_Window)
    return;

#if REMOVE_IMGUI == 0
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
#endif

  glfwDestroyWindow(m_Window);
  m_Window = nullptr;
  glfwTerminate();
}

void Application::Update() {
  const float currentFrame = static_cast<float>(glfwGetTime());
  m_DeltaTime = currentFrame - m_LastFrame;
  m_LastFrame = currentFrame;
  m_FrameTimeMs = m_DeltaTime * 1000.0f;

  m_Callbacks.ProcessInput(*this, m_DeltaTime);
}

void Application::Render() {
  BeginImGuiFrame();
  RenderDebugWindow();
  RenderScene();
  RenderImGui();

  glfwSwapBuffers(m_Window);
  glfwPollEvents();
}

void Application::BeginImGuiFrame() {
#if REMOVE_IMGUI == 0
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(),
                                ImGuiDockNodeFlags_PassthruCentralNode);
#endif
}

void Application::RenderDebugWindow() {
#if REMOVE_IMGUI == 0
  ImGuiIO &io = ImGui::GetIO();
  const EditorSceneStats sceneStats = GetEditorSceneStats();

  ImGui::Begin("Pigeon Engine Debug");

  if (ImGui::CollapsingHeader("Performance", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Text("FPS: %.1f", io.Framerate);
    ImGui::Text("Frame Time: %.3f ms", m_FrameTimeMs);
    ImGui::Text("Delta Time: %.5f s", m_DeltaTime);
  }

  if (ImGui::CollapsingHeader("Viewport", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Text("Window Size: %d x %d", m_ScreenWidth, m_ScreenHeight);
    ImGui::Text("Framebuffer: %u x %u", PathManager::SCR_WIDTH,
                PathManager::SCR_HEIGHT);
    ImGui::Text("Mouse: %.2f, %.2f", m_MouseX, m_MouseY);
    ImGui::Checkbox("Camera Cursor Released", &m_IsOutCamera);
    ImGui::Text("Camera Capture Toggle: F release, G capture");
  }

  if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Text("Position: %.2f, %.2f, %.2f", m_Camera.Position.x,
                m_Camera.Position.y, m_Camera.Position.z);
    ImGui::Text("Front: %.2f, %.2f, %.2f", m_Camera.Front.x, m_Camera.Front.y,
                m_Camera.Front.z);
    ImGui::Text("Yaw/Pitch: %.2f / %.2f", m_Camera.Yaw, m_Camera.Pitch);
    ImGui::Text("Zoom: %.2f", m_Camera.Zoom);
    ImGui::Text("Speed: %.2f", m_Camera.MovementSpeed);
    if (ImGui::Button("Save Camera Position"))
      savePosition(m_Camera.Position);
  }

  if (ImGui::CollapsingHeader("Editor Scene", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Text("Objects: %d", sceneStats.objects);
    ImGui::Text("Cubes: %d", sceneStats.cubes);
    ImGui::Text("Images: %d", sceneStats.images);
    ImGui::Text("Particles: %d", sceneStats.particles);
    ImGui::Text("Models: %d", sceneStats.models);
    ImGui::Text("Water Grids: %d", sceneStats.waterGrids);
    ImGui::Checkbox("Hovering Cube", &m_IsHovering);
    ImGui::Checkbox("Moving", &m_IsMoving);
    ImGui::Checkbox("Collided with Cube", &m_Collided);
    ImGui::ColorEdit3("Sphere color", spherecolor);
    ImGui::InputInt("Sphere count first", &countfirst);
    ImGui::InputInt("Sphere count second", &countsecond);
  }

  ImGui::End();
#endif
}

void Application::RenderScene() {
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  ScreenSizeConfiguration(m_Window, m_ScreenWidth, m_ScreenHeight);
  CreationManager(m_Window, m_CubeShader, m_Camera, m_ScreenWidth,
                  m_ScreenHeight, m_MouseX, m_MouseY, m_IsHovering,
                  m_IsMoving);

  m_Grid.renderGrid(m_Camera, m_Window);
  m_WindowManager.render(m_Camera, m_Window);
  m_Skybox.renderSkybox(m_SkyboxShader, m_ScreenWidth, m_ScreenHeight,
                        m_Window, m_Camera);

  if (m_Pyramid)
    m_Pyramid->draw(m_Camera);

  glfwGetCursorPos(m_Window, &m_MouseX, &m_MouseY);
  m_MouseX = std::clamp(m_MouseX, 0.0, static_cast<double>(m_ScreenWidth));
  m_MouseY = std::clamp(m_MouseY, 0.0, static_cast<double>(m_ScreenHeight));

  const float flipY = m_ScreenHeight - static_cast<float>(m_MouseY);

  glm::mat4 projection =
      glm::perspective(glm::radians(m_Camera.Zoom),
                       static_cast<float>(m_ScreenWidth) /
                           static_cast<float>(m_ScreenHeight),
                       0.1f, 100.0f);
  glm::mat4 view = m_Camera.GetViewMatrix();

  m_Sphere->Draw(view, projection, glm::vec3(5.0f), glm::vec3(spherecolor[0], spherecolor[1], spherecolor[2]));

  if (m_Fontmanager)
    RenderEditorText(*m_Fontmanager, m_ScreenWidth, m_ScreenHeight);
}

void Application::RenderImGui() {
#if REMOVE_IMGUI == 0
  ImGui::Render();
  int displayW;
  int displayH;
  glfwGetFramebufferSize(m_Window, &displayW, &displayH);
  glViewport(0, 0, displayW, displayH);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
}
