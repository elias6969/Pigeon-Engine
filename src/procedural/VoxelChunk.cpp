#include "Variables.h"
#include "Voxel.h"
#include "Camera.h"
#include "Shader.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>
#include <glm/gtc/noise.hpp>
#include <stb_image.h>
#include <iostream>
#include <cassert>

// For each of the 6 cube faces: 6 vertices, relative positions [0..1]
static const glm::vec3 FACE_POS[6][6] = {
  // +X
  {{1,0,0},{1,1,0},{1,1,1},{1,0,0},{1,1,1},{1,0,1}},
  // -X
  {{0,0,1},{0,1,1},{0,1,0},{0,0,1},{0,1,0},{0,0,0}},
  // +Y
  {{0,1,1},{1,1,1},{1,1,0},{0,1,1},{1,1,0},{0,1,0}},
  // -Y
  {{0,0,0},{1,0,0},{1,0,1},{0,0,0},{1,0,1},{0,0,1}},
  // +Z
  {{0,0,1},{1,0,1},{1,1,1},{0,0,1},{1,1,1},{0,1,1}},
  // -Z
  {{1,0,0},{0,0,0},{0,1,0},{1,0,0},{0,1,0},{1,1,0}}
};

// UVs for each of those 6 verts
static const glm::vec2 FACE_UV[6] = {
  {0,0},{0,1},{1,1},{0,0},{1,1},{1,0}
};

inline int idx3(int x,int y,int z,int sx,int sy){
  return x + y*sx + z*sx*sy;
}

VoxelChunk::VoxelChunk(int sx,int sy,int sz,const std::string &texFolder)
 : sizeX(sx), sizeY(sy), sizeZ(sz),
   textureFolder(texFolder),
   typeGrid(sx*sy*sz, 0xFF)  // 0xFF means “no voxel here”
{
  generateVoxels();
  buildMesh();
  setupGL();
  loadTextureArray();

  // load your instanced shader
  shader.LoadShaders((PathManager::shaderPath + "/voxel_inst.vs").c_str(),
                     (PathManager::shaderPath + "/voxel_inst.fs").c_str());
  shader.use();
  shader.setInt ("textureArray", 0);
  shader.setFloat("Alpha", alpha);
}

VoxelChunk::~VoxelChunk(){
  glDeleteBuffers(1, &VBO);
  glDeleteVertexArrays(1, &VAO);
  glDeleteTextures(1, &textureArray);
}

void VoxelChunk::generateVoxels(){
  voxels.clear();
  voxels.reserve(sizeX*sizeY*sizeZ);
  float noiseScale = 0.1f;
  float heightMul  = sizeY / 2.0f;

  for(int x=0; x<sizeX; ++x){
    for(int z=0; z<sizeZ; ++z){
      float n = glm::perlin(glm::vec2(x*noiseScale, z*noiseScale));
      int maxY = int((n+1.0f)/2.0f * heightMul);
      for(int y=0; y<maxY; ++y){
        // pick your layer: 0=grass,1=dirt,2=stone,3=bedrock
        uint8_t layer = (y==0 ? 3
                          : y>maxY-2 ? 0
                          : y>maxY-5 ? 1
                                     : 2);
        voxels.emplace_back(glm::vec3(x,y,z), layer);
        typeGrid[idx3(x,y,z,sizeX,sizeY)] = layer;
      }
    }
  }
}

void VoxelChunk::buildMesh(){
  meshVertices.clear();
  meshVertices.reserve(voxels.size()*6*6);

  auto hasVoxel = [&](int x,int y,int z){
    if(x<0||x>=sizeX||y<0||y>=sizeY||z<0||z>=sizeZ) return false;
    return typeGrid[idx3(x,y,z,sizeX,sizeY)] != 0xFF;
  };

  for(auto &v : voxels){
    int x=int(v.position.x),
        y=int(v.position.y),
        z=int(v.position.z);
    float layer = float(v.textureType);

    // check each of the 6 directions
    for(int d=0; d<6; ++d){
      int nx = x + (d==0?1 : d==1?-1:0);
      int ny = y + (d==2?1 : d==3?-1:0);
      int nz = z + (d==4?1 : d==5?-1:0);
      if(!hasVoxel(nx,ny,nz)){
        // emit that face
        for(int i=0; i<6; ++i){
          auto p = FACE_POS[d][i];
          meshVertices.push_back((x + p.x)*scale.x);
          meshVertices.push_back((y + p.y)*scale.y);
          meshVertices.push_back((z + p.z)*scale.z);
          meshVertices.push_back(FACE_UV[i].x);
          meshVertices.push_back(FACE_UV[i].y);
          meshVertices.push_back(layer);
        }
      }
    }
  }
}

void VoxelChunk::setupGL(){
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER,
               meshVertices.size()*sizeof(float),
               meshVertices.data(),
               GL_STATIC_DRAW);
  GLsizei stride = 6 * sizeof(float);

  // pos
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,stride,(void*)0);
  // uv
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,stride,(void*)(3*sizeof(float)));
  // layer index
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2,1,GL_FLOAT,GL_FALSE,stride,(void*)(5*sizeof(float)));

  glBindVertexArray(0);
}

void VoxelChunk::loadTextureArray(){
  const std::vector<std::string> files = {
    "grass.jpg","dirt.jpg","stone.jpg","bedrock.jpg"
  };
  int W=0,H=0,C=0;

  glGenTextures(1, &textureArray);
  glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray);

  // first image: allocate storage
  {
    auto path = textureFolder + "/" + files[0];
    unsigned char *data = stbi_load(path.c_str(), &W,&H,&C,4);
    assert(data && "Texture load failed");
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, W, H,
                 (GLsizei)files.size(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
                    0,0,0, W,H,1,
                    GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
  }

  // remaining layers
  for(int i=1;i<files.size();++i){
    auto path = textureFolder + "/" + files[i];
    int w,h,c;
    unsigned char *data = stbi_load(path.c_str(), &w,&h,&c,4);
    assert(data);
    if(w!=W||h!=H) std::cerr<<"Warning: texture size mismatch\n";
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
                    0,0,i, W,H,1,
                    GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
  }

  glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void VoxelChunk::draw(const Camera &camera) {
    shader.use();

    // 1) Compute aspect from your PathManager values
    float aspect = float(PathManager::SCR_WIDTH)
                 / float(PathManager::SCR_HEIGHT);

    // 2) No cast needed — make sure GetViewMatrix() is declared const
    shader.setMat4("view",       camera.GetViewMatrix());
    shader.setMat4("projection", glm::perspective(
        glm::radians(45.0f),
        aspect,
        0.1f, 100.0f
    ));
    shader.setMat4("model", glm::mat4(1.0f));

    // bind texture array & VAO
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray);

    glBindVertexArray(VAO);
    glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      // draw all your culled faces in one go
      GLsizei vertexCount = GLsizei(meshVertices.size() / 6);
      glDrawArrays(GL_TRIANGLES, 0, vertexCount);

    glDisable(GL_BLEND);
    glBindVertexArray(0);
}

