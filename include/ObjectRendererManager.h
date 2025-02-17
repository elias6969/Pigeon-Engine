#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <stb_image.h>
#include <iostream>
#include <vector>
#include "Shader.h"
#include "Camera.h"
#include "vao_manager.h"


bool CheckCollision(glm::vec3 &Position1, glm::vec3 &Position2, float size);

class Grid {
public:
    std::vector<float> generateGrid(float size, float spacing);

    void setupGrid(Shader &shader, float size, float spacing);

    void renderGrid(Shader& shader, Camera& camera, GLFWwindow* window);
private: 
    GLuint gridVAO = 0, gridVBO = 0;
    std::vector<float> gridVertices;
};

class Particle {
public:
    Shader shader;
    glm::vec3 Position;
    glm::vec3 Rotation;
    int ParticleAmount;
    float Height;
    float Speed;
    const char* texturePath;
    void InitParticle();
    void renderParticles(Camera &camera,
        int screen_width, int screen_height, bool RenderParticle, GLFWwindow* windwow);
private:
    GLuint ParticleVAO, ParticleVBO, Particletexture;
    int x;
};

// Pigeon-Class for rendering cubes (Because even pigeons need boxes!)

class Cube {
private:
    GLuint VAO, VBO, texture; 
public:
    Shader shader;
    glm::vec3 Position; 
    glm::vec3 Rotation; 
    float Alpha;
    glm::vec3 size;      
    float r, g, b;      
    const char* texturePath;

    void loadCube();
    void render(Camera& camera,
                int screenWidth,
                int screenHeight,
                GLFWwindow* window,
                double &mouseX, double &mouseY,
                bool &ishovering, bool &isMoving);
};

class SkyBox {
public:
    void texturebufferLoading(Shader& shader);
    void renderSkybox(Shader& shader, int screenWidth, int screenHeight, GLFWwindow* window, Camera &camera);
private:
    unsigned int vao, vbo;
};

class Image {
public:
    Shader shader;
    const char* imagePath;
    glm::vec3 Position;
    glm::vec3 Rotation;
    float Alpha, r,b,g;
    glm::vec3 size;
    Image();  // Constructor to load an image from a file
    void loadImage();                   // Load image data (texture)
    void render(Camera &camera, unsigned int SCR_WIDTH, unsigned int SCR_HEIGHT);                // Render the image

private:
    GLuint textureID;                   
    GLuint vao, vbo, ebo;
};

class TransparentWindow
{
public:
  void init();
  void render(Camera& camera, GLFWwindow* window);
private:
  unsigned int VAO, VBO, EBO, texture;
  VAOManager vaomanger;
  Shader shader;
};
