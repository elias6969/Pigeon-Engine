#include "Camera.h"
#include "Shader.h"
#include "Variables.h"
#include "VoxelTest.h"
#include "textureManager.h"


#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>
#include <GLFW/glfw3.h>
#include <cassert>
#include <filesystem>
#include <glad/glad.h>
#include <glm/ext.hpp>
#include <iostream>
#include <stb_image.h>
#include <vector>

// Face vertex positions for each of the 6 cube faces
static const glm::vec3 FACE_POS[6][6] = {
    {{1, 0, 0}, {1, 1, 0}, {1, 1, 1}, {1, 0, 0}, {1, 1, 1}, {1, 0, 1}}, // +X
    {{0, 0, 1}, {0, 1, 1}, {0, 1, 0}, {0, 0, 1}, {0, 1, 0}, {0, 0, 0}}, // -X
    {{0, 1, 1}, {1, 1, 1}, {1, 1, 0}, {0, 1, 1}, {1, 1, 0}, {0, 1, 0}}, // +Y
    {{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 0}, {1, 0, 1}, {0, 0, 1}}, // -Y
    {{0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 0, 1}, {1, 1, 1}, {0, 1, 1}}, // +Z
    {{1, 0, 0}, {0, 0, 0}, {0, 1, 0}, {1, 0, 0}, {0, 1, 0}, {1, 1, 0}}  // -Z
};

// Texture coordinates for each face
static const glm::vec2 FACE_UV[6] = {{0, 0}, {0, 1}, {1, 1},
                                     {0, 0}, {1, 1}, {1, 0}};

// Texture list to be loaded into the array
static const std::vector<std::string> textures = {"grass.jpg", "dirt.jpg",
                                                  "bird.png"};

// Load a 2D texture array from multiple image files

GLuint loadTextureArray(const std::vector<std::string> &textureFiles, const std::string &folder) {
    constexpr int TARGET_WIDTH = 16;
    constexpr int TARGET_HEIGHT = 16;
    constexpr int CHANNELS = 4;

    stbi_set_flip_vertically_on_load(true);

    std::cout << "[VoxelTest] 📁 Texture folder: " << folder << std::endl;

    GLuint texID = 0;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texID);

    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, TARGET_WIDTH, TARGET_HEIGHT, textureFiles.size());

    for (size_t i = 0; i < textureFiles.size(); ++i) {
        const std::string fullPath = folder + "/" + textureFiles[i];
        std::cout << " ➤ Trying to load: " << fullPath << std::endl;

        int width = 0, height = 0, channels = 0;
        unsigned char* data = stbi_load(fullPath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

        std::vector<unsigned char> resized(TARGET_WIDTH * TARGET_HEIGHT * CHANNELS);

        if (!data) {
            std::cerr << " ❌ stbi_load failed for: " << fullPath << " — using fallback white texture.\n";
            std::fill(resized.begin(), resized.end(), 255); // Fallback white
        } else {
            bool ok = stbir_resize_uint8(
                data, width, height, 0,
                resized.data(), TARGET_WIDTH, TARGET_HEIGHT, 0, CHANNELS
            );

            if (!ok) {
                std::cerr << " ❌ Resize failed for: " << fullPath << " — using fallback.\n";
                std::fill(resized.begin(), resized.end(), 255);
            } else {
                std::cout << " ✅ Loaded + resized: " << fullPath << " ➜ " << TARGET_WIDTH << "x" << TARGET_HEIGHT << "\n";
            }

            stbi_image_free(data);
        }

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
            0, 0, 0, i,
            TARGET_WIDTH, TARGET_HEIGHT, 1,
            GL_RGBA, GL_UNSIGNED_BYTE,
            resized.data());
    }

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    std::cout << "[VoxelTest] ✅ Texture array uploaded.\n";
    return texID;
}


int VoxelTest::index(int x, int y, int z) const {
  return x + CHUNK_X * (y + CHUNK_Y * z);
}

void VoxelTest::setVoxel(int x, int y, int z, const VoxelStruct &voxel) {
  voxels[index(x, y, z)] = voxel;
}

VoxelTest::VoxelTest(int x, int y, int z)
    : CHUNK_X(x), CHUNK_Y(y), CHUNK_Z(z), voxels(x * y * z, VoxelStruct{}),
      VAO(0), VBO(0), textureArray(0), vertexCount(0) {

  std::cout << "[VoxelTest] Initializing chunk: " << CHUNK_X << "x" << CHUNK_Y
            << "x" << CHUNK_Z << std::endl;

  std::cout << "[VoxelTest] textures size: " << textures.size() << std::endl;
  std::cout << "[VoxelTest] texturePath: " << PathManager::texturePath
            << std::endl;

  for (const auto &tex : textures) {
    std::cout << " ➤ Will try to load: " << PathManager::texturePath + "/" + tex
              << std::endl;
  }
  textureArray = loadTextureArray(textures, PathManager::texturePath);
  if (textureArray == 0) {
    std::cerr << "[VoxelTest] Failed to load texture array.\n";
    return;
  }

  // Fill voxel data
  for (int y = 0; y < CHUNK_Y; ++y)
    for (int z = 0; z < CHUNK_Z; ++z)
      for (int x = 0; x < CHUNK_X; ++x)
        if (y < 4)
          setVoxel(x, y, z, VoxelStruct{1, 2, true});

  // Build mesh from voxels
  for (int z = 0; z < CHUNK_Z; ++z)
    for (int y = 0; y < CHUNK_Y; ++y)
      for (int x = 0; x < CHUNK_X; ++x) {
        const auto &voxel = voxels[index(x, y, z)];
        if (!voxel.solid)
          continue;

        for (int face = 0; face < 6; ++face) {
          int nx = x + (face == 0) - (face == 1);
          int ny = y + (face == 2) - (face == 3);
          int nz = z + (face == 4) - (face == 5);

          bool solidNeighbor = nx >= 0 && ny >= 0 && nz >= 0 && nx < CHUNK_X &&
                               ny < CHUNK_Y && nz < CHUNK_Z &&
                               voxels[index(nx, ny, nz)].solid;

          if (solidNeighbor)
            continue;

          for (int i = 0; i < 6; ++i) {
            meshVertices.push_back({glm::vec3(x, y, z) + FACE_POS[face][i],
                                    FACE_UV[i],
                                    static_cast<float>(voxel.textureIndex)});
          }
        }
      }

  if (meshVertices.empty()) {
    std::cerr << "[VoxelTest] ⚠️ No visible voxel faces. Mesh is empty.\n";
    return;
  }

  // Upload to GPU
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, meshVertices.size() * sizeof(VoxelVertex),
               meshVertices.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex),
                        (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex),
                        (void *)offsetof(VoxelVertex, uv));
  glEnableVertexAttribArray(1);

  glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex),
                        (void *)offsetof(VoxelVertex, textureIndex));
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);

  vertexCount = static_cast<int>(meshVertices.size());

  // Load shader
  shader.LoadShaders((PathManager::shaderPath + "testvox.vs").c_str(),
                     (PathManager::shaderPath + "testvox.fs").c_str());

  std::cout << "[VoxelTest] ✅ Initialized with " << vertexCount
            << " vertices.\n";
}

void VoxelTest::draw(Camera &camera) {
  if (vertexCount == 0 || VAO == 0)
    return;

  shader.use();
  shader.setMat4("projection", static_cast<float>(PathManager::SCR_WIDTH) /
                                   PathManager::SCR_HEIGHT);
  shader.setMat4("view", camera.GetViewMatrix());
  shader.setMat4("model", glm::mat4(1.0f));
  shader.setInt("textureArray", 0);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray);

  glBindVertexArray(VAO);
  glDrawArrays(GL_TRIANGLES, 0, vertexCount);
  glBindVertexArray(0);
}
