#include "Callbacks.h"

#include "Application.h"

#include <glad/glad.h>

void Callbacks::FramebufferSize(GLFWwindow *window, int width, int height) {
  if (height == 0)
    return;

  glViewport(0, 0, width, height);

  auto *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
  if (!app)
    return;

  app->m_ScreenWidth = width;
  app->m_ScreenHeight = height;
  PathManager::SCR_WIDTH = static_cast<unsigned int>(width);
  PathManager::SCR_HEIGHT = static_cast<unsigned int>(height);
}

void Callbacks::MouseMove(GLFWwindow *window, double xposIn, double yposIn) {
  auto *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
  if (!app || app->m_IsOutCamera)
    return;

  const float xpos = static_cast<float>(xposIn);
  const float ypos = static_cast<float>(yposIn);

  if (app->m_FirstMouse) {
    app->m_LastMouseX = xpos;
    app->m_LastMouseY = ypos;
    app->m_FirstMouse = false;
  }

  const float xoffset = xpos - app->m_LastMouseX;
  const float yoffset = app->m_LastMouseY - ypos;

  app->m_LastMouseX = xpos;
  app->m_LastMouseY = ypos;

  app->m_Camera.ProcessMouseMovement(xoffset, yoffset);
}

void Callbacks::Scroll(GLFWwindow *window, double, double yoffset) {
  auto *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
  if (!app)
    return;

  app->m_Camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void Callbacks::ProcessInput(Application &app, float deltaTime) {
  GLFWwindow *window = app.m_Window;

  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  app.m_Camera.MovementSpeed =
      (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ? 10.0f : 2.5f;

  if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !app.m_IsOutCamera) {
    app.m_IsOutCamera = true;
    app.m_FirstMouse = true;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }

  if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && app.m_IsOutCamera) {
    app.m_IsOutCamera = false;
    app.m_FirstMouse = true;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  }

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    app.m_Camera.ProcessKeyboard(FORWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    app.m_Camera.ProcessKeyboard(BACKWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    app.m_Camera.ProcessKeyboard(LEFT, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    app.m_Camera.ProcessKeyboard(RIGHT, deltaTime);

  if (app.m_IsOutCamera)
    glfwGetCursorPos(window, &app.m_MouseX, &app.m_MouseY);
}
