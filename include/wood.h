#ifndef WOOD_H
#define WOOD_H

#include "environment.h"

class Wood : public Environment {
private:

public:
    Wood(GLuint shaderProgram, glm::mat4 &projection) : Environment(shaderProgram, projection) {}
    void init() override;
    void renderScene() override;
    void updateScene() override;
    void processKeyboard() override;
};

#endif
