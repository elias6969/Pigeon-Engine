#ifndef GRID_H
#define GRID_H

#include <vector>
#include <glad/glad.h>
#include "Shader.h"
#include "Camera.h"
#include <GLFW/glfw3.h>

class Grid {
public:
    // Parameters for water and grid rendering.
    float speed, frequency, amplitude, size, spacing;

    // Water grid functions.
    std::vector<float> generateGridWater(float size, float spacing);
    void setupGridWater();
    void renderGridWater(Camera& camera, GLFWwindow* window);

    // Regular grid (line) functions.
    std::vector<float> generateGrid(float size, float spacing);
    void setupGrid();
    void renderGrid(Camera& camera, GLFWwindow* window);

private:
    // Shaders for each grid.
    static Shader waterShader, gridShader;

    // Water grid buffers and data.
    GLuint waterVAO = 0, waterVBO = 0, waterEBO = 0;
    std::vector<float> waterVertices;
    std::vector<unsigned int> waterIndices;

    // Regular grid buffers and data.
    GLuint gridVAO = 0, gridVBO = 0;
    std::vector<float> gridVertices;
};

#endif // GRID_H
