#ifndef IMGUIMANAGER_H
#define IMGUIMANAGER_H
#include <GLFW/glfw3.h>
#include <imgui.h>
void initIMGUI(GLFWwindow* window);
void CreationManager();
void AddEmojiRanges(ImVector<ImWchar>& ranges);
#endif
