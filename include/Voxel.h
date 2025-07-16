#pragma once

#include "Camera.h"
#include "Shader.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <vector>

struct Voxel {
  glm::vec3 position;
  glm::vec3 color;
  uint8_t textureType;
  bool isActive;

  Voxel(const glm::vec3 &pos, const glm::vec3 &col, uint8_t texType, bool active = true)
      : position(pos), color(col), textureType(texType), isActive(active) {}
};

class VoxelChunk {
public:
  VoxelChunk(int sizeX, int sizeY, int sizeZ);

  const std::vector<Voxel> &getVoxels() const;
  void loadCubes();
  void draw(Camera &camera, GLFWwindow *window);

private:
  const char *texturePath;
  int sizeX, sizeY, sizeZ;
  std::vector<Voxel> voxels;

  Shader shader;
  GLuint VAO, VBO, texture;
  glm::vec3 size = glm::vec3(1.0f);
  glm::vec3 rotation = glm::vec3(0.0f);
  float Alpha = 1.0f;

  void generateChunk();
  GLuint textures[4];
};
