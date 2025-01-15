#include <cstdlib>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/fwd.hpp>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "ScreenEffect.h"
#include "Shader.h"
#include "Camera.h"
#include "filemanager.h"

struct variables
{
    unsigned int vao, vbo;
    GLuint textureColorbuffer;
    Shader ScreenShader;
    VirtualFileSystem resourcespath;

    // Constructor
    variables()
        : vao(0), vbo(0), resourcespath("../resources/")
    {
    }

    std::string shaderpath = resourcespath.getFullPath("Shaders/");
} var;

void initScreen(const unsigned int SCR_WIDTH, const unsigned int SCR_HEIGHT)
{
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    
    glGenVertexArrays(1, &var.vao);
    glGenBuffers(1, &var.vbo);

    glBindVertexArray(var.vao);
    glBindBuffer(GL_ARRAY_BUFFER, var.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    
    // position attrib
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    // texCoord attrib (2 floats offset)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // Load and use the screen shader
    var.ScreenShader.LoadShaders((var.shaderpath + "screen.vs").c_str(),
                                 (var.shaderpath + "screen.fs").c_str());
    var.ScreenShader.use();
    var.ScreenShader.setInt("screenTexture", 0);

    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // Create a color attachment texture
    glGenTextures(1, &var.textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, var.textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           var.textureColorbuffer,
                           0);

    // Create a renderbuffer object for depth and stencil attachments
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    // Unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderScreenEFT()
{
    var.ScreenShader.use();
    var.ScreenShader.setInt("screenTexture", 1);
    glBindVertexArray(var.vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, var.textureColorbuffer);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
