/********************************************************
 *   PigeonEngine ScreenEffect
 *   Rewritten from scratch with extra pigeon-power!
 *******************************************************/

#include <cstdlib>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/fwd.hpp>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Pigeon includes
#include "ScreenEffect.h"
#include "Shader.h"
#include "Camera.h"
#include "filemanager.h"


//--------------------------------------------------
// Helper function: create a screen quad
//--------------------------------------------------
static void PigeonCreateQuad()
{
    // Positions and texture coordinates (full-screen quad)
    float pigeonQuad[] = {
        //  pos.x   pos.y   tex.x  tex.y
        -1.0f,  1.0f,       0.0f,  1.0f,
        -1.0f, -1.0f,       0.0f,  0.0f,
         1.0f, -1.0f,       1.0f,  0.0f,

        -1.0f,  1.0f,       0.0f,  1.0f,
         1.0f, -1.0f,       1.0f,  0.0f,
         1.0f,  1.0f,       1.0f,  1.0f
    };

    // Generate and bind the VAO & VBO
    glGenVertexArrays(1, &PigeonVars::gVAO);
    glGenBuffers(1, &PigeonVars::gVBO);

    glBindVertexArray(PigeonVars::gVAO);
    glBindBuffer(GL_ARRAY_BUFFER, PigeonVars::gVBO);

    // Send our vertices to the GPU
    glBufferData(GL_ARRAY_BUFFER, sizeof(pigeonQuad), pigeonQuad, GL_STATIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // Texture-coordinate attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // Unbind to keep things tidy
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

//--------------------------------------------------
// Create the Framebuffer and Renderbuffer
//--------------------------------------------------
static void PigeonCreateFramebuffer(unsigned int width, unsigned int height)
{
    // Framebuffer
    glGenFramebuffers(1, &PigeonVars::gFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, PigeonVars::gFrameBuffer);

    // Texture color attachment
    glGenTextures(1, &PigeonVars::gTextureColorBuffer);
    glBindTexture(GL_TEXTURE_2D, PigeonVars::gTextureColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Attach texture to the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           PigeonVars::gTextureColorBuffer,
                           0);

    // Depth & Stencil
    glGenRenderbuffers(1, &PigeonVars::gRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, PigeonVars::gRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                              GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER,
                              PigeonVars::gRBO);

    // Error check
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "[Pigeon] Oops! Framebuffer not complete!\n";

    // Unbind the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//--------------------------------------------------
// This initializes all the screen stuff
//--------------------------------------------------
void initScreen(const unsigned int SCR_WIDTH, const unsigned int SCR_HEIGHT)
{
    // 1) Create the full-screen quad
    PigeonCreateQuad();

    // 2) Create and compile the screen shader
    PigeonVars::ScreenShader.LoadShaders(
        (PigeonVars::ShaderPath + "screen.vs").c_str(),
        (PigeonVars::ShaderPath + "screen.fs").c_str()
    );
    PigeonVars::ScreenShader.use();
    // We'll sample from texture unit 0 by default
    PigeonVars::ScreenShader.setInt("screenTexture", 0);

    // 3) Setup the framebuffer
    PigeonCreateFramebuffer(SCR_WIDTH, SCR_HEIGHT);
}

//--------------------------------------------------
// Render the final effect using the custom FBO
//--------------------------------------------------
void RenderScreenEFT()
{
    // Bind the pigeon’s FBO
    glBindFramebuffer(GL_FRAMEBUFFER, PigeonVars::gFrameBuffer);

    // Activate our lovely screen shader
    PigeonVars::ScreenShader.use();
    // This time, let’s say we want to sample from texture unit 1
    PigeonVars::ScreenShader.setInt("screenTexture", 1);

    // Enable blending for alpha
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw that sweet full-screen quad
    glBindVertexArray(PigeonVars::gVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, PigeonVars::gTextureColorBuffer);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Turn off blending
    glDisable(GL_BLEND);

    // Unbind VAO just to be safe
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
