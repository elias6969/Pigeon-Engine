#include "animator.h"
#include "modelLoader.h"
#include <cstdlib>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/fwd.hpp>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Shader.h"
#include "Camera.h"
#include "filemanager.h"
#include "modelLoader.h"
#include "vao_manager.h"
#include "animator.h"

namespace data
{
    // Adjust these paths to your liking
    VirtualFileSystem resource("../resources/");
    std::string ShaderPath = resource.getFullPath("Shaders/");
    std::string ModelPath  = resource.getFullPath("models/");
};

void CharacterModel::loadModel()
{
    // Load your fancy shaders
    SpiderShader.LoadShaders(
        (data::ShaderPath + "model.vs").c_str(),
        (data::ShaderPath + "model.fs").c_str()
    );

    // Load your spider model
    SpiderModel.loadModel(
        (data::ModelPath + "Car.obj").c_str()
    );
}

void CharacterModel::RenderModel(Camera &camera, unsigned int SCR_WIDTH, unsigned int SCR_HEIGHT)
{
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT),
        0.1f,
        1000.0f
    );
    glm::mat4 view = camera.GetViewMatrix(); 

    // Use the shader
    SpiderShader.use();
    SpiderShader.setMat4("projection", projection);
    SpiderShader.setMat4("view", view);
    
    //Diffuse
    //std::srand(static_cast<unsigned int>(std::time(nullptr)));
    glm::vec3 diffuse = glm::vec3(0.5f,1.0f,10.0f);
    diffuse.x = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    diffuse.y = static_cast<float>(rand()) / static_cast<float>(RAND_MAX); 
    diffuse.z = static_cast<float>(rand()) / static_cast<float>(RAND_MAX); 
    SpiderShader.setVec3("materialDiffuse", diffuse);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
    SpiderShader.setMat4("model", model);
    SpiderModel.Draw(SpiderShader);
}

void CharacterModel::initializeModelRenderingSystem(Shader &shader, Model &model, std::string modelname, std::string vertexname, std::string fragmentname)
{
  shader.LoadShaders((data::ShaderPath + vertexname).c_str(), (data::ShaderPath + fragmentname).c_str());
  model.loadModel((data::ModelPath + modelname).c_str());
}

void CharacterModel::RenderAnimatedCharacterModel(Animator &animator, Camera &camera, float modelsize, float height, glm::vec3 Position, unsigned int SCR_WIDTH, unsigned int SCR_HEIGHT, Model &threedmodel, Shader &shader)
{
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT),
        0.1f,
        1000.0f
    );
    glm::mat4 view = camera.GetViewMatrix(); 

    float currentime = glfwGetTime();
    // Use the shader
    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);


    auto transforms = animator.GetFinalBoneMatrices();
		for (int i = 0; i < transforms.size(); ++i){
		  shader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
    }

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, Position);
    model = glm::scale(model, glm::vec3(modelsize));
    shader.setMat4("model", model);
    threedmodel.Draw(shader);
}

void CharacterModel::IMGUIinitializeModelRenderingSystem()
{
    if (!ModelPath) {
        std::cerr << "ERROR: ModelPath is null!" << std::endl;
     return;
    }
    std::cout << "Model path: " << ModelPath << std::endl;
    ModelPosition = glm::vec3(0.0f);
    modelSize = 0.0005f;
    switch (currentRenderMode) {
    case ModelRenderMode::NORMAL:
        std::cout << "MODELMANAGER::RENDER::MODE::NORMAL" << std::endl;
        IMGUIShader.LoadShaders((data::ShaderPath + "").c_str(), (data::ShaderPath + "").c_str());
    break;
    case ModelRenderMode::RAINBOW:
        std::cout << "MODELMANAGER::RENDER::MODE::RAINBOW" << std::endl;
        IMGUIShader.LoadShaders((data::ShaderPath + "mountain.vs").c_str(), (data::ShaderPath + "mountain.fs").c_str());
        //std::cout << "MODEL::PATH::" << TESTPATH << std::endl;
    break;
    case ModelRenderMode::LIGHT:
        std::cout << "MODELMANAGER::RENDER::MODE::LIGHT" << std::endl;
    break;
    }
    IMGUIModelManager.loadModel(ModelPath);
}

void CharacterModel::IMGUIRenderModel(Camera &camera, unsigned int SCR_WIDTH, unsigned int SCR_HEIGHT)
{
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT),
        0.1f,
        1000.0f
    );
    glm::mat4 view = camera.GetViewMatrix(); 

    float currentime = glfwGetTime();
    // Use the shader
    IMGUIShader.use();
    IMGUIShader.setMat4("projection", projection);
    IMGUIShader.setMat4("view", view);
    float height = 0.5f;
    if(currentRenderMode == ModelRenderMode::RAINBOW){
        IMGUIShader.setFloat("vHeight", height);
    }
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, ModelPosition);
    model = glm::scale(model, glm::vec3(modelSize));
    IMGUIShader.setMat4("model", model);
    IMGUIModelManager.Draw(IMGUIShader);
}
