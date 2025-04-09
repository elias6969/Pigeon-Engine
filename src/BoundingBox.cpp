#include <glad/glad.h>
#include "BoundingBox.h"
#include "filemanager.h"

BoundingBox::BoundingBox(glm::vec3 min, glm::vec3 max) : min(min), max(max), VAO(0), VBO(0) {
    setupBuffers();
    VirtualFileSystem resources("../assets/");
    std::string shaderPath = resources.getFullPath("Shaders/");
    shader.LoadShaders((shaderPath + "grid.vs").c_str(), (shaderPath + "grid.fs").c_str());
}

BoundingBox::~BoundingBox() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

std::vector<float> generateBoxVertices(glm::vec3 min, glm::vec3 max) {
    return {
        // Bottom square
        min.x, min.y, min.z, max.x, min.y, min.z,  
        max.x, min.y, min.z, max.x, min.y, max.z,  
        max.x, min.y, max.z, min.x, min.y, max.z,  
        min.x, min.y, max.z, min.x, min.y, min.z,  

        // Top square
        min.x, max.y, min.z, max.x, max.y, min.z,  
        max.x, max.y, min.z, max.x, max.y, max.z,  
        max.x, max.y, max.z, min.x, max.y, max.z,  
        min.x, max.y, max.z, min.x, max.y, min.z,  

        // Vertical lines
        min.x, min.y, min.z, min.x, max.y, min.z,  
        max.x, min.y, min.z, max.x, max.y, min.z,  
        max.x, min.y, max.z, max.x, max.y, max.z,  
        min.x, min.y, max.z, min.x, max.y, max.z  
    };
}

void BoundingBox::setupBuffers() {
    vertices = generateBoxVertices(min, max);

    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void BoundingBox::update(glm::vec3 newMin, glm::vec3 newMax) {
    min = newMin;
    max = newMax;
    setupBuffers();
}

void BoundingBox::render(Camera& camera) {
    if (VAO == 0) return;

    shader.use();

    glm::mat4 view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    glm::mat4 model = glm::mat4(1.0f);

    shader.setMat4("model", model);
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);

    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(vertices.size() / 3));
    glBindVertexArray(0);
}
