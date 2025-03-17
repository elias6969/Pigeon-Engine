#ifndef SKYBOX_H
#define SKYBOX_H

#include <glad/glad.h>
#include "Shader.h"
#include "Camera.h"
#include <GLFW/glfw3.h>

class SkyBox {
public:
    void texturebufferLoading(Shader& shader);
    void renderSkybox(Shader& shader, int screenWidth, int screenHeight, GLFWwindow* window, Camera &camera);
private:
    unsigned int vao, vbo;
};

#endif 
