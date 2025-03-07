#include "ObjectRendererManager.h"
#include <GL/gl.h>
#include <cstdlib>
#include <dirent.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include <iostream>
#include <bits/stdc++.h>
#include "Shader.h"
#include "Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "filemanager.h"
#include "modelLoader.h"
#include "vao_manager.h"
#include <iterator>
#include <stdexcept>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

/*
▗▄▄▄  ▗▄▄▄▖ ▗▄▄▖▗▖    ▗▄▖ ▗▄▄▖  ▗▄▖▗▄▄▄▖▗▄▄▄▖ ▗▄▖ ▗▖  ▗▖
▐▌  █ ▐▌   ▐▌   ▐▌   ▐▌ ▐▌▐▌ ▐▌▐▌ ▐▌ █    █  ▐▌ ▐▌▐▛▚▖▐▌
▐▌  █ ▐▛▀▀▘▐▌   ▐▌   ▐▛▀▜▌▐▛▀▚▖▐▛▀▜▌ █    █  ▐▌ ▐▌▐▌ ▝▜▌
▐▙▄▄▀ ▐▙▄▄▖▝▚▄▄▖▐▙▄▄▖▐▌ ▐▌▐▌ ▐▌▐▌ ▐▌ █  ▗▄█▄▖▝▚▄▞▘▐▌  ▐▌
*/
unsigned int loadCubemap(std::vector<std::string> faces);


unsigned int loadTexture(const char* filePath)
{
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filePath, &width, &height, &nrChannels, 0);
    if (!data)
    {
        std::cerr << "Failed to load texture: " << filePath << std::endl;
        return 0;
    }else {
      std::cout << "successfully loaded texture" << filePath << std::endl;
    }

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    // Set up texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Determine correct format (RGB or RGBA)
    GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

    // Upload the image data to the GPU
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
        GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Cleanup
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texID;
}

namespace Var
{
    //Class definitions
    Cube cube;
    Grid grid;
    Particle particle;
    SkyBox skybox;
    
    int SCR_WIDTH, SCR_HEIGHT;
    //Path to different folders
    VirtualFileSystem vfs("../resources");
    std::string resourcePath = vfs.getFullPath("Shaders/");
    std::string texturePath = vfs.getFullPath("Textures/");
    std::string shaderPath = vfs.getFullPath("Shaders/");
    std::string cubemappath = vfs.getFullPath("cubemap/");
    
    std::vector<std::string> faces
    {
        cubemappath + "right.jpg",
        cubemappath + "left.jpg",
        cubemappath + "up.jpg",
        cubemappath + "down.jpg",
        cubemappath + "front.jpg",
        cubemappath + "back.jpg"
    };

    unsigned int cubemapTexture = 0;
};

void ScreenSize(int &screenwidth, int &screenheight)
{
    Var::SCR_HEIGHT = screenheight;
    Var::SCR_WIDTH = screenwidth;
}
bool CheckCollision(glm::vec3 &Position1, glm::vec3 &Position2, float size)
{
  bool collisionX = Position1.x + size >= Position2.x && Position2.x + size >= Position1.x;
  bool collisionY = Position1.y + size >= Position2.y && Position2.y + size >= Position1.y;
  bool collisionZ = Position1.z + size >= Position2.z && Position2.z + size >= Position1.z;
  return collisionX && collisionY && collisionZ;
}

/*
▗▄▄▖  ▗▄▖ ▗▄▄▖▗▄▄▄▖▗▄▄▄▖ ▗▄▄▖▗▖   ▗▄▄▄▖
▐▌ ▐▌▐▌ ▐▌▐▌ ▐▌ █    █  ▐▌   ▐▌   ▐▌   
▐▛▀▘ ▐▛▀▜▌▐▛▀▚▖ █    █  ▐▌   ▐▌   ▐▛▀▀▘
▐▌   ▐▌ ▐▌▐▌ ▐▌ █  ▗▄█▄▖▝▚▄▄▖▐▙▄▄▖▐▙▄▄▖
*/

void Particle::InitParticle()
{
    // Define vertices for a small quad representing a particle.
    float particleVertices[] = {
        // Positions       // Texture Coords
        0.05f,  0.05f, 0.0f,  1.0f, 1.0f, // Top Right
        0.05f, -0.05f, 0.0f,  1.0f, 0.0f, // Bottom Right
       -0.05f, -0.05f, 0.0f,  0.0f, 0.0f, // Bottom Left

        0.05f,  0.05f, 0.0f,  1.0f, 1.0f, // Top Right
       -0.05f, -0.05f, 0.0f,  0.0f, 0.0f, // Bottom Left
       -0.05f,  0.05f, 0.0f,  0.0f, 1.0f  // Top Left
    };

    // Generate and bind the Vertex Array Object (VAO) for the particle.
    glGenVertexArrays(1, &ParticleVAO);
    glBindVertexArray(ParticleVAO);

    // Generate and bind the Vertex Buffer Object (VBO) for the particle.
    glGenBuffers(1, &ParticleVBO);
    glBindBuffer(GL_ARRAY_BUFFER, ParticleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particleVertices), particleVertices, GL_STATIC_DRAW);

    // Define vertex attribute layout: positions (location = 0) and texture coordinates (location = 1).
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Unbind the VBO and VAO to prevent accidental modifications.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Generate a texture for the particle and bind it.
    glGenTextures(1, &Particletexture);
    glBindTexture(GL_TEXTURE_2D, Particletexture);

    // Set texture wrapping and filtering parameters.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load the texture image using stb_image.
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(false);
    std::string fullPath;
    std::cout << "TEXTURE::PATH::" << texturePath << std::endl;


    if(texturePath == nullptr || strlen(texturePath) == 0) {
      std::cout << "TEXTURE::PATH::NULL OR EMPTY\n";
      texturePath = "/home/lighht19/Documents/Pigeon-Engine/resources/Textures/masuka.jpg";
    }
    std::cout << (Var::texturePath + "Break.jpg").c_str() << std::endl;
    std::cout << "PARTICLE::TEXTURE::PATH::" << texturePath << std::endl;
    unsigned char* data = stbi_load(texturePath, &width, &height, &nrChannels, 0);

    // Check if the texture was successfully loaded.
    if (data) {
        GLenum format;
        // Determine texture format based on number of channels.
        switch (nrChannels) {
        case 1: format = GL_RED; break;
        case 3: format = GL_RGB; break;
        case 4: format = GL_RGBA; break;
        default:
            stbi_image_free(data);
            throw std::runtime_error("Unsupported texture format");
        }

        // Upload the texture data to the GPU and generate mipmaps.
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        // Handle texture loading failure.
        throw std::runtime_error("Failed to load texture: for particle");
    }

    // Free the texture image memory after uploading it to the GPU.
    stbi_image_free(data);

    // Compile and link shaders for the particle system.
    shader.LoadShaders((Var::shaderPath + "particle.vs").c_str(), (Var::resourcePath + "particle.fs").c_str());

    // Activate the shader program and set the texture uniform.
    shader.use();
    shader.setInt("texture1", 0);
    //assert(ParticleAmount >= 0 && "ParticleAmount should not be negative!");
    const int min_particles = 10;
    //FIXME
    //if(ParticleAmount >= min_particles){
      //ParticleAmount = min_particles;
    //}
}

void UpdateParticleBuffers(std::vector<glm::vec3> particlePositions) {
    GLuint ParticleVBO, ParticleVAO;
    glBindBuffer(GL_ARRAY_BUFFER, ParticleVBO);
    glBufferData(GL_ARRAY_BUFFER, particlePositions.size() * sizeof(glm::vec3), particlePositions.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

//RENDER PARTICLE
void Particle::renderParticles(Camera &camera, bool RenderParticle, GLFWwindow* window)
{
    //OPTIMIZE
    if (!RenderParticle)
        return;
    static bool initialized = false;
    std::vector<glm::vec3> particlePositions;
    particlePositions.push_back(Position);
    std::vector<glm::vec3> particleVelocities;

    // Resize vectors properly every frame if needed
    if (particlePositions.size() != static_cast<size_t>(ParticleAmount)) {
        //particlePositions.clear();
        //particleVelocities.clear();
        particlePositions.resize(ParticleAmount);
        particleVelocities.resize(ParticleAmount);

        for (int i = 0; i < ParticleAmount; ++i) {
            particlePositions[i] = glm::vec3(
                (float(rand()) / float(RAND_MAX)) * 2.0f - 1.0f,  // Random X position
                (float(rand()) / float(RAND_MAX)) * 2.0f,         // Random Y position
                (float(rand()) / float(RAND_MAX)) * 2.0f - 1.0f   // Random Z position
            );

            particleVelocities[i] = glm::vec3(
                (float(rand()) / float(RAND_MAX)) * 0.05f - 0.025f,  // X velocity
                (float(rand()) / float(RAND_MAX)) * 0.1f + 0.05f,    // Y velocity
                (float(rand()) / float(RAND_MAX)) * 0.05f - 0.025f   // Z velocity
            );
        }
    }

    // Update particle positions
    for (int i = 0; i < ParticleAmount; ++i) {
        particlePositions[i] += particleVelocities[i] * 0.01f;

        // Reset particles that fall below a threshold or exceed a height limit
        if (particlePositions[i].y < -0.5f || particlePositions[i].y > Height) {
            particlePositions[i] = glm::vec3(
                (float(rand()) / float(RAND_MAX)) * 2.0f - 1.0f,
                (float(rand()) / float(RAND_MAX)) * 2.0f,
                (float(rand()) / float(RAND_MAX)) * 2.0f - 1.0f
            );

            particleVelocities[i] = glm::vec3(
                (float(rand()) / float(RAND_MAX)) * 0.05f - 0.025f,
                (float(rand()) / float(RAND_MAX)) * 0.1f + 0.05f,
                (float(rand()) / float(RAND_MAX)) * 0.05f - 0.025f
            );
        }
    }

    // Check if the VAO and texture are initialized
    if (ParticleVAO == 0 || Particletexture == 0) {
        throw std::runtime_error("Particle system is not properly initialized. Ensure InitParticle() is called first.");
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Particletexture);

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)Var::SCR_WIDTH / (float)Var::SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);

    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    glBindVertexArray(ParticleVAO);

    for (const auto& position : particlePositions) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::scale(model, glm::vec3(0.8f));

        static float rotationAngle = 0.0f;
        rotationAngle += 0.05f;
        if (rotationAngle >= 360.0f) {
            rotationAngle -= 360.0f;
        }
        //model = glm::rotate(model, glm::radians(rotationAngle), glm::vec3(0.01f, 0.01f, 0.01f));

        shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    glBindVertexArray(0);
    glDisable(GL_BLEND);

    if (glfwWindowShouldClose(window)) {
        glDeleteVertexArrays(1, &ParticleVAO);
        glDeleteBuffers(1, &ParticleVBO);
    }
}

/*
 ▗▄▄▖▗▄▄▖ ▗▄▄▄▖▗▄▄▄  
▐▌   ▐▌ ▐▌  █  ▐▌  █ 
▐▌▝▜▌▐▛▀▚▖  █  ▐▌  █ 
▝▚▄▞▘▐▌ ▐▌▗▄█▄▖▐▙▄▄▀ 
*/
std::vector<float> Grid::generateGrid(float size, float spacing) 
{
    std::vector<float> vertices;

    for (float x = -size; x <= size; x += spacing) {
        vertices.push_back(x); vertices.push_back(0.0f); vertices.push_back(-size); // Start
        vertices.push_back(x); vertices.push_back(0.0f); vertices.push_back(size);  // End
    }

    // Lines parallel to X-axis
    for (float z = -size; z <= size; z += spacing) {
        vertices.push_back(-size); vertices.push_back(0.0f); vertices.push_back(z); // Start
        vertices.push_back(size); vertices.push_back(0.0f); vertices.push_back(z);  // End
    }

    return vertices;
}

void Grid::setupGrid(Shader &shader, float size, float spacing) {
    if (gridVAO != 0) {
        glDeleteVertexArrays(1, &gridVAO);
        glDeleteBuffers(1, &gridVBO);
    }

    gridVertices = generateGrid(size, spacing);

    glGenVertexArrays(1, &gridVAO);
    glGenBuffers(1, &gridVBO);

    glBindVertexArray(gridVAO);

    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    shader.LoadShaders((Var::shaderPath + "grid.vs").c_str(), (Var::shaderPath + "grid.fs").c_str());
}

void Grid::renderGrid(Shader& shader, Camera& camera, GLFWwindow* window) {
    if (gridVAO == 0) {
        std::cerr << "Grid VAO is not initialized! Call setupGrid() first.\n";
        return;
    }

    shader.use();

    glm::mat4 view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)Var::SCR_WIDTH/(float)Var::SCR_HEIGHT, 0.1f, 100.0f);
    // Set the model matrix (identity matrix for now)
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);

    glBindVertexArray(gridVAO);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(gridVertices.size() / 3));
    glBindVertexArray(0);
    if(glfwWindowShouldClose(window)){
      glDeleteVertexArrays(1, &gridVAO);
      glDeleteBuffers(1, &gridVBO);
    }
}

/* 
 ▗▄▄▖▗▖ ▗▖▗▄▄▖ ▗▄▄▄▖
▐▌   ▐▌ ▐▌▐▌ ▐▌▐▌   
▐▌   ▐▌ ▐▌▐▛▀▚▖▐▛▀▀▘
▝▚▄▄▖▝▚▄▞▘▐▙▄▞▘▐▙▄▄▖
*/

void Cube::loadCube() {
    float vertices[] = {
        // 36 vertices, each with:
        //   3 floats for position (x, y, z)
        //   2 floats for texture coords (u, v)
        // => 5 floats per vertex
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };

    // Generate and bind VAO/VBO
    glGenVertexArrays(1, &Var::cube.VAO);
    glGenBuffers(1, &Var::cube.VBO);

    glBindVertexArray(Var::cube.VAO);

    // Make sure you bind Var::cube.VBO, not just VBO
    glBindBuffer(GL_ARRAY_BUFFER, Var::cube.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute: layout(location = 0)
    // Each vertex is 5 floats (3 pos + 2 tex coords)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinates attribute: layout(location = 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // (No color attribute pointer here, since we only have 3 + 2 = 5 floats)

    // ================== Generate and Load Texture ==================
    glGenTextures(1, &Var::cube.texture);
    glBindTexture(GL_TEXTURE_2D, Var::cube.texture);

    // Texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    
    if(!texturePath){
        texturePath = "/home/lighht19/Documents/Pigeon-Engine/resources/Textures/masuka.jpg";
    }
    texture = loadTexture(texturePath);

    // ================== Compile and Use Shader ==================
    shader.LoadShaders((Var::shaderPath + "normalCube.vs").c_str(),
                       (Var::shaderPath + "normalCube.fs").c_str());
    shader.use();
    shader.setInt("texture1", 0);
std::cout << "Loading texture for image: " << texturePath 
          << " | Assigned textureID: " << texture << std::endl;
    // Optional: set default transforms, color values, etc.
    size = glm::vec3(1.0f);
    r = 0.0f; 
    g = 1.5f; 
    b = 3.0f; 
}


void Cube::render(Camera& camera,
                  GLFWwindow* window,
                  double &mouseX,
                  double &mouseY,
                  bool &ishovering,
                  bool &isMoving)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Bind texture to texture unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Use our shader
    shader.use();
    shader.setFloat("r", r);
    shader.setFloat("g", g);
    shader.setFloat("b", b);
    shader.setFloat("Alpha", Alpha);
    shader.setInt("Basetexture", 0); 

    // ----------------------------------------------------
    // MODEL MATRIX: Translate -> Rotate -> Scale
    // ----------------------------------------------------
    glm::mat4 model(1.0f);
    model = glm::translate(model, Position);

    // Rotate around X, Y, Z by editor-controlled degrees
    model = glm::rotate(model, glm::radians(Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, size);

    // VIEW & PROJECTION
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        (float)Var::SCR_WIDTH / (float)Var::SCR_HEIGHT,
        0.1f,
        100.0f
    );

    // Send matrices to shader
    shader.setMat4("model", model);
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);

    // ---------------------------------------------------------------
    // Bounding Box for Hover Detection (in screen space)
    // ---------------------------------------------------------------
    // Start with local-space corners of the cube [-0.5..0.5]
    glm::vec3 localCorners[8] = {
        { -0.5f, -0.5f, -0.5f },
        {  0.5f, -0.5f, -0.5f },
        {  0.5f,  0.5f, -0.5f },
        { -0.5f,  0.5f, -0.5f },
        { -0.5f, -0.5f,  0.5f },
        {  0.5f, -0.5f,  0.5f },
        {  0.5f,  0.5f,  0.5f },
        { -0.5f,  0.5f,  0.5f },
    };

    float minX = FLT_MAX, maxX = -FLT_MAX;
    float minY = FLT_MAX, maxY = -FLT_MAX;

    for (int i = 0; i < 8; ++i)
    {
        // Transform corner to world space, then clip space
        glm::vec4 worldPos = model * glm::vec4(localCorners[i], 1.0f);
        glm::vec4 clipSpace = projection * view * worldPos;

        // Convert to normalized device coords
        glm::vec3 ndc = glm::vec3(clipSpace) / clipSpace.w;

        // Convert to screen coords
        float screenX = (ndc.x * 0.5f + 0.5f) * Var::SCR_WIDTH;
        float screenY = (0.5f - ndc.y * 0.5f) * Var::SCR_HEIGHT;

        // Expand bounding rect
        minX = std::min(minX, screenX);
        maxX = std::max(maxX, screenX);
        minY = std::min(minY, screenY);
        maxY = std::max(maxY, screenY);
    }

    // Check if mouse is within bounding box
    if (mouseX >= minX && mouseX <= maxX && 
        mouseY >= minY && mouseY <= maxY)
    {
        ishovering = true;
    }
    else {
        ishovering = false;
    }

    // ------------------------------------------
    // Dragging logic (move cube with mouse)
    // ------------------------------------------
    if (ishovering && (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS))
    {
        static bool dragInit = false; // tracks initialization of this drag

        if (!isMoving) {
            isMoving = true;
            dragInit = false;
        }
        if (isMoving) {
            static double lastX, lastY;
            if (!dragInit) {
                lastX = mouseX;
                lastY = mouseY;
                dragInit = true;
            }
            double dX = (mouseX - lastX);
            double dY = (mouseY - lastY);
            lastX = mouseX;
            lastY = mouseY;

            // Move along camera vectors
            float moveScale = 0.01f;
            glm::vec3 camRight = glm::normalize(glm::cross(camera.Front, camera.Up));
            glm::vec3 camUp    = glm::normalize(camera.Up);

            Position += camRight * static_cast<float>(dX) * moveScale;
            Position -= camUp   * static_cast<float>(dY) * moveScale;
        }
    }
    else {
        // Mouse not pressed or no longer hovering
        isMoving = false;
    }

    // Right-click ends movement
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        isMoving = false;
    }

    // ------------------------------------------
    // Render the cube
    // ------------------------------------------
    glBindVertexArray(Var::cube.VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Cleanup if the window is closing
    if (glfwWindowShouldClose(window))
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
    glDisable(GL_BLEND);
}
/*
 ▗▄▄▖▗▖ ▗▖▗▖  ▗▖▗▄▄▖  ▗▄▖ ▗▖  ▗▖
▐▌   ▐▌▗▞▘ ▝▚▞▘ ▐▌ ▐▌▐▌ ▐▌ ▝▚▞▘ 
 ▝▀▚▖▐▛▚▖   ▐▌  ▐▛▀▚▖▐▌ ▐▌  ▐▌  
▗▄▄▞▘▐▌ ▐▌  ▐▌  ▐▙▄▞▘▝▚▄▞▘▗▞▘▝▚▖
*/
unsigned int loadCubemap(std::vector<std::string> faces) {
    GLuint textureID = 0;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    
    stbi_set_flip_vertically_on_load(false);
    int width, height, nrComponents;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);
        if (data) {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;
            else {
                stbi_image_free(data);
                throw std::runtime_error("Unsupported texture format");
            }

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Set the base level and max level for mipmaps
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);

    return textureID;
}

void SkyBox::texturebufferLoading(Shader& shader) {
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    // Load shaders once
    static bool isTextureLoaded = false;  // Flag to check if the texture has been loaded
    if (!isTextureLoaded) {
        // Load the cubemap texture only once
        Var::cubemapTexture = loadCubemap(Var::faces);
        isTextureLoaded = true;  // Mark the texture as loaded
    }

    static bool isShaderLoaded = false;  // Flag to check if shaders have been loaded
    if (!isShaderLoaded) {
        // Load shaders only once
        shader.LoadShaders((Var::resourcePath + "skybox.vs").c_str(), (Var::resourcePath + "skybox.fs").c_str());
        isShaderLoaded = true;  // Mark shaders as loaded
    }
    stbi_set_flip_vertically_on_load(false);
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    
    // Set the active texture unit
    shader.use();
    shader.setInt("skybox", 0); // Set the texture unit in the shader}

    glBindTexture(GL_TEXTURE_CUBE_MAP, Var::cubemapTexture);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
}

void SkyBox::renderSkybox(Shader& shader, int screenWidth, int screenHeight, GLFWwindow* window, Camera& camera) {
    glDepthFunc(GL_LEQUAL);
    shader.use();

    // Remove translation from the view matrix
    glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);

    // Pass matrices to the shader
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    float currentTime = glfwGetTime();
    glUniform1f(glGetUniformLocation(shader.ID, "time"), currentTime);
    
    // Bind the skybox VAO and texture
    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, Var::cubemapTexture);

    // Draw the skybox (36 vertices: 6 faces, 2 triangles per face, 3 vertices per triangle)
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Unbind the VAO after drawing
    glBindVertexArray(0);

    // Restore the depth function to default (for other objects)
    glDepthFunc(GL_LESS);
    if(glfwWindowShouldClose(window)){
      glDeleteVertexArrays(1, &vao);
      glDeleteBuffers(1, &vbo);
    }
}

// Image class definition
Image::Image() 
{
  std::cout << "LOADING::IMAGE" << std::endl;
}

void Image::loadImage()
{
    float planeVertices[] = {
        // Positions          // Texture Coords
        -0.5f,  0.0f, -0.5f,  0.0f, 0.0f,  // Bottom-left
         0.5f,  0.0f, -0.5f,  1.0f, 0.0f,  // Bottom-right
         0.5f,  0.0f,  0.5f,  1.0f, 1.0f,  // Top-right
        -0.5f,  0.0f,  0.5f,  0.0f, 1.0f   // Top-left
    };

    unsigned int planeIndices[] = {
        0, 1, 2, // First triangle
        2, 3, 0  // Second triangle
    };

    // Generate and bind VAO, VBO, and EBO
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeIndices), planeIndices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinates attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    if(imagePath == NULL){
      textureID = loadTexture("example");
      std::cout << "ERROR::NO-IMAGE-PATH-WAS-ASSIGNED" << std::endl;
    }else{
      textureID = loadTexture(imagePath);
    }
    shader.LoadShaders((Var::shaderPath + "plane.vs").c_str(), (Var::shaderPath + "plane.fs").c_str());
    std::cout << "LOADING::COMPLETE" << std::endl;
}

void Image::render(Camera &camera) {
    shader.use();

    // Set up transformations
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, Position);
    model = glm::rotate(model, glm::radians(Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)Var::SCR_WIDTH / (float)Var::SCR_HEIGHT, 0.1f, 100.0f);
    glm::vec4 colorTint = glm::vec4(r,b,g,Alpha);
    shader.setMat4("model", model);
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    shader.setVec4("colorTint", colorTint);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    shader.setInt("texture1", 0);

    // Draw plane
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

/*
▗▖ ▗▖▗▄▄▄▖▗▖  ▗▖▗▄▄▄   ▗▄▖ ▗▖ ▗▖ ▗▄▄▖
▐▌ ▐▌  █  ▐▛▚▖▐▌▐▌  █ ▐▌ ▐▌▐▌ ▐▌▐▌   
▐▌ ▐▌  █  ▐▌ ▝▜▌▐▌  █ ▐▌ ▐▌▐▌ ▐▌ ▝▀▚▖
▐▙█▟▌▗▄█▄▖▐▌  ▐▌▐▙▄▄▀ ▝▚▄▞▘▐▙█▟▌▗▄▄▞▘
*/
namespace windowsHolder
{
  std::vector<glm::vec3> windows 
  {
    glm::vec3(-1.5f, 0.0f, -0.48f),
    glm::vec3( 1.5f, 0.0f, 0.51f),
    glm::vec3( 0.0f, 0.0f, 0.7f),
    glm::vec3(-0.3f, 0.0f, -2.3f),
    glm::vec3( 0.5f, 0.0f, -0.6f) 
  };
};

void TransparentWindow::init()
{
    // Enable blending once, coo coo
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_SRC_COLOR);
    glBlendEquation(GL_FUNC_SUBTRACT);

    // We define a quad with 4 vertices, plus indices for 2 triangles
    // positions       // texture coords
    std::vector<float> vertices = {
         0.5f,  0.5f, 0.0f,   1.0f, 1.0f,  // Top right
         0.5f, -0.5f, 0.0f,   1.0f, 0.0f,  // Bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f,  // Bottom left
        -0.5f,  0.5f, 0.0f,   0.0f, 1.0f   // Top left
    };
    std::vector<unsigned int> indices = {
        0, 1, 2,
        2, 3, 0
    };

    // Load and compile our window shaders
    shader.LoadShaders((Var::shaderPath + "window.vs").c_str(),
                       (Var::shaderPath + "window.fs").c_str());

    // Generate buffers and arrays
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 vertices.size() * sizeof(float),
                 vertices.data(),
                 GL_STATIC_DRAW);

    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size() * sizeof(unsigned int),
                 indices.data(),
                 GL_STATIC_DRAW);

    // Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0
    );
    // Texture coords
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
        (void*)(3 * sizeof(float))
    );
    glBindVertexArray(0); // Unbind for safety
    // Load texture
    texture = loadTexture((Var::texturePath + "window.png").c_str());
    glDisable(GL_BLEND);
}

void TransparentWindow::render(Camera& camera, GLFWwindow* window)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_SRC_COLOR);
    glBlendEquation(GL_FUNC_SUBTRACT);
    std::vector<std::pair<float, glm::vec3>> sorted;
    sorted.reserve(windowsHolder::windows.size());

    for (auto& wPos : windowsHolder::windows)
    {
        float distance = glm::length(camera.Position - wPos);
        sorted.push_back(std::make_pair(distance, wPos));
    }

    std::sort(sorted.begin(), sorted.end(),
              [](auto &a, auto &b){ return a.first > b.first; });

    shader.use();
    


    glm::mat4 projection = glm::perspective(
        glm::radians(camera.Zoom),
        (float)Var::SCR_WIDTH / (float)Var::SCR_HEIGHT,
        0.1f,
        100.0f
    );
    glm::mat4 view = camera.GetViewMatrix();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);
    glBindTexture(GL_TEXTURE_2D, texture);

    for (auto &win : sorted)
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, win.second);
        shader.setMat4("model", model);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    if (glfwWindowShouldClose(window))
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }
    glDisable(GL_BLEND);
}
