#ifndef MODELLOADER_H
#define MODELLOADER_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/fwd.hpp>
#include <iostream>
#include <glm/glm.hpp>
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "animator.h"

enum class ModelRenderMode
{
    NORMAL = 0,
    RAINBOW = 1,
    LIGHT = 2,
    COUNT
};

class CharacterModel
{
public:
    ModelRenderMode currentRenderMode = ModelRenderMode::NORMAL;
    glm::vec3 ModelPosition;
    glm::vec3 Rotation;
    const char* ModelPath;
    float vHeight;
    float maxHeight;
    float minHeight;
    float modelSize;
    //FUNCTIONS
    void initializeModelRenderingSystem(Shader &shader, Model &model, std::string modelname, std::string vertexname, std::string fragmentname );

    void RenderAnimatedCharacterModel(Animator &animator, Camera &camera, float modelsize, float height, glm::vec3 Position, const unsigned int SCR_WIDTH, const unsigned int SCR_HEIGHT, Model &threedmodel, Shader &shader);
    void loadModel();

    void RenderModel(Camera &camera, const unsigned int SCR_WIDTH, const unsigned int SCR_HEIGHT);
    


    void IMGUIinitializeModelRenderingSystem();
    void IMGUIRenderModel(Camera &camera, unsigned int SCR_WIDTH, unsigned int SCR_HEIGHT);
private:

    Model IMGUIModelManager;
    Shader IMGUIShader;
    Shader SpiderShader;
    Model SpiderModel;
};
#endif
