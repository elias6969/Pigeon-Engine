#include "ParticleManager.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdlib>   // for rand()
#include <iostream>
#include <ctime>
#include "Variables.h"

MesmerizingParticleSystem::MesmerizingParticleSystem(unsigned int maxParticles)
    : maxParticles(maxParticles)
{
    // Reserve space for particles.
    particles.resize(maxParticles);
    // Optionally initialize the random seed.
    std::srand(static_cast<unsigned>(std::time(0)));
}

MesmerizingParticleSystem::~MesmerizingParticleSystem() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void MesmerizingParticleSystem::init() {
    // --- Load Shaders ---
    // Make sure you have a shader setup (see shaders code further down).
    particleShader.LoadShaders((PathManager::shaderPath + "newParticle.vs").c_str(), (PathManager::shaderPath + "newParticle.fs").c_str());
    
    // --- Setup Buffer ---
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Allocate dynamic buffer (we update all particles every frame).
    glBufferData(GL_ARRAY_BUFFER, maxParticles * sizeof(multiParticle), nullptr, GL_DYNAMIC_DRAW);
    
    // Set attribute pointers.
    // Attribute 0: Position (vec3)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(multiParticle), (void*)offsetof(multiParticle, Position));
    // Attribute 1: Color (vec4)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(multiParticle), (void*)offsetof(multiParticle, Color));
    // Attribute 2: Size (float)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(multiParticle), (void*)offsetof(multiParticle, Size));
    
    glBindVertexArray(0);
    
    // Initialize all particles.
    for (auto &p : particles) {
        respawnParticle(p);
    }
}

void MesmerizingParticleSystem::respawnParticle(multiParticle &particle) {
    // Start near the origin with a random offset.
    float spread = 2.0f;
    particle.Position = glm::vec3(
        ((std::rand() % 100) / 100.0f - 0.5f) * spread,
        ((std::rand() % 100) / 100.0f - 0.5f) * spread,
        ((std::rand() % 100) / 100.0f - 0.5f) * spread
    );
    
    // Give a swirling velocity – random angle and moderate speed.
    float angle = (std::rand() % 360) * 3.14159f / 180.0f;
    float speed = ((std::rand() % 100) / 100.0f) * 0.5f;
    particle.Velocity = glm::vec3(cos(angle) * speed, ((std::rand() % 100) / 100.0f) * 0.5f, sin(angle) * speed);
    
    // A vibrant, random color.
    particle.Color = glm::vec4(
        (std::rand() % 100) / 100.0f,
        (std::rand() % 100) / 100.0f,
        (std::rand() % 100) / 100.0f,
        1.0f
    );
    
    // Lifespan between 3 and 4 seconds.
    particle.Life = 3.0f + ((std::rand() % 100) / 100.0f);
    
    // Random size (point sprite size).
    particle.Size = 5.0f + (std::rand() % 10);
}

void MesmerizingParticleSystem::update(float deltaTime) {
    // Update each particle.
    for (auto &particle : particles) {
        particle.Life -= deltaTime;
        if (particle.Life <= 0.0f) {
            respawnParticle(particle);
        } else {
            // Basic motion update.
            particle.Position += particle.Velocity * deltaTime;
            // Add swirling: modulate X by a sine wave that varies with time and position.
            float swirl = sin(glfwGetTime() + particle.Position.x * 5.0f) * 0.1f;
            particle.Position.x += swirl;
            // Fade the particle color (alpha) based on remaining life.
            particle.Color.a = particle.Life / 4.0f;
        }
    }
    
    // Update the VBO with new particle data.
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, maxParticles * sizeof(multiParticle), particles.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MesmerizingParticleSystem::render(Camera &camera) {
    particleShader.use();

    // Setup camera matrices.
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)PathManager::SCR_WIDTH / (float)PathManager::SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    particleShader.setMat4("projection", projection);
    particleShader.setMat4("view", view);

    // Render particles as GL_POINTS.
    glBindVertexArray(VAO);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glDrawArrays(GL_POINTS, 0, maxParticles);
    glBindVertexArray(0);
}

MultiEffectParticleSystem::MultiEffectParticleSystem(unsigned int maxParticles)
    : maxParticles(maxParticles), VAO(0), VBO(0), currentMode(ParticleEffectMode::BASIC)
{
    // Pre-allocate the particle array
    particles.resize(maxParticles);
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

MultiEffectParticleSystem::~MultiEffectParticleSystem()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void MultiEffectParticleSystem::init(ParticleEffectMode initialMode)
{
    currentMode = initialMode;
    loadShadersForMode();

    // Generate and bind VAO, VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // We’ll update the buffer each frame with new particle data.
    glBufferData(GL_ARRAY_BUFFER, maxParticles * sizeof(multiParticle), nullptr, GL_DYNAMIC_DRAW);

    // Layout:  (pos vec3) + (color vec4) + (size float) => total 8 floats
    //  - aPos: location=0 (vec3)
    //  - aColor: location=1 (vec4)
    //  - aSize: location=2 (float)
    glEnableVertexAttribArray(0); // aPos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(multiParticle), (void*)offsetof(multiParticle, Position));

    glEnableVertexAttribArray(1); // aColor
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(multiParticle), (void*)offsetof(multiParticle, Color));

    glEnableVertexAttribArray(2); // aSize
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(multiParticle), (void*)offsetof(multiParticle, Size));

    glBindVertexArray(0);

    // Spawn all particles initially
    for (auto &p : particles) {
        respawnParticle(p);
    }
}

void MultiEffectParticleSystem::setEffectMode(ParticleEffectMode mode)
{
    currentMode = mode;
    loadShadersForMode();
}

void MultiEffectParticleSystem::loadShadersForMode()
{
    // For example, if you keep your shader files in PathManager::shaderPath
    // And you have separate .vs/.fs for each effect.
    std::string vsPath, fsPath;
    switch (currentMode) {
    case ParticleEffectMode::BASIC:
        vsPath = PathManager::shaderPath + "basic_particle.vs";
        fsPath = PathManager::shaderPath + "basic_particle.fs";
        break;
    case ParticleEffectMode::NOISE_DISTORTION:
        vsPath = PathManager::shaderPath + "noise_particle.vs";
        fsPath = PathManager::shaderPath + "noise_particle.fs";
        break;
    case ParticleEffectMode::COLOR_CYCLE:
        vsPath = PathManager::shaderPath + "color_cycle_particle.vs";
        fsPath = PathManager::shaderPath + "color_cycle_particle.fs";
        break;
    default:
        // fallback
        vsPath = PathManager::shaderPath + "basic_particle.vs";
        fsPath = PathManager::shaderPath + "basic_particle.fs";
        break;
    }

    particleShader.LoadShaders(vsPath.c_str(), fsPath.c_str());
}

void MultiEffectParticleSystem::respawnParticle(multiParticle &p)
{
    // Basic random spawn around the origin
    float spread = 2.0f;
    p.Position = glm::vec3(
        ((std::rand() % 100) / 100.0f - 0.5f) * spread,
        ((std::rand() % 100) / 100.0f - 0.5f) * spread,
        ((std::rand() % 100) / 100.0f - 0.5f) * spread
    );

    // Swirling velocity
    float angle = (std::rand() % 360) * 3.14159f / 180.0f;
    float speed = (std::rand() % 100) / 100.0f * 0.5f;
    p.Velocity = glm::vec3(cos(angle)*speed, 0.1f, sin(angle)*speed);

    // Vibrant color
    p.Color = glm::vec4(
        (std::rand() % 100) / 100.0f,
        (std::rand() % 100) / 100.0f,
        (std::rand() % 100) / 100.0f,
        1.0f
    );

    // Lifespan between 3 and 5 seconds
    p.Life = 3.0f + (std::rand() % 100) / 100.0f * 2.0f;

    // Random point size
    p.Size = 5.0f + (std::rand() % 10);
}

void MultiEffectParticleSystem::update(float deltaTime)
{
    // For each particle, decrease life, reset if dead, otherwise move + do swirl
    for (auto &p : particles) {
        p.Life -= deltaTime;
        if (p.Life <= 0.0f) {
            respawnParticle(p);
        } else {
            // Move
            p.Position += p.Velocity * deltaTime;

            // swirl offset
            float swirl = sin((float)glfwGetTime() + p.Position.x * 3.0f) * 0.1f;
            p.Position.x += swirl;

            // fade alpha
            p.Color.a = p.Life / 5.0f; // fade across 5 secs
        }
    }

    // Update VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, maxParticles * sizeof(multiParticle), particles.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MultiEffectParticleSystem::render(Camera &camera)
{
    particleShader.use();

    // Setup camera transforms
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)PathManager::SCR_WIDTH / (float)PathManager::SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    particleShader.setMat4("projection", projection);
    particleShader.setMat4("view", view);

    glBindVertexArray(VAO);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glDrawArrays(GL_POINTS, 0, maxParticles);
    glBindVertexArray(0);
}


