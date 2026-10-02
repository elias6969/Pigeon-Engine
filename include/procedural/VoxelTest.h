#pragma once

#include <vector>
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "Camera.h"
#include "Shader.h"

struct VoxelStruct {
    uint8_t type = 0;
    uint8_t textureIndex = 0;
    bool solid = false;
};

struct VoxelVertex {
    glm::vec3 position;
    glm::vec2 uv;
    float textureIndex;
};

class VoxelTest {
public:
    VoxelTest(int x, int y, int z);
    void draw(Camera& camera);

private:
    // Constants for chunk dimensions
    const int CHUNK_X;
    const int CHUNK_Y;
    const int CHUNK_Z;

    // GPU stuff
    GLuint VAO;
    GLuint VBO;
    GLuint textureArray;
    Shader shader;

    // Voxel and mesh data
    std::vector<VoxelStruct> voxels;
    std::vector<VoxelVertex> meshVertices;
    int vertexCount;

    // Internal helpers
    void setVoxel(int x, int y, int z, const VoxelStruct& voxel);
    int index(int x, int y, int z) const;
};
