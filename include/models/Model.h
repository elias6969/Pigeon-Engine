#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.h"
#include "Shader.h"
#include "assimp_glm_helpers.h"
#include "animdata.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <filesystem>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

using namespace std;

class Model 
{
public:
    // Public model data members. These stay public for the current editor code,
    // but model loading keeps them reset and consistent between reloads.
    vector<Texture> textures_loaded; // Cache of loaded textures
    vector<Mesh> meshes;
    string directory;
    bool gammaCorrection;
    bool animated; // whether to process bone data

    // These getter methods are now public:
    auto& GetBoneInfoMap() { return m_BoneInfoMap; }
    int& GetBoneCount() { return m_BoneCounter; }

    // Load or reload a model from disk. Supported by Assimp, so OBJ, FBX, GLTF/GLB
    // and many other common 3D formats can be used when their textures are beside
    // the source file or embedded in the model.
    void loadModel(string const &path, bool animated = false, bool gamma = false)
    {
        textures_loaded.clear();
        meshes.clear();
        m_BoneInfoMap.clear();
        m_BoneCounter = 0;
        directory.clear();

        // Set the state based on the provided parameters.
        this->gammaCorrection = gamma;
        this->animated = animated;

        Assimp::Importer importer;
        const unsigned int importFlags =
            aiProcess_Triangulate |
            aiProcess_GenSmoothNormals |
            aiProcess_FlipUVs |
            aiProcess_CalcTangentSpace |
            aiProcess_JoinIdenticalVertices |
            aiProcess_LimitBoneWeights |
            aiProcess_ValidateDataStructure;

        const aiScene* scene = importer.ReadFile(path, importFlags);
        
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
            return;
        }
    
        // Get directory from the model path.
        const std::filesystem::path modelPath(path);
        const std::filesystem::path parentPath = modelPath.parent_path();
        directory = parentPath.empty() ? "." : parentPath.string();
    
        // Process the node recursively.
        processNode(scene->mRootNode, scene);
    }


    // Draw the model by drawing each mesh.
    void Draw(Shader &shader)
    {
        for(unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }

private:
    // Private bone info storage.
    map<string, BoneInfo> m_BoneInfoMap;
    int m_BoneCounter = 0;

    // Process a node recursively.
    void processNode(aiNode *node, const aiScene *scene)
    {
        for(unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        for(unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }
    }

    // Set bone data to default for a vertex.
    void SetVertexBoneDataToDefault(Vertex& vertex)
    {
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
        {
            vertex.m_BoneIDs[i] = -1;
            vertex.m_Weights[i] = 0.0f;
        }
    }

    // Process a mesh from Assimp and return a Mesh instance.
    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        // Process vertices.
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            SetVertexBoneDataToDefault(vertex);
            vertex.Position = AssimpGLMHelpers::GetGLMVec(mesh->mVertices[i]);
            vertex.Normal = mesh->HasNormals()
                ? AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i])
                : glm::vec3(0.0f);

            if (mesh->mTextureCoords[0])
            {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            }
            else
            {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }

            if (mesh->HasTangentsAndBitangents())
            {
                vertex.Tangent = AssimpGLMHelpers::GetGLMVec(mesh->mTangents[i]);
                vertex.Bitangent = AssimpGLMHelpers::GetGLMVec(mesh->mBitangents[i]);
            }
            else
            {
                vertex.Tangent = glm::vec3(0.0f);
                vertex.Bitangent = glm::vec3(0.0f);
            }

            vertices.push_back(vertex);
        }

        // Process faces (indices).
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        // Process materials. Keep the texture names aligned with Mesh::Draw().
        if (mesh->mMaterialIndex < scene->mNumMaterials)
        {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", scene);
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
            vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", scene);
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
            vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal", scene);
            textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
            vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_height", scene);
            textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
        }

        // Process bone weights only if this is an animated model.
        if (animated && mesh->mNumBones > 0)
        {
            ExtractBoneWeightForVertices(vertices, mesh, scene);
        }

        return Mesh(vertices, indices, textures);
    }

    // Add bone data (id and weight) to a vertex.
    void SetVertexBoneData(Vertex& vertex, int boneID, float weight)
    {
        for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
        {
            if (vertex.m_BoneIDs[i] < 0)
            {
                vertex.m_Weights[i] = weight;
                vertex.m_BoneIDs[i] = boneID;
                break;
            }
        }
    }

    // Extract bone data for each vertex.
    void ExtractBoneWeightForVertices(vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene)
    {
        for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
        {
            int boneID = -1;
            string boneName = mesh->mBones[boneIndex]->mName.C_Str();
            
            if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end())
            {
                BoneInfo newBoneInfo;
                newBoneInfo.id = m_BoneCounter;
                newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
                m_BoneInfoMap[boneName] = newBoneInfo;
                boneID = m_BoneCounter;
                m_BoneCounter++;
            }
            else
            {
                boneID = m_BoneInfoMap[boneName].id;
            }
            assert(boneID != -1);
            auto weights = mesh->mBones[boneIndex]->mWeights;
            int numWeights = mesh->mBones[boneIndex]->mNumWeights;
            for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
            {
                int vertexId = weights[weightIndex].mVertexId;
                float weight = weights[weightIndex].mWeight;
                assert(vertexId < vertices.size());
                SetVertexBoneData(vertices[vertexId], boneID, weight);
            }
        }
    }

    GLenum TextureFormatFromComponentCount(int nrComponents)
    {
        if (nrComponents == 1)
            return GL_RED;
        if (nrComponents == 3)
            return GL_RGB;
        return GL_RGBA;
    }

    GLenum TextureInternalFormat(GLenum format, bool gamma)
    {
        if (!gamma)
            return format;
        if (format == GL_RGB)
            return GL_SRGB;
        if (format == GL_RGBA)
            return GL_SRGB_ALPHA;
        return format;
    }

    void UploadTexture(unsigned int textureID, int width, int height, GLenum format, GLenum internalFormat, const void* data)
    {
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    unsigned int TextureFromEmbeddedTexture(const aiTexture* embeddedTexture, bool gamma)
    {
        unsigned int textureID;
        glGenTextures(1, &textureID);

        if (embeddedTexture->mHeight == 0)
        {
            int width;
            int height;
            int nrComponents;
            unsigned char* data = stbi_load_from_memory(
                reinterpret_cast<const unsigned char*>(embeddedTexture->pcData),
                embeddedTexture->mWidth,
                &width,
                &height,
                &nrComponents,
                0);

            if (data)
            {
                const GLenum format = TextureFormatFromComponentCount(nrComponents);
                UploadTexture(textureID, width, height, format, TextureInternalFormat(format, gamma), data);
                stbi_image_free(data);
            }
            else
            {
                cout << "Embedded texture failed to load: " << stbi_failure_reason() << endl;
            }

            return textureID;
        }

        vector<unsigned char> pixels(embeddedTexture->mWidth * embeddedTexture->mHeight * 4);
        for (unsigned int i = 0; i < embeddedTexture->mWidth * embeddedTexture->mHeight; ++i)
        {
            const aiTexel& texel = embeddedTexture->pcData[i];
            pixels[i * 4 + 0] = texel.r;
            pixels[i * 4 + 1] = texel.g;
            pixels[i * 4 + 2] = texel.b;
            pixels[i * 4 + 3] = texel.a;
        }

        UploadTexture(textureID, embeddedTexture->mWidth, embeddedTexture->mHeight, GL_RGBA, TextureInternalFormat(GL_RGBA, gamma), pixels.data());
        return textureID;
    }

    // Loads a texture from an external file, or from Assimp embedded texture data.
    unsigned int TextureFromFile(const char* path, const string& directory, const aiScene* scene, bool gamma = false)
    {
        if (scene)
        {
            const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(path);
            if (embeddedTexture)
                return TextureFromEmbeddedTexture(embeddedTexture, gamma);
        }

        string texturePath = string(path);
        std::replace(texturePath.begin(), texturePath.end(), '\\', '/');

        std::filesystem::path filename(texturePath);
        if (filename.is_relative())
            filename = std::filesystem::path(directory) / filename;

        unsigned int textureID;
        glGenTextures(1, &textureID);
        int width, height, nrComponents;
        unsigned char* data = stbi_load(filename.string().c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            const GLenum format = TextureFormatFromComponentCount(nrComponents);
            UploadTexture(textureID, width, height, format, TextureInternalFormat(format, gamma), data);
            stbi_image_free(data);
        }
        else
        {
            cout << "Texture failed to load at path: " << filename.string() << endl;
            stbi_image_free(data);
        }
        return textureID;
    }
    
    // Loads all textures of a given type from a material.
    vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName, const aiScene* scene)
    {
        vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if (textures_loaded[j].type == typeName && strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true;
                    break;
                }
            }
            if (!skip)
            {
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), directory, scene, gammaCorrection);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);
            }
        }
        return textures;
    }
};

#endif
