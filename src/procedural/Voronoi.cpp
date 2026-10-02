#include "Voronoi.h"
#include "Camera.h"
#include "Shader.h"
#include "Variables.h"
#include "textureManager.h"
#include <string>

#include <GLFW/glfw3.h>
#include <cassert>
#include <filesystem>
#include <glad/glad.h>
#include <glm/ext.hpp>
#include <iostream>
#include <vector>

float quadVertices[] = { // pos       // tex
    -1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,
    -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f};

Voronoi::Voronoi(int NUM_POINTS, const std::vector<glm::vec2> &seeds)
    : numPoints(NUM_POINTS), seedPoints(seeds) {
  // init shader
  voronoiShader.LoadShaders((PathManager::shaderPath + "vor.vs").c_str(),
                            (PathManager::shaderPath + "vor.fs").c_str());

  glGenVertexArrays(1, &quadVAO);
  glGenBuffers(1, &quadVBO);
  glBindVertexArray(quadVAO);
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
}

void Voronoi::setColors(const std::vector<glm::vec3> &colors) {
  seedColors = colors;
}

void Voronoi::drawVo(Camera &camera) {
  voronoiShader.use();
  voronoiShader.setMat4("projection",
                        static_cast<float>(PathManager::SCR_WIDTH) /
                            PathManager::SCR_HEIGHT);
  voronoiShader.setMat4("view", camera.GetViewMatrix());
  voronoiShader.setMat4("model", glm::mat4(1.0f));

  voronoiShader.setInt("numPoints", numPoints);
  voronoiShader.setVec2(
      "resolution", glm::vec2(PathManager::SCR_WIDTH, PathManager::SCR_HEIGHT));

  for (unsigned int i = 0; i < numPoints; i++) {
    voronoiShader.setVec2(("seedPoints[" + std::to_string(i) + "]").c_str(),
                          seedPoints[i]);
    voronoiShader.setVec3(("seedColors[" + std::to_string(i) + "]").c_str(),
                          seedColors[i]);
  }

  glBindVertexArray(quadVAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
}
