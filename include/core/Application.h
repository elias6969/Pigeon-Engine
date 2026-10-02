#ifndef APPLICATION_H
#define APPLICATION_H

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <memory>

#include "Callbacks.h"
#include "Camera.h"
#include "Cube.h"
#include "Grid.h"
#include "IMGUIManager.h"
#include "Image.h"
#include "Model.h"
#include "Particle.h"
#include "ParticleManager.h"
#include "Shader.h"
#include "SkyBox.h"
#include "Sphere.h"
#include "Variables.h"
#include "WindowModule.h"
#include "bird.h"
#include "modelLoader.h"
#include "pyramid.h"

class Application {
public:
  Application();
  ~Application();

  int Run();

private:
  friend class Callbacks;

  bool Init();
  void InitWindow();
  bool InitOpenGL();
  void InitEngineResources();
  void InitImGui();
  void Shutdown();

  void Update();
  void Render();
  void BeginImGuiFrame();
  void RenderDebugWindow();
  void RenderScene();
  void RenderImGui();

  GLFWwindow *m_Window = nullptr;
  Callbacks m_Callbacks;

  int m_ScreenWidth = 1200;
  int m_ScreenHeight = 1000;

  Camera m_Camera{glm::vec3(0.0f, 0.0f, 3.0f)};
  float m_LastMouseX = m_ScreenWidth / 2.0f;
  float m_LastMouseY = m_ScreenHeight / 2.0f;
  bool m_FirstMouse = true;

  bool m_IsHovering = false;
  bool m_IsOutCamera = false;
  bool m_IsMoving = false;
  bool m_Collided = false;

  double m_MouseX = 0.0;
  double m_MouseY = 0.0;

  float m_DeltaTime = 0.0f;
  float m_LastFrame = 0.0f;
  float m_FrameTimeMs = 0.0f;

  float spherecolor[3] = {0.0f, 0.0f, 0.0f};
  int countfirst{10}, countsecond{5};

  OpenGLState m_OpenGLState{{0.1f, 0.2f, 0.3f, 1.0f}, true};

  Cube m_Cube;
  Grid m_Grid;
  SkyBox m_Skybox;
  TransparentWindow m_WindowManager;
  CharacterModel m_PlayerManager;
  Image m_ImageManager;
  Model m_PlayerModel;
  MultiEffectParticleSystem m_MultiParticles{5000};

  Shader m_CubeShader;
  Shader m_SkyboxShader;
  Shader m_ParticleShader;
  Shader m_PlayerShader;
  Shader m_BoxShader;
  Shader m_BirdShader;
  Shader m_PyramidShader;

  std::unique_ptr<Pyramid> m_Pyramid;
  std::unique_ptr<Sphere> m_Sphere;
};

#endif
