#ifndef GEOMANAGER
#define GEOMANAGER
#include <iostream>
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
