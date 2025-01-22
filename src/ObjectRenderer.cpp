#include "ObjectRendererManager.h"
#include <GL/gl.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <iostream>
#include <bits/stdc++.h>
#include "Shader.h"
#include "Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "filemanager.h"
#include "vao_manager.h"
#include <stdexcept>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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
    Sphere sphere;
    Particle particle;
    SkyBox skybox;
    
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

void Particle::InitParticle(Shader& shader)
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
    unsigned char* data = stbi_load((Var::texturePath + "Break.jpg").c_str(), &width, &height, &nrChannels, 0);

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
        throw std::runtime_error("Failed to load texture: " + Var::texturePath + "cube.jpeg");
    }

    // Free the texture image memory after uploading it to the GPU.
    stbi_image_free(data);

    // Compile and link shaders for the particle system.
    shader.LoadShaders((Var::shaderPath + "normalCube.vs").c_str(), (Var::resourcePath + "normalCube.fs").c_str());

    // Activate the shader program and set the texture uniform.
    shader.use();
    shader.setInt("texture1", 0);
}

void UpdateParticleBuffers(std::vector<glm::vec3> particlePositions) {
    GLuint ParticleVBO, ParticleVAO;
    glBindBuffer(GL_ARRAY_BUFFER, ParticleVBO);
    glBufferData(GL_ARRAY_BUFFER, particlePositions.size() * sizeof(glm::vec3), particlePositions.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Particle::renderParticles(Shader& shader, int &amount, int speed, Camera &camera,
    int screen_width, int screen_height, float &height, bool RenderParticle, GLFWwindow* window)
{
    if(!RenderParticle)
      return ;
    if(amount < 0) amount = 0;

    // Ensure the particle system is initialized only once.
    static std::vector<glm::vec3> particlePositions; //Storing particle positions
    static std::vector<glm::vec3> particleVelocities; //Storing Particle velocity 
    static std::vector<glm::vec3> test;
    static bool initialized = false;

    if (!initialized) {
        initialized = true;
    if (particlePositions.size() != static_cast<size_t>(amount)) {
        // Resize vectors to match the desired particle amount.
        particlePositions.resize(amount);
        particleVelocities.resize(amount);
        
        // Initialize particles with random positions and velocities.
        for (int i = 0; i < amount; ++i) {
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
    }

    // Update particle positions
    for (int i = 0; i < amount; ++i) {
        particlePositions[i] += particleVelocities[i] * 0.01f; // Simulate time delta

        // Reset particles that fall below a threshold or exceed a height limit
        if (particlePositions[i].y < -0.5f || particlePositions[i].y > height) {
            particlePositions[i] = glm::vec3(
                (float(rand()) / float(RAND_MAX)) * 2.0f - 1.0f,  // Reset X position
                (float(rand()) / float(RAND_MAX)) * 2.0f,         // Reset Y position
                (float(rand()) / float(RAND_MAX)) * 2.0f - 1.0f   // Reset Z position
            );

            particleVelocities[i] = glm::vec3(
                (float(rand()) / float(RAND_MAX)) * 0.05f - 0.025f,  // X velocity
                (float(rand()) / float(RAND_MAX)) * 0.1f + 0.05f,    // Y velocity
                (float(rand()) / float(RAND_MAX)) * 0.05f - 0.025f   // Z velocity
            );
        }
    }

    // Check if the VAO and texture are initialized
    if (ParticleVAO == 0 || Particletexture == 0) {
        throw std::runtime_error("Particle system is not properly initialized. Ensure InitParticle() is called first.");
    }

    // Enable blending for rendering transparent particles
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Activate the shader program
    shader.use();

    // Bind the particle texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Particletexture);

    // Set view and projection matrices
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)screen_width / screen_height, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);

    shader.setMat4("view", view);
    shader.setMat4("projection", projection);

    // Bind the VAO for particles
    glBindVertexArray(ParticleVAO);

    // Render each particle
    for (const auto& position : particlePositions) {
        // Create a transformation matrix for each particle
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::scale(model, glm::vec3(0.8f)); // Adjust particle size
        static float rotationAngle = 0.0f;
        rotationAngle+=0.05f;
        if(rotationAngle>=360.0f){
          rotationAngle -=360.0f;
        }
        model = glm::rotate(model, glm::radians(rotationAngle), glm::vec3(0.01f, 0.01f, 0.01f)); 
        shader.setMat4("model", model);

        // Render the particle quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    // Cleanup: Unbind the VAO and disable blending
    glBindVertexArray(0);
    glDisable(GL_BLEND);
    if(glfwWindowShouldClose(window)){
      glDeleteVertexArrays(1, &ParticleVAO);
      glDeleteBuffers(1, &ParticleVBO);
    }
}

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
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
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

void Cube::loadCube(Shader& shader) {
    float vertices[] = {
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

    glGenVertexArrays(1, &Var::cube.VAO);
    glGenBuffers(1, &Var::cube.VBO);

    glBindVertexArray(Var::cube.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, Var::cube.VBO);

    // Upload vertex data
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Vertex attributes setup
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Generate texture
    glGenTextures(1, &Var::cube.texture);
    glBindTexture(GL_TEXTURE_2D, Var::cube.texture);

    // Texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load image
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* datatexture = stbi_load((Var::texturePath + "cube.jpeg").c_str(), &width, &height, &nrChannels, 0);
    if (datatexture) {
        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;
        else
            throw std::runtime_error("Unsupported texture format");

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, datatexture);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        throw std::runtime_error("Failed to load texture");
    }
    stbi_image_free(datatexture);
    shader.LoadShaders((Var::shaderPath + "normalCube.vs").c_str(), (Var::shaderPath + "normalCube.fs").c_str());
    // Shader setup
    shader.use();
    shader.setInt("texture1", 0);
}

void Cube::render(Shader& ourshader,
    glm::vec3& cubeposition, Camera& camera,
    int screen_width, int screen_height, GLFWwindow* window, double &mouseX, double &mouseY, bool& ishovering, bool& isMoving) {
  
    // Activate texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Var::cube.texture);

    static float rotationAngle = 0.0f;
    rotationAngle+=0.05f;
    if(rotationAngle>=360.0f)
      rotationAngle -=360.0f;

    ourshader.use();

    // Setup matrices
    glm::mat4 model = glm::mat4(1.0f); 
    model = glm::translate(model, cubeposition); 
    model = glm::rotate(model, glm::radians(rotationAngle), glm::vec3(0.01f, 0.01f, 0.01f)); 
                                                                                 //
    glm::mat4 view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up); // View matrix
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)screen_width / (float)screen_height, 0.1f, 100.0f); // Projection matrix

    // Cube's 8 vertices (in world space, relative to the cube's center)
    glm::vec3 cubeVertices[8] = {
        cubeposition + glm::vec3(-0.5f, -0.5f, -0.5f),
        cubeposition + glm::vec3(0.5f, -0.5f, -0.5f),
        cubeposition + glm::vec3(0.5f,  0.5f, -0.5f),
        cubeposition + glm::vec3(-0.5f,  0.5f, -0.5f),
        cubeposition + glm::vec3(-0.5f, -0.5f,  0.5f),
        cubeposition + glm::vec3(0.5f, -0.5f,  0.5f),
        cubeposition + glm::vec3(0.5f,  0.5f,  0.5f),
        cubeposition + glm::vec3(-0.5f,  0.5f,  0.5f)
    };

    // Variables to store the min and max screen-space coordinates
    float minX = FLT_MAX, maxX = -FLT_MAX;
    float minY = FLT_MAX, maxY = -FLT_MAX;

    // Project each vertex to screen space
    for (int i = 0; i < 8; ++i) {
        glm::vec4 vertex = projection * view * glm::vec4(cubeVertices[i], 1.0f);

        // Convert to normalized device coordinates (NDC)
        glm::vec3 ndc = glm::vec3(vertex) / vertex.w;

        // Convert to screen space
        float screenX = (ndc.x * 0.5f + 0.5f) * screen_width;
        float screenY = (0.5f - ndc.y * 0.5f) * screen_height;

        // Update min/max bounds in screen space
        minX = std::min(minX, screenX);
        maxX = std::max(maxX, screenX);
        minY = std::min(minY, screenY);
        maxY = std::max(maxY, screenY);
    }

    // Calculate cube width and height in screen space
    float cubeScreenWidth = maxX - minX;
    float cubeScreenHeight = maxY - minY;

    // Check if the cursor is within the cube's bounds
    if (mouseX >= minX && mouseX <= maxX && mouseY >= minY && mouseY <= maxY) {
        ishovering = true;
    }
    else ishovering = false;

    // If hovering and left mouse button is pressed, enable cube dragging
    if (ishovering && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        static bool isDraggingInitialized = false; // Ensure drag initialization happens once

        if (!isMoving) {
            isMoving = true;
            isDraggingInitialized = false; // Reset initialization state for a new drag
        }

        if (isMoving) {
            static double lastMouseX;
            static double lastMouseY;

            // Initialize the drag state only once when the movement starts
            if (!isDraggingInitialized) {
                lastMouseX = mouseX;
                lastMouseY = mouseY;
                isDraggingInitialized = true; // Prevent reinitialization during dragging
            }

            // Calculate the mouse movement delta
            double deltaX = mouseX - lastMouseX;
            double deltaY = mouseY - lastMouseY;

            // Update the last mouse position
            lastMouseX = mouseX;
            lastMouseY = mouseY;
            glm::vec3 cameraRight = glm::normalize(glm::cross(camera.Front, camera.Up)); // Right vector
            glm::vec3 cameraUp = glm::normalize(camera.Up);                            // Up vector
            glm::vec3 cameraForward = glm::normalize(camera.Front);                   // Forward vector

            // Adjust movement for screen-to-world scale (tweak 0.01f for speed adjustment)
            float scale = 0.01f;
            cubeposition += cameraRight * static_cast<float>(deltaX) * scale; // Move along camera's right
            cubeposition -= cameraUp * static_cast<float>(deltaY) * scale;
        }
    }
    else {
        // Reset dragging state if the mouse button is released
        isMoving = false;
    }

    // If right mouse button is pressed, exit "select cube mode"
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        isMoving = false;  // Stop moving the cube
    }

    // Pass matrices to the shader
    ourshader.setMat4("model", model);
    ourshader.setMat4("view", view);
    ourshader.setMat4("projection", projection);
    glUniform1i(glGetUniformLocation(ourshader.ID, "Basetexture"), 0);

    // Render cube
    glBindVertexArray(Var::cube.VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);    // OpenGL rendering code here

    if(glfwWindowShouldClose(window)){
      glDeleteVertexArrays(1, &VAO);
      glDeleteBuffers(1, &VBO);
    }
}


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

// Sphere class definition
Sphere::Sphere() {
    // Initialize sphere properties
}

void Sphere::render() {
    std::cout << "Rendering Sphere" << std::endl;
    // OpenGL rendering code here
}

_VBO::_VBO() : _vboID(0), _isInitialized(false) {}

// Initialize the VBO (generate a buffer ID)
void _VBO::initializeData() {
    if (!_isInitialized) {
        glGenBuffers(1, &_vboID); // Generate the VBO
        _isInitialized = true;
        std::cout << "VBO initialized with ID: " << _vboID << std::endl;
    }
    else {
        std::cout << "VBO is already initialized." << std::endl;
    }
}

// Add raw data to the internal buffer (to be loaded into the GPU later)
void _VBO::addRawData(const void* data, int size) {
    if (!_isInitialized) {
        std::cerr << "Error: VBO not initialized. Call initializeData() first." << std::endl;
        return;
    }

    // Append raw data to the _data vector
    const char* rawData = static_cast<const char*>(data);
    _data.insert(_data.end(), rawData, rawData + size);
    std::cout << "Added " << size << " bytes of raw data to VBO buffer." << std::endl;
}

// Upload the raw data to the GPU
void _VBO::loadVBO() {
    if (!_isInitialized) {
        std::cerr << "Error: VBO not initialized. Call initializeData() first." << std::endl;
        return;
    }

    if (_data.empty()) {
        std::cerr << "Error: No data to upload to VBO." << std::endl;
        return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, _vboID);  // Bind the VBO
    glBufferData(GL_ARRAY_BUFFER, _data.size(), _data.data(), GL_STATIC_DRAW); // Upload data
    glBindBuffer(GL_ARRAY_BUFFER, 0);  // Unbind the VBO

    std::cout << "VBO loaded with " << _data.size() << " bytes of data." << std::endl;

    // Clear the local data buffer after uploading
    _data.clear();
}

// Render the VBO (bind it for use in rendering)
void _VBO::render() {
    if (!_isInitialized) {
        std::cerr << "Error: VBO not initialized. Call initializeData() first." << std::endl;
        return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, _vboID);  // Bind the VBO
    // Rendering logic is dependent on how the data is used, typically done elsewhere
    std::cout << "VBO bound for rendering with ID: " << _vboID << std::endl;
    glBindBuffer(GL_ARRAY_BUFFER, 0);  // Unbind after rendering
}

bool hasPositions(){
	return true;
}
bool hasNormals() {
	return true;
}
bool hasTexCoords() {
	return true;
}
bool hasTangents() {
	return true;
}
bool hasBitangents() {
	return true;
}
bool hasColors() {
	return true;
}
bool hasBones() {
	return true;
}
bool hasIndices() {
	return true;
}
bool hasMaterialIDs() {
	return true;
}
bool hasAnimations() {
	return true;
}

// Helper constant for PI
static constexpr float PI = 3.14159265359f;

void Cylinder::cylinderloader(float radius, float height, int segments)
{
    Position = glm::vec3(5.0f, 0.0f, 0.0f);
    std::vector<float> positions; // each vertex = 5 floats: (x, y, z, u, v)
    positions.reserve((2 + 2 * segments) * 5);

    std::vector<unsigned int> indices;

    float halfHeight = height * 0.5f;
    float angleStep = 2.0f * PI / float(segments);

    // -- TOP CENTER --
    positions.push_back(0.0f);        // x
    positions.push_back(+halfHeight); // y
    positions.push_back(0.0f);        // z
    positions.push_back(0.5f);        // u
    positions.push_back(0.5f);        // v

    // -- TOP CIRCLE --
    for (int i = 0; i < segments; ++i)
    {
        float angle = i * angleStep;
        float x = radius * std::cos(angle);
        float z = radius * std::sin(angle);

        positions.push_back(x);
        positions.push_back(+halfHeight);
        positions.push_back(z);

        // Radial mapping for top circle
        float u = 0.5f + (x / (2.0f * radius));
        float v = 0.5f + (z / (2.0f * radius));
        positions.push_back(u);
        positions.push_back(v);
    }

    unsigned int bottomCenterIndex = (unsigned int)(positions.size() / 5);
    // -- BOTTOM CENTER --
    positions.push_back(0.0f);
    positions.push_back(-halfHeight);
    positions.push_back(0.0f);
    positions.push_back(0.5f);
    positions.push_back(0.5f);

    unsigned int bottomRingStart = bottomCenterIndex + 1;
    // -- BOTTOM CIRCLE --
    for (int i = 0; i < segments; ++i)
    {
        float angle = i * angleStep;
        float x = radius * std::cos(angle);
        float z = radius * std::sin(angle);

        positions.push_back(x);
        positions.push_back(-halfHeight);
        positions.push_back(z);

        float u = 0.5f + (x / (2.0f * radius));
        float v = 0.5f + (z / (2.0f * radius));
        positions.push_back(u);
        positions.push_back(v);
    }

    // -- BUILD INDICES --
    // Top cap
    for (int i = 1; i <= segments; ++i)
    {
        int next = (i == segments) ? 1 : (i + 1);
        indices.push_back(0);     // top center
        indices.push_back(i);
        indices.push_back(next);
    }

    // Bottom cap
    for (int i = bottomRingStart; i < bottomRingStart + segments; ++i)
    {
        int next = (i == bottomRingStart + segments - 1) ? bottomRingStart : (i + 1);
        indices.push_back(bottomCenterIndex);
        indices.push_back(i);
        indices.push_back(next);
    }

    // Side
    for (int i = 1; i <= segments; ++i)
    {
        int top_i = i;
        int top_next = (i == segments) ? 1 : (i + 1);

        int bottom_i = bottomRingStart + (i - 1);
        int bottom_next = bottomRingStart + (top_next - 1);

        indices.push_back(top_i);
        indices.push_back(bottom_i);
        indices.push_back(top_next);

        indices.push_back(top_next);
        indices.push_back(bottom_i);
        indices.push_back(bottom_next);
    }

    indexCount_ = (unsigned int)indices.size();

    // == CREATE / BIND VAO, VBO, EBO ==
    glGenVertexArrays(1, &VAO_);
    glGenBuffers(1, &VBO_);
    glGenBuffers(1, &EBO_);

    glBindVertexArray(VAO_);

    // VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO_);
    glBufferData(GL_ARRAY_BUFFER,
        positions.size() * sizeof(float),
        positions.data(),
        GL_STATIC_DRAW);

    // EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        indices.size() * sizeof(unsigned int),
        indices.data(),
        GL_STATIC_DRAW);

    // Positions => layout(location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        5 * sizeof(float),
        (void*)0
    );

    // Texture coords => layout(location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        2,
        GL_FLOAT,
        GL_FALSE,
        5 * sizeof(float),
        (void*)(3 * sizeof(float))
    );

    glBindVertexArray(0);

    // Load your texture here and store the returned ID
    // Replace with the desired texture path
    textureID_ = loadTexture((Var::resourcePath + "Textures/bottom.jpg").c_str());
    isInitialized_ = true;
}

Cylinder::~Cylinder()
{
    glDeleteBuffers(1, &VBO_);
    glDeleteBuffers(1, &EBO_);
    glDeleteVertexArrays(1, &VAO_);

    // If you'd like to also delete the texture to avoid leaks:
    // glDeleteTextures(1, &textureID_);
}

void Cylinder::draw(Shader& shader) const
{
	if (!isInitialized_)
	{
		std::cerr << "Cylinder is not initialized!" << std::endl;
		return;
	}

    // Use our shader
    shader.use(); // Ensure we're using this shader

    // Bind our VAO
    glBindVertexArray(VAO_);
    glm::vec3 cylinderPosition = Position; // Your cylinder position
    shader.setVec3("position", cylinderPosition);
    // Set active texture unit to 0 and bind our cylinder texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID_);

    // Let the shader know that 'myTexture' is at texture unit 0
    int textureLoc = glGetUniformLocation(shader.ID, "myTexture");
    glUniform1i(textureLoc, 0);

    // Draw filled
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, nullptr);

    // Optional wireframe overlay
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, nullptr);

    // Reset to fill mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glBindVertexArray(0);
}

// Image class definition
Image::Image() 
{

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

    textureID = loadTexture((Var::texturePath + "masuka.jpg").c_str());
    shader.LoadShaders((Var::shaderPath + "plane.vs").c_str(), (Var::shaderPath + "plane.fs").c_str());

}

void Image::render(Camera &camera, glm::vec3 &Position,  unsigned int SCR_WIDTH, unsigned int SCR_HEIGHT) {
    shader.use();

    // Set up transformations
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, Position);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);

    shader.setMat4("model", model);
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    shader.setInt("texture1", 0);

    // Draw plane
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

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

    // (Optional) Test draw - You can remove or relocate this as desired
    // glBindVertexArray(VAO);
    // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    // glBindVertexArray(0);
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
        1200.0f / 1000.0f,
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
