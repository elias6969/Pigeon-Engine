#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "GeoManager.h"
#include "Shader.h"
#include "filemanager.h"

void geoData::initGeometry()
{
    float points[] = {
        -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, // top-left
         0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // top-right
         0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // bottom-right
        -0.5f, -0.5f, 1.0f, 1.0f, 0.0f  // bottom-left
    };
    VirtualFileSystem vfs("../resources/");
    std::string shaderPath = vfs.getFullPath("Shaders/");
    shader.LoadAdvShaders((shaderPath + "GeoShader.vs").c_str(), (shaderPath + "GeoShader.fs").c_str(), (shaderPath + "GeoShader.gs").c_str());
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), &points, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0); 
}

void geoData::RenderGeo()
{
    shader.use();
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, 4);
}
