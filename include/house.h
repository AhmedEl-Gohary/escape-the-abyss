#ifndef HOUSE_H
#define HOUSE_H

#include <GL/glew.h>
#include <GL/glut.h>
#include "environment.h"
#include "model_loader.h"

class House : public Environment {
private:
    glm::mat4 playerTransformation;
    glm::vec3 playerPosition;
    ModelLoader playerModel;

    void generatePlayer();
    void renderPlayer();

public:

    House(GLuint shaderProgram, glm::mat4 &projection) : Environment(shaderProgram, projection) {}
    void init() override;
    void renderScene() override;
    void updateScene() override;
    void processKeyboard() override;
};
#endif
