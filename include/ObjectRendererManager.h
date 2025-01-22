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
    void InitParticle(Shader& shader);
    void renderParticles(Shader& shader, int &amount, int speed, Camera &camera,
        int screen_width, int screen_height, float &height, bool RenderParticle, GLFWwindow* windwow);
private:
    GLuint ParticleVAO, ParticleVBO, Particletexture;
    int x;
};

// Pigeon-Class for rendering cubes (Because even pigeons need boxes!)
class Cube {
public: 
    void loadCube(Shader& shader);                  // Load cube data (vertices, texture, etc.)
    void render(Shader& ourshader,
        glm::vec3& cubeposition, Camera& camera,
        int screen_width, int screen_height, GLFWwindow* window, double &mouseX, double &mouseY, bool& ishovering, bool& isMoving);              // Render the cube
private:
    GLuint VAO, VBO, texture;                  // Cube-specific OpenGL objects (VAO and VBO)
};

class SkyBox {
public:
    void texturebufferLoading(Shader& shader);
    void renderSkybox(Shader& shader, int screenWidth, int screenHeight, GLFWwindow* window, Camera &camera);
private:
    unsigned int vao, vbo;
};

class _VBO {
public:
    _VBO();
    void initializeData();
    void loadVBO();
    void render();
    void addRawData(const void* data, int size);

private:
    unsigned int _vboID;  // OpenGL VBO ID
    std::vector<char> _data;  // Raw data to be uploaded to GPU
    bool _isInitialized; // Flag to indicate if the VBO is initialized
};

class Cylinder
{
public:
    void cylinderloader(float radius, float height, int segments);
    ~Cylinder();
    void draw(Shader& shader) const;

private:
	bool isInitialized_{ false }; ///< Flag to indicate if the cylinder is initialized
    GLuint textureID_;
    unsigned int VAO_{ 0 };       ///< Vertex Array Object
    unsigned int VBO_{ 0 };       ///< Vertex Buffer Object
    unsigned int EBO_{ 0 };       ///< Element Buffer Object
    unsigned int indexCount_{ 0 };///< How many indices we have to draw
    glm::vec3 Position;
};

class Sphere {
public:
    Sphere();                         // Constructor to initialize sphere
	void initializeData();          // Initialize sphere data (vertices, texture, etc.)
    void loadSphere();                // Load sphere data (vertices, texture, etc.)
    void render();              // Render the sphere
private:
    GLuint VAO, VBO;                  // Sphere-specific OpenGL objects (VAO and VBO)
};

class Image {
public:
    Image();  // Constructor to load an image from a file
    void loadImage();                   // Load image data (texture)
    void render(Camera &camera, glm::vec3 &Position, unsigned int SCR_WIDTH, unsigned int SCR_HEIGHT);                // Render the image

private:
    GLuint textureID;                   
    GLuint vao, vbo, ebo;
    Shader shader;
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
