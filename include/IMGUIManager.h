#ifndef IMGUIMANAGER_H
#define IMGUIMANAGER_H
#include <glm/fwd.hpp>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <imgui.h>
#include "Shader.h"
#include "Camera.h"
#include "iostream"

void initIMGUI(GLFWwindow* window);

void UpdateManager(GLFWwindow* window, Camera &camera);
void CreationManager(GLFWwindow* window, Shader &shader, Camera &camera, int scrwidth, int scrheight, double &mouseX, double &mouseY, bool &ishovering, bool &isMoving);

void AddEmojiRanges(ImVector<ImWchar>& ranges);
struct OpenGLState {
    float clearColor[4];
    bool depthTestEnabled;
};

void stateGame(OpenGLState &opengl);

void loadScene();
void saveScene();
void savePosition(const glm::vec3 &position);
glm::vec3 loadPosition();

#endif
