// Image.h
#ifndef IMAGE_H
#define IMAGE_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "Shader.h"
#include "Camera.h"
#include <GLFW/glfw3.h>

class Image {
public:
    Shader shader;
    const char* imagePath;
    glm::vec3 Position;
    glm::vec3 Rotation;
    float Alpha;
    float r, g, b;
    glm::vec3 size;

    Image();  // Constructor to load the image file.
    void loadImage();   // Load image data into a texture.
    void render(Camera &camera); // Render the image.

private:
    GLuint textureID;
    GLuint vao, vbo, ebo;
};

#endif // IMAGE_H
