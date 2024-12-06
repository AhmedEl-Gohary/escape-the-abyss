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
        int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
        int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);

        // Calculate center of the window
        int centerX = windowWidth / 2;
        int centerY = windowHeight / 2;

        // Calculate delta movement
        int deltaX = x - centerX;
        int deltaY = y - centerY;

        // Only process movement if there's a significant change
        if (deltaX != 0 || deltaY != 0) {
            // Adjust camera
            camera.adjustCameraCenter(deltaX, deltaY);

            // Reset cursor to window center
            glutWarpPointer(centerX, centerY);
        }
    }
};

#endif

