#ifndef VARIABLES_H
#define VARIABLES_H
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>

class PathManager {
public:
    // Declare variables as extern.
    
    static std::string texturePath;
    static std::string shaderPath;
    static std::string cubemappath;
    static std::string cubemappathtest;
    static std::string fontpath;
    static std::string modelPath;
    static unsigned int SCR_HEIGHT;
    static unsigned int SCR_WIDTH;
};

// Declare the function prototype.
void ScreenSizeConfiguration(GLFWwindow* window, int &width, int &height);

#endif // VARIABLES_H
