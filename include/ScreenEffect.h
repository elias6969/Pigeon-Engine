#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "filemanager.h"
#include "Shader.h"

namespace PigeonVars
{
    // Buffers & objects
    static unsigned int gVAO = 0;
    static unsigned int gVBO = 0;
    static unsigned int gFrameBuffer = 0;
    static unsigned int gRBO = 0; // depth/stencil
    static GLuint       gTextureColorBuffer = 0;

    // Our sweet pigeon-approved shader
    static Shader ScreenShader;

    // Resource file paths
    static VirtualFileSystem ResourcePath("../resources/");
    static std::string ShaderPath = ResourcePath.getFullPath("Shaders/");
}
void initScreen(const unsigned int SCR_WIDTH, const unsigned int SCR_HEIGHT);

void RenderScreenEFT();



