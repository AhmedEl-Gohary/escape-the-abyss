#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <string>
#include <vector>
#include <map>
#include <GL/glew.h>
#include <assimp/scene.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <iostream>
#include "glm/glm.hpp"

struct Texture {
    GLuint id;
    std::string type;
    std::string path;
};

struct Bone {
    std::string name;
    glm::mat4 offset_matrix;
    std::vector<std::pair<unsigned int, float>> weights;
};

struct Mesh {
    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;
    std::vector<Bone> bones;
    std::map<std::string, unsigned int> bone_mapping;
    GLuint VAO, VBO, EBO;
};

class ModelLoader {
public:
    ModelLoader();
    ~ModelLoader();

    std::vector<Mesh> meshes;
    std::map<std::string, unsigned int> global_bone_mapping;
    std::vector<glm::mat4> global_bone_final_transformations;
    std::vector<Bone> global_bones;

    glm::mat4 global_inverse_transform;

    std::vector<aiAnimation*> animations;
    std::vector<const aiScene*> scenes;
    float current_time = 0.0f;
    unsigned int current_animation = 0;

    void loadModel(const std::string& model_name);
    void draw();

    static glm::mat4 aiMatrix4x4ToGlmMat4(const aiMatrix4x4& aiMat) {
        glm::mat4 glmMat;

        glmMat[0][0] = aiMat.a1; glmMat[1][0] = aiMat.a2; glmMat[2][0] = aiMat.a3; glmMat[3][0] = aiMat.a4;
        glmMat[0][1] = aiMat.b1; glmMat[1][1] = aiMat.b2; glmMat[2][1] = aiMat.b3; glmMat[3][1] = aiMat.b4;
        glmMat[0][2] = aiMat.c1; glmMat[1][2] = aiMat.c2; glmMat[2][2] = aiMat.c3; glmMat[3][2] = aiMat.c4;
        glmMat[0][3] = aiMat.d1; glmMat[1][3] = aiMat.d2; glmMat[2][3] = aiMat.d3; glmMat[3][3] = aiMat.d4;

        return glmMat;
    }

    void updateAnimation(float delta_time) {
        if (animations.empty()) return;

        aiAnimation* current_anim = animations[current_animation];
        current_time += delta_time * current_anim->mTicksPerSecond;

        // Reset or loop animation
        if (current_time > current_anim->mDuration) {
            current_time = 0.0f;
        }

        // Calculate bone transformations at current time
        calculateBoneTransforms(current_anim,
                                scenes[current_animation],
                                current_time);
    }

private:

    GLuint loadTextureFromFile(const std::string& texturePath);

    GLuint loadTexture(aiMaterial *material, aiTextureType type, const std::string &model_name);

    Mesh processMesh(aiMesh *mesh, const aiScene *scene, const std::string &model_name);

    void processNode(aiNode *node, const aiScene *scene, const std::string &model_name);

    void loadAnimations(const aiScene* scene) {
        for (unsigned int i = 0; i < scene->mNumAnimations; i++) {
            animations.push_back(scene->mAnimations[i]);
            scenes.push_back(scene);
        }
    }

//    void calculateBoneTransforms(aiAnimation* animation, float time_in_ticks) {
//        // Implement bone transformation calculation
//        // This involves interpolating between keyframes for each bone
//        // You'll need to implement:
//        // 1. Finding the right keyframes
//        // 2. Interpolating position, rotation, and scaling
//        // 3. Calculating final transformation matrices
//    }

    void calculateBoneTransforms(aiAnimation* animation, const aiScene* scene, float time_in_ticks) {
        // Transformation matrix for the root node
        glm::mat4 root_transform = global_inverse_transform;

        const aiNode* node = scene->mRootNode;

        if (!node) {
            std::cerr << "No root node found in scene" << std::endl;
            return;
        }

        // Recursively update bone transforms
        readNodeHierarchy(time_in_ticks, animation, node, root_transform);
    }

    void readNodeHierarchy(float animation_time,
                                        aiAnimation* animation,
                                        const aiNode* node,
                                        const glm::mat4& parent_transform) {
        // Get the node name
        std::string node_name(node->mName.data);

        // Find the animation for this node
        aiNodeAnim* node_anim = findNodeAnim(animation, node_name);

        // Calculate interpolated transformation
        glm::mat4 node_transformation = aiMatrix4x4ToGlmMat4(node->mTransformation);

        if (node_anim) {
            // Interpolate scaling
            glm::vec3 scaling = interpolateScaling(animation_time, node_anim);
            glm::mat4 scaling_matrix = glm::scale(glm::mat4(1.0f), scaling);

            // Interpolate rotation
            glm::quat rotation = interpolateRotation(animation_time, node_anim);
            glm::mat4 rotation_matrix = glm::mat4_cast(rotation);

            // Interpolate translation
            glm::vec3 translation = interpolatePosition(animation_time, node_anim);
            glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), translation);

            // Combine transformations
            node_transformation = translation_matrix * rotation_matrix * scaling_matrix;
        }

        // Final transformation
        glm::mat4 global_transformation = parent_transform * node_transformation;

        // Update bone transformation if this node is a bone
        auto it = global_bone_mapping.find(node_name);
        if (it != global_bone_mapping.end()) {
            unsigned int bone_index = it->second;
            global_bone_final_transformations[bone_index] =
                    global_inverse_transform * global_transformation *
                    global_bones[bone_index].offset_matrix;
        }

        // Recursively process child nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            readNodeHierarchy(animation_time, animation, node->mChildren[i], global_transformation);
        }
    }

// Helper methods for finding and interpolating animation keyframes
    aiNodeAnim* findNodeAnim(aiAnimation* animation, const std::string& node_name) {
        for (unsigned int i = 0; i < animation->mNumChannels; i++) {
            aiNodeAnim* node_anim = animation->mChannels[i];
            if (std::string(node_anim->mNodeName.data) == node_name) {
                return node_anim;
            }
        }
        return nullptr;
    }

    glm::vec3 interpolateScaling(float animation_time, aiNodeAnim* node_anim) {
        // Find the two scaling keyframes
        for (unsigned int i = 0; i < node_anim->mNumScalingKeys - 1; i++) {
            if (animation_time < node_anim->mScalingKeys[i + 1].mTime) {
                aiVectorKey& start_key = node_anim->mScalingKeys[i];
                aiVectorKey& end_key = node_anim->mScalingKeys[i + 1];

                float delta_time = end_key.mTime - start_key.mTime;
                float factor = (animation_time - start_key.mTime) / delta_time;

                aiVector3D start_scale = start_key.mValue;
                aiVector3D end_scale = end_key.mValue;

                aiVector3D scaled = start_scale + (end_scale - start_scale) * factor;
                return glm::vec3(scaled.x, scaled.y, scaled.z);
            }
        }

        // Return last scaling if no interpolation needed
        return glm::vec3(
                node_anim->mScalingKeys[node_anim->mNumScalingKeys - 1].mValue.x,
                node_anim->mScalingKeys[node_anim->mNumScalingKeys - 1].mValue.y,
                node_anim->mScalingKeys[node_anim->mNumScalingKeys - 1].mValue.z
        );
    }

    glm::quat interpolateRotation(float animation_time, aiNodeAnim* node_anim) {
        // Similar to scaling interpolation, but with quaternion interpolation
        for (unsigned int i = 0; i < node_anim->mNumRotationKeys - 1; i++) {
            if (animation_time < node_anim->mRotationKeys[i + 1].mTime) {
                aiQuaternion& start_rot = node_anim->mRotationKeys[i].mValue;
                aiQuaternion& end_rot = node_anim->mRotationKeys[i + 1].mValue;

                float delta_time = node_anim->mRotationKeys[i + 1].mTime -
                                   node_anim->mRotationKeys[i].mTime;
                float factor = (animation_time - node_anim->mRotationKeys[i].mTime) / delta_time;

                aiQuaternion interpolated;
                aiQuaternion::Interpolate(interpolated, start_rot, end_rot, factor);
                interpolated.Normalize();

                return glm::quat(
                        interpolated.w,
                        interpolated.x,
                        interpolated.y,
                        interpolated.z
                );
            }
        }

        // Return last rotation if no interpolation needed
        aiQuaternion& last_rot = node_anim->mRotationKeys[node_anim->mNumRotationKeys - 1].mValue;
        return glm::quat(last_rot.w, last_rot.x, last_rot.y, last_rot.z);
    }

    glm::vec3 interpolatePosition(float animation_time, aiNodeAnim* node_anim) {
        // Similar implementation to scaling interpolation
        for (unsigned int i = 0; i < node_anim->mNumPositionKeys - 1; i++) {
            if (animation_time < node_anim->mPositionKeys[i + 1].mTime) {
                aiVectorKey& start_key = node_anim->mPositionKeys[i];
                aiVectorKey& end_key = node_anim->mPositionKeys[i + 1];

                float delta_time = end_key.mTime - start_key.mTime;
                float factor = (animation_time - start_key.mTime) / delta_time;

                aiVector3D start_pos = start_key.mValue;
                aiVector3D end_pos = end_key.mValue;

                aiVector3D interpolated = start_pos + (end_pos - start_pos) * factor;
                return glm::vec3(interpolated.x, interpolated.y, interpolated.z);
            }
        }

        // Return last position if no interpolation needed
        return glm::vec3(
                node_anim->mPositionKeys[node_anim->mNumPositionKeys - 1].mValue.x,
                node_anim->mPositionKeys[node_anim->mNumPositionKeys - 1].mValue.y,
                node_anim->mPositionKeys[node_anim->mNumPositionKeys - 1].mValue.z
        );
    }
};

#endif // MODEL_LOADER_H