#ifndef MESMERIZING_PARTICLE_SYSTEM_H
#define MESMERIZING_PARTICLE_SYSTEM_H

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "Shader.h"    
#include "Camera.h"   


struct multiParticle {
    glm::vec3 Position;
    glm::vec3 Velocity;
    glm::vec4 Color;
    float Life;   // in seconds
    float Size;   // point size
};

enum class ParticleEffectMode {
    BASIC,
    NOISE_DISTORTION,
    COLOR_CYCLE,
};

class MesmerizingParticleSystem {
public:
    // Constructor: maxParticles defines how many particles are managed.
    MesmerizingParticleSystem(unsigned int maxParticles = 5000);
    ~MesmerizingParticleSystem();

    // Initializes the particle system (allocates buffers, loads shaders).
    void init();

    // Updates each particle based on elapsed time and resets dead ones.
    void update(float deltaTime);

    // Renders the particles using the given camera.
    void render(Camera &camera);

private:
    // Helper to (re)spawn a particle with randomized, swirling properties.
    void respawnParticle(multiParticle &particle);

    std::vector<multiParticle> particles;
    unsigned int maxParticles;
    GLuint VAO, VBO;   // Buffer objects for particle attributes.
    Shader particleShader;
};



class MultiEffectParticleSystem {
public:
    MultiEffectParticleSystem(unsigned int maxParticles = 2000);
    ~MultiEffectParticleSystem();

    // Call once to allocate buffers and set an initial effect mode.
    void init(ParticleEffectMode initialMode);

    // Update each particle per frame.
    void update(float deltaTime);

    // Render all particles.
    void render(Camera &camera);

    // Switch effect mode (changes shaders / logic).
    void setEffectMode(ParticleEffectMode mode);

private:
    // Internal method to (re)spawn a particle with random properties.
    void respawnParticle(multiParticle &p);

    // Internal method to load the correct shaders for the current mode.
    void loadShadersForMode();

private:
    std::vector<multiParticle> particles;
    unsigned int maxParticles;
    GLuint VAO, VBO;

    // The current effect / visual style.
    ParticleEffectMode currentMode;

    // The current shader for rendering.
    Shader particleShader;
};

#endif // MESMERIZING_PARTICLE_SYSTEM_H
