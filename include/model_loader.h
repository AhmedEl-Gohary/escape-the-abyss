#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <string>
#include <vector>
#include <GL/glew.h>
#include <assimp/scene.h>

struct Texture {
    GLuint id;
    std::string type;
    std::string path;
};

struct Mesh {
    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;
    GLuint VAO, VBO, EBO;
};

class ModelLoader {
public:
    ModelLoader();
    ~ModelLoader();

    std::vector<Mesh> meshes;

    void loadModel(const std::string& model_name);
    void draw();

private:

    GLuint loadTextureFromFile(const std::string& texturePath);

    GLuint loadTexture(aiMaterial *material, aiTextureType type, const std::string &model_name);

    Mesh processMesh(aiMesh *mesh, const aiScene *scene, const std::string &model_name);

    void processNode(aiNode *node, const aiScene *scene, const std::string &model_name);
};

#endif // MODEL_LOADER_H