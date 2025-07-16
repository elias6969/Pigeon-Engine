#include "Variables.h"
#include "Voxel.h"
#include "filemanager.h"
#include "textureManager.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/common.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <stb_image.h>

void VoxelChunk::loadCubes() {
  float vertices[] = {
      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 0.0f,
      0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
      -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

      -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,
      0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
      -0.5f, 0.5f,  0.5f,  0.0f, 1.0f, -0.5f, -0.5f, 0.5f,  0.0f, 0.0f,

      -0.5f, 0.5f,  0.5f,  1.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 1.0f, 1.0f,
      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
      -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, 0.5f,  0.5f,  1.0f, 0.0f,

      0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
      0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 0.0f, 1.0f,
      0.5f,  -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 1.0f,
      0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,
      -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

      -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
      0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
      -0.5f, 0.5f,  0.5f,  0.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f};

  // Generate & bind VAO/VBO
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // ——— Load your four terrain textures ———
  textures[0] = loadTexture((PathManager::texturePath + "grass.jpg").c_str());
  textures[1] = loadTexture((PathManager::texturePath + "dirt.jpg").c_str());
  textures[2] = loadTexture((PathManager::texturePath + "stone.jpg").c_str());
  textures[3] = loadTexture((PathManager::texturePath + "bedrock.jpg").c_str());

  // ——— Compile & configure shader ———
  shader.LoadShaders((PathManager::shaderPath + "voxel.vs").c_str(),
                     (PathManager::shaderPath + "voxel.fs").c_str());
  shader.use();

  // tell each sampler which texture unit it's bound to
  shader.setInt("grassTexture", 0);
  shader.setInt("dirtTexture", 1);
  shader.setInt("stoneTexture", 2);
  shader.setInt("bedrockTexture", 3);

  // default voxel size
  size = glm::vec3(1.0f);
}
VoxelChunk::VoxelChunk(int sizeX, int sizeY, int sizeZ)
    : sizeX(sizeX), sizeY(sizeY), sizeZ(sizeZ) {
  generateChunk();
}

const std::vector<Voxel> &VoxelChunk::getVoxels() const { return voxels; }

void VoxelChunk::generateChunk() {
  voxels.clear();
  voxels.reserve(sizeX * sizeY * sizeZ);

  float scale = 0.4f;                    // Controls frequency
  float heightMultiplier = sizeY / 2.0f; // Controls max height

  for (int x = 0; x < sizeX; ++x) {
    for (int z = 0; z < sizeZ; ++z) {
      float heightValue = glm::perlin(glm::vec2(x * scale, z * scale));
      int maxY =
          static_cast<int>((heightValue + 1.0f) / 2.0f * heightMultiplier);

      for (int y = 0; y < maxY; ++y) {
        glm::vec3 pos(x, y, z);
        uint8_t textureType = 2;
        if (y > maxY - 2)
          textureType = 0;
        else if (y > maxY - 5)
          textureType = 1;
        else if (y == 0)
          textureType = 3;

        glm::vec3 color;
        switch (textureType) {
        case 0:
          color = glm::vec3(0.1f, 0.8f, 0.1f);
          break; // Grass
        case 1:
          color = glm::vec3(0.5f, 0.3f, 0.1f);
          break; // Dirt
        case 2:
          color = glm::vec3(0.3f, 0.3f, 0.3f);
          break; // Stone
        case 3:
          color = glm::vec3(0.1f, 0.1f, 0.1f);
          break; // Bedrock
        }
        voxels.emplace_back(pos, color, textureType, true);
      }
    }
  }
}

void VoxelChunk::draw(Camera &camera, GLFWwindow *window) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // bind all four textures once
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, textures[0]);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, textures[1]);
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, textures[2]);
    glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, textures[3]);

    shader.use();
    shader.setMat4("view",       camera.GetViewMatrix());
    shader.setMat4("projection", glm::perspective(
      glm::radians(45.0f),
      float(PathManager::SCR_WIDTH) / float(PathManager::SCR_HEIGHT),
      0.1f, 100.0f
    ));

    glBindVertexArray(VAO);
    for (auto &v : voxels) {
        if (!v.isActive) continue;

        // choose which of the four bound textures to sample
        shader.setInt("textureType", v.textureType);

        // model matrix
        glm::mat4 model = glm::translate(glm::mat4(1.0f), v.position)
                        * glm::scale   (glm::mat4(1.0f), size);
        shader.setMat4("model", model);
        shader.setFloat("Alpha", Alpha);

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    glDisable(GL_BLEND);
}
