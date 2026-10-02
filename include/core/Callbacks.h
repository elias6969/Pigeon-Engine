#ifndef CALLBACKS_H
#define CALLBACKS_H

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

class Application;

class Callbacks {
public:
  static void FramebufferSize(GLFWwindow *window, int width, int height);
  static void MouseMove(GLFWwindow *window, double xpos, double ypos);
  static void Scroll(GLFWwindow *window, double xoffset, double yoffset);

  void ProcessInput(Application &app, float deltaTime);
};

#endif
