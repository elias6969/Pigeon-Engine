#ifndef GEOMANAGER
#define GEOMANAGER
#include <iostream>
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Shader.h"

class geoData
{
private:
    Shader shader;
    unsigned int VBO, VAO;
public:
    void initGeometry();
    void RenderGeo();
};
#endif 
