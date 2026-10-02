// Image.h
#ifndef IMAGE_H
#define IMAGE_H

#include "Camera.h"
#include "Shader.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>

class Image {
public:
  static Shader shader;
  const char *imagePath;
  glm::vec3 Position;
  glm::vec3 Rotation;
  float Alpha;
  float r, g, b;
  glm::vec3 size;

  void loadImage();
  void render(Camera &camera);

private:
  GLuint textureID;
  GLuint vao, vbo, ebo;
};

#endif // IMAGE_H
