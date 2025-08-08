#ifndef PYRAMID_H
#define PYRAMID_H

#include "Camera.h"
#include "Shader.h"
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <vector>

struct pyramid_Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 texCoords;
};

class Pyramid {
public:
  Pyramid(Shader shader, float size, float height);
  void init(float size, float height);
  std::vector<pyramid_Vertex> generatePyramid(float size, float height);
  void draw(Camera &camera);
  glm::vec3 position = glm::vec3(0.0f);
  glm::vec3 color = glm::vec3(1.0f);

private:
  Shader shader;
  GLuint vao = 0, vbo = 0, texture;
  std::vector<pyramid_Vertex> verts;
};
#endif // !PYRAMID_H
