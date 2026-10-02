#include "bird.h"
#include "Camera.h"
#include "Shader.h"
#include "Variables.h"
#include "textureManager.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <cassert>
#include <filesystem>
#include <glad/glad.h>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <string>
#include <vector>

// Simple 2D quad (0–1 space)
static float quadVerts[] = {
    // Pos     // UV
    0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,

    0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
};

BirdTrail::BirdTrail(Shader &shader, unsigned int maxTrailLength, float size)
    : shader(shader), trailLimit(maxTrailLength), size(size) {

  texture = loadTexture((PathManager::texturePath + "bird.png").c_str());
  shader.LoadShaders((PathManager::shaderPath + "bird.vs").c_str(),
                     (PathManager::shaderPath + "bird.fs").c_str());

  init();
}

void BirdTrail::init() {
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0); // position
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);

  glEnableVertexAttribArray(1); // texcoord
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        (void *)(2 * sizeof(float)));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void BirdTrail::update(const glm::vec2 &screenPos) {
  trailPositions.push_back(screenPos);
  if (trailPositions.size() > trailLimit)
    trailPositions.erase(trailPositions.begin());
}

void BirdTrail::draw() {
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBlendEquation(GL_FUNC_ADD);
  shader.use();

  glm::mat4 projection = glm::ortho(0.0f, (float)PathManager::SCR_WIDTH, 0.0f,
                                    (float)PathManager::SCR_HEIGHT);
  shader.setMat4("projection", projection);
  shader.setMat4("view", glm::mat4(1.0f));
  shader.setInt("birdTexture", 0);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  glBindVertexArray(VAO);

  for (const glm::vec2 &pos : trailPositions) {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f));
    model = glm::scale(model, glm::vec3(size, size, 1.0f));
    shader.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 6);
  }

  glBindVertexArray(0);
  glDisable(GL_BLEND);
}
