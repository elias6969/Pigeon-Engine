#ifndef TRANSPARENTWINDOW_H
#define TRANSPARENTWINDOW_H

#include <glad/glad.h>
#include "Shader.h"
#include "Camera.h"
#include <GLFW/glfw3.h>
#include "vao_manager.h"

class TransparentWindow {
public:
    void init();
    void render(Camera& camera, GLFWwindow* window);
private:
    unsigned int VAO, VBO, EBO, texture;
    VAOManager vaomanger;
    Shader shader;
};

#endif // TRANSPARENTWINDOW_H
