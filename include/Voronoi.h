#ifndef VORONOI_H
#define VORONOI_H
#include "Camera.h"
#include "Shader.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <string>
#include <vector>

class Voronoi {
public:
  Voronoi(int NUM_POINTS, const std::vector<glm::vec2> &seeds);
  void setColors(const std::vector<glm::vec3>& colors);
  void drawVo(Camera &camera);

private:
  unsigned int quadVAO, quadVBO;
  Shader voronoiShader;
  std::vector<glm::vec2> seedPoints;
  std::vector<glm::vec3> seedColors;
  int numPoints;
};
#endif // !VORONOI_H
