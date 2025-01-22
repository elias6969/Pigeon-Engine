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

class CharacterModel
{
public:
    void customModel(Shader &shader, Model &model, std::string modelname, std::string vertexname, std::string fragmentname );

    void customRenderModel(Animator &animator, Camera &camera, float modelsize, float height, glm::vec3 Position, const unsigned int SCR_WIDTH, const unsigned int SCR_HEIGHT, Model &threedmodel, Shader &shader);
    void loadModel();

    void RenderModel(Camera &camera, const unsigned int SCR_WIDTH, const unsigned int SCR_HEIGHT);
    
private:
    Shader SpiderShader;
    Model SpiderModel;
};
#endif
