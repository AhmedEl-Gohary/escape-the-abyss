#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "model_loader.h"
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <GL/glew.h>
#include <assimp/postprocess.h>

/**
 * @brief Constant prefix for relative path to model assets
 *
 * All model files are expected to be located under this base directory
 */
const std::string PREFIX_RELATIVE_PATH = "../assets/models/";

/**
 * @brief Default constructor for ModelLoader
 *
 * Initializes the ModelLoader object. Currently does not perform any specific initialization.
 */
ModelLoader::ModelLoader() {
    // Initialize members if needed
}

/**
 * @brief Destructor for ModelLoader
 *
 * Cleans up OpenGL resources by deleting vertex arrays, vertex buffers,
 * and element buffers for all loaded meshes.
 */
ModelLoader::~ModelLoader() {
    // Clean up OpenGL resources
    for (auto& mesh : meshes) {
        glDeleteVertexArrays(1, &mesh.VAO);
        glDeleteBuffers(1, &mesh.VBO);
        glDeleteBuffers(1, &mesh.EBO);
    }
}

/**
 * @brief Load a 3D model from a file
 *
 * @param model_name Name of the model to load (corresponds to directory and .obj file)
 *
 * This method clears any previously loaded meshes and uses Assimp to import
 * the 3D model with specified processing flags:
 * - Triangulate: Convert all faces to triangles
 * - FlipUVs: Flip texture coordinates on the y-axis
 * - GenNormals: Generate normals if not present in the model
 */
void ModelLoader::loadModel(const std::string& model_name) {
    // Clear any existing meshes
    meshes.clear();

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(PREFIX_RELATIVE_PATH + "/" + model_name + "/" + model_name + ".obj",
                                             aiProcess_Triangulate |
                                             aiProcess_FlipUVs |
                                             aiProcess_GenNormals
    );

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }

    // Process the root node recursively
    processNode(scene->mRootNode, scene, model_name);
}

/**
 * @brief Recursively process a node and its children in the 3D model
 *
 * @param node Current node being processed
 * @param scene Pointer to the entire scene
 * @param model_name Name of the model being loaded
 *
 * This method traverses the scene graph, processing meshes at the current node
 * and then recursively processing child nodes.
 */
void ModelLoader::processNode(aiNode* node, const aiScene* scene, const std::string& model_name) {
    // Process all the node's meshes
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene, model_name));
    }

    // Recursively process child nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene, model_name);
    }
}

/**
 * @brief Process a single mesh from the 3D model
 *
 * @param mesh Pointer to the mesh to process
 * @param scene Pointer to the entire scene
 * @param model_name Name of the model being loaded
 * @return Mesh Processed mesh with vertex data, indices, and textures
 *
 * This method extracts vertex positions, normals, texture coordinates,
 * indices, and textures from a mesh. It also sets up OpenGL buffers
 * and vertex attribute pointers.
 */
Mesh ModelLoader::processMesh(aiMesh* mesh, const aiScene* scene, const std::string& model_name) {
    Mesh newMesh;

    // Vertex data
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        // Positions
        newMesh.vertices.push_back(mesh->mVertices[i].x);
        newMesh.vertices.push_back(mesh->mVertices[i].y);
        newMesh.vertices.push_back(mesh->mVertices[i].z);

        // Normals
        if (mesh->HasNormals()) {
            newMesh.vertices.push_back(mesh->mNormals[i].x);
            newMesh.vertices.push_back(mesh->mNormals[i].y);
            newMesh.vertices.push_back(mesh->mNormals[i].z);
        } else {
            newMesh.vertices.push_back(0.0f);
            newMesh.vertices.push_back(0.0f);
            newMesh.vertices.push_back(0.0f);
        }

        // Texture coordinates
        if (mesh->mTextureCoords[0]) {
            newMesh.vertices.push_back(mesh->mTextureCoords[0][i].x);
            newMesh.vertices.push_back(mesh->mTextureCoords[0][i].y);
        } else {
            newMesh.vertices.push_back(0.0f);
            newMesh.vertices.push_back(0.0f);
        }
    }

    // Indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            newMesh.indices.push_back(face.mIndices[j]);
        }
    }

    // Textures
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        // Diffuse texture
        GLuint diffuseTexture = loadTexture(material, aiTextureType_DIFFUSE, model_name);
        if (diffuseTexture) {
            newMesh.textures.push_back({diffuseTexture, "texture_diffuse", "diffuse"});
        }
    }

    // Create OpenGL buffers
    glGenVertexArrays(1, &newMesh.VAO);
    glGenBuffers(1, &newMesh.VBO);
    glGenBuffers(1, &newMesh.EBO);

    glBindVertexArray(newMesh.VAO);

    // Vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, newMesh.VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 newMesh.vertices.size() * sizeof(GLfloat),
                 &newMesh.vertices[0],
                 GL_STATIC_DRAW
    );

    // Index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newMesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 newMesh.indices.size() * sizeof(GLuint),
                 &newMesh.indices[0],
                 GL_STATIC_DRAW
    );

    // Vertex attributes
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // Texture coordinate attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    // Unbind VAO
    glBindVertexArray(0);

    return newMesh;
}

/**
 * @brief Load texture from material
 *
 * @param material Pointer to the material containing the texture
 * @param type Type of texture to load (e.g., diffuse, specular)
 * @param model_name Name of the model
 * @return GLuint Texture ID or 0 if texture loading fails
 *
 * This method extracts the texture path from the material and loads it using
 * loadTextureFromFile().
 */
GLuint ModelLoader::loadTexture(aiMaterial* material, aiTextureType type, const std::string& model_name) {
    aiString texturePath;
    if (material->GetTexture(type, 0, &texturePath) == AI_SUCCESS) {
        std::string path = PREFIX_RELATIVE_PATH + "/" + model_name + "/" + texturePath.C_Str();
        return loadTextureFromFile(path);
    }
    return 0;
}

/**
 * @brief Load texture from file
 *
 * @param texturePath Path to the texture file
 * @return GLuint Texture ID
 *
 * This method loads a texture file using stb_image, creates an OpenGL texture,
 * sets texture parameters, and generates mipmaps. It supports 1, 3, and 4 channel images.
 */
GLuint ModelLoader::loadTextureFromFile(const std::string& texturePath) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load image
    int width, height, nrChannels;
    unsigned char *data = stbi_load(texturePath.c_str(), &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = nrChannels == 1 ? GL_RED :
                        nrChannels == 3 ? GL_RGB : GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cerr << "Texture failed to load at path: " << texturePath << std::endl;
    }
    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D, 0);
    return textureID;
}

/**
 * @brief Draw all loaded meshes
 *
 * Iterates through all loaded meshes, binds their textures (if any),
 * and renders them using OpenGL draw calls.
 */
void ModelLoader::draw() {
    for (auto& mesh : meshes) {
        // Bind textures if any
        if (!mesh.textures.empty()) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mesh.textures[0].id);
        }

        glBindVertexArray(mesh.VAO);
        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}