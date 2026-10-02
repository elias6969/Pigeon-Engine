#ifndef SPHERE_H
#define SPHERE_H

#include "Shader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

class Sphere {
public:
  Sphere(float radius = 1.0f, unsigned int sectorCount = 36,
         unsigned int stackCount = 18);
  ~Sphere();

  void Draw(const glm::mat4 &view, const glm::mat4 &projection,
            const glm::vec3 &position, const glm::vec3 &color);

private:
  void generateSphere();
  void setupMesh();

  float radius;
  GLuint texture;
  unsigned int sectorCount;
  unsigned int stackCount;

  std::vector<float> vertices;
  std::vector<unsigned int> indices;

  GLuint VAO, VBO, EBO;

  Shader shader;
};

#endif
