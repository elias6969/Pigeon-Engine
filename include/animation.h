#pragma once

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include <functional>
#include <cassert>
#include "bone.h"
#include "animdata.h"
#include "Model.h"
#include "assimp_glm_helpers.h"

struct AssimpNodeData
{
    glm::mat4 transformation;
    std::string name;
    int childrenCount;
    std::vector<AssimpNodeData> children;
};

class Animation
{
public:
    Animation() = default;

    void LoadAnimation(const std::string& animationPath, Model* model)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
        assert(scene && scene->mRootNode);
        auto animation = scene->mAnimations[0];
        m_Duration = animation->mDuration;
        m_TicksPerSecond = animation->mTicksPerSecond;
        
        // Calculate global inverse transformation if needed.
        aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
        globalTransformation = globalTransformation.Inverse();

        // Build the node hierarchy.
        ReadHierarchyData(m_RootNode, scene->mRootNode);
        ReadMissingBones(animation, *model);

        // Store a pointer to the Model's bone info map.
        m_BoneInfoMapPtr = &model->GetBoneInfoMap();
    }

    ~Animation() { }

    Bone* FindBone(const std::string& name)
    {
        auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
            [&](const Bone& bone) {
                return bone.GetBoneName() == name;
            }
        );
        return (iter == m_Bones.end()) ? nullptr : &(*iter);
    }

    inline float GetTicksPerSecond() const { return m_TicksPerSecond; }
    inline float GetDuration() const { return m_Duration; }
    inline const AssimpNodeData& GetRootNode() const { return m_RootNode; }
    
    // Return the Model's bone info map.
    inline const std::map<std::string, BoneInfo>& GetBoneIDMap() const
    {
        assert(m_BoneInfoMapPtr && "Bone info map pointer is not set.");
        return *m_BoneInfoMapPtr;
    }

private:
    // Reads bone channels and updates the Model's bone info map.
    void ReadMissingBones(const aiAnimation* animation, Model& model)
    {
        int size = animation->mNumChannels;
        auto& boneInfoMap = model.GetBoneInfoMap();
        int& boneCount = model.GetBoneCount();

        for (int i = 0; i < size; i++)
        {
            auto channel = animation->mChannels[i];
            std::string boneName = channel->mNodeName.data;

            if (boneInfoMap.find(boneName) == boneInfoMap.end())
            {
                boneInfoMap[boneName].id = boneCount;
                boneCount++;
            }
            m_Bones.push_back(Bone(boneName, boneInfoMap[boneName].id, channel));
        }
    }

    void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src)
    {
        assert(src);
        dest.name = src->mName.data;
        dest.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
        dest.childrenCount = src->mNumChildren;
        for (int i = 0; i < src->mNumChildren; i++)
        {
            AssimpNodeData newData;
            ReadHierarchyData(newData, src->mChildren[i]);
            dest.children.push_back(newData);
        }
    }

    float m_Duration = 0.0f;
    int m_TicksPerSecond = 0;
    std::vector<Bone> m_Bones;
    AssimpNodeData m_RootNode;
    
    // Pointer to the Model's bone info map.
    const std::map<std::string, BoneInfo>* m_BoneInfoMapPtr = nullptr;
};
