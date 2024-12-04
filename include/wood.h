#ifndef WOOD_H
#define WOOD_H

#include <GL/glew.h>
#include <GL/glut.h>
#include "environment.h"
#include "model_loader.h"
#include "ctime"

class Wood : public Environment {
private:
    std::vector<glm::mat4> treeTransformations; // Store tree transformations
    ModelLoader treeModel;  // Model loader for tree models

    void generateForest(int treeCount);

    void renderForest();

    bool checkCollision(const glm::vec3& cameraPosition, const glm::vec3& treePos);

public:
    const float WOODS_SIZE = 100.0f;  // Size of the woods area
    const int TREE_COUNT = 30;        // Number of trees to generate
    const float TREE_RADIUS = 1.0f;    // Collision radius for trees

    Wood(GLuint shaderProgram, glm::mat4 &projection) : Environment(shaderProgram, projection) {}
    void init() override;
    void renderScene() override;
    void updateScene() override;
    void processKeyboard() override;
};

#endif
