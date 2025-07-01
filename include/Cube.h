#ifndef CUBE_H
#define CUBE_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "Shader.h"
#include "Camera.h"
#include <GLFW/glfw3.h>


class Cube {
public:
    static Shader shader;
    glm::vec3 Position;
    glm::vec3 Rotation;
    float Alpha;
    float time;
    glm::vec3 size;
    float r, g, b;
    const char* texturePath;

    void loadCube();
    void render(Camera& camera,
                GLFWwindow* window,
                double &mouseX, double &mouseY,
                bool &ishovering, bool &isMoving);
private:
    GLuint VAO, VBO, texture;
};

#endif 
