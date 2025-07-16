#include "textureManager.h"
#include <stb_image.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

extern unsigned int loadTexture(const char *filePath) {
  int width, height, nrChannels;
  unsigned char *data = stbi_load(filePath, &width, &height, &nrChannels, 0);
  if (!data) {
    std::cerr << "Failed to load texture: " << filePath << std::endl;
    return 0;
  } else {
    std::cout << "successfully loaded texture" << filePath << std::endl;
  }

  GLuint texID;
  glGenTextures(1, &texID);
  glBindTexture(GL_TEXTURE_2D, texID);

  // Set up texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Determine correct format (RGB or RGBA)
  GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

  // Upload the image data to the GPU
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
               GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  // Cleanup
  stbi_image_free(data);
  glBindTexture(GL_TEXTURE_2D, 0);

  return texID;
}

