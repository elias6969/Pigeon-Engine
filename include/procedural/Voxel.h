#pragma once

#include "Camera.h"
#include "Shader.h"
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <glad/glad.h>

// A minimal voxel: position + which texture layer to sample
struct Voxel {
    glm::vec3 position;
    uint8_t   textureType;
    Voxel(const glm::vec3 &pos, uint8_t texType)
      : position(pos), textureType(texType) {}
};

class VoxelChunk {
public:
    // Pass in your chunk dimensions and the folder where
    // you keep: grass.jpg, dirt.jpg, stone.jpg, bedrock.jpg
    VoxelChunk(int sizeX, int sizeY, int sizeZ,
               const std::string &textureFolder);
    ~VoxelChunk();

    // Call each frame to draw
    void draw(const Camera &camera);

private:
    void generateVoxels();
    void buildMesh();        // produce only exposed faces
    void setupGL();          // VAO/VBO
    void loadTextureArray(); // pack 4 images into a GL_TEXTURE_2D_ARRAY

    int sizeX, sizeY, sizeZ;
    std::string textureFolder;

    std::vector<Voxel>      voxels;
    std::vector<uint8_t>    typeGrid;     // occupancy + textureType per cell
    std::vector<float>      meshVertices; // x,y,z, u,v, layerPacked

    GLuint VAO = 0, VBO = 0;
    GLuint textureArray = 0;
    Shader shader;

    glm::vec3 scale = glm::vec3(1.0f);
    float     alpha = 1.0f;
};

