#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "camera.h"
#include <GL/glut.h>

class Environment {
protected:
    Camera camera;
    bool keys[256] = {false};
public:
    bool isRunning{true};
    GLuint shaderProgram;
    glm::mat4 &projection;

    virtual ~Environment() = default;
    Environment(GLuint shaderProgram, glm::mat4 &projection) : shaderProgram(shaderProgram), projection(projection) {}
    virtual void init() = 0;
    virtual void renderScene() = 0;
    virtual void updateScene() = 0;

    virtual void onMouseClick(int button, int state, int x, int y) = 0;
    void keyboardDown(unsigned char key, int x, int y) { keys[key] = true; }
    void keyboardUp(unsigned char key, int x, int y) { keys[key] = false; }
    virtual void processKeyboard() = 0;

    void mouseMotion(int x, int y){
        camera.adjustCameraCenter(x, y);
    }
};

#endif

