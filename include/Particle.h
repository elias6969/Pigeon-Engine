#ifndef PARTICLE_H
#define PARTICLE_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "Shader.h"
#include "Camera.h"
#include <GLFW/glfw3.h>

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
    void renderParticles(Camera &camera, bool RenderParticle, GLFWwindow* window);

private:
    GLuint ParticleVAO, ParticleVBO, ParticleTexture;
    int x; // Could be used as an index or counter.
};

#endif // PARTICLE_H
