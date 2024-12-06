// GL liberaries
#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
// global liberaries
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <vector>
// header files
#include "camera.h"
#include "wood.h"
#include "house.h"
#include "sound.h"

Camera camera;
Environment *environment;

bool keys[256] = {false};

GLuint shaderProgram;
glm::mat4 projection;

GLuint loadShader(const char* shaderPath, GLenum shaderType) {
    std::ifstream shaderFile(shaderPath);
    if (!shaderFile.is_open()) {
        std::cerr << "Failed to load shader file: " << shaderPath << std::endl;
        return 0;
    }
    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    std::string shaderCode = shaderStream.str();
    const char* shaderSource = shaderCode.c_str();

    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
    }

    return shader;
}

GLuint createShaderProgram(const char* vertexPath, const char* fragmentPath) {
    GLuint vertexShader = loadShader(vertexPath, GL_VERTEX_SHADER);
    GLuint fragmentShader = loadShader(fragmentPath, GL_FRAGMENT_SHADER);

    GLuint curShaderProgram = glCreateProgram();
    glAttachShader(curShaderProgram, vertexShader);
    glAttachShader(curShaderProgram, fragmentShader);
    glLinkProgram(curShaderProgram);

    GLint success;
    glGetProgramiv(curShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(curShaderProgram, 512, nullptr, infoLog);
        std::cerr << "Program linking failed: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return curShaderProgram;
}

void setupOpenGL() {
    glEnable(GL_DEPTH_TEST);
    shaderProgram = createShaderProgram("../src/shaders/vertex_shader.glsl", "../src/shaders/fragment_shader.glsl");

    environment = new House(shaderProgram, projection);
    environment->init();
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    projection = glm::perspective(glm::radians(45.0f),
                                  (float)width / (float)height,
                                  0.1f, 100.0f); // Adjust projection
}

void keyboardDown(unsigned char key, int x, int y) {
    environment->keyboardDown(key, x, y);
}

void keyboardUp(unsigned char key, int x, int y) {
    environment->keyboardUp(key, x, y);
}

void mouseMotion(int x, int y) {
    environment->mouseMotion(x, y);
}

void renderScene() {
    environment->renderScene();
}

void onMouseClick(int button, int state, int x, int y) {
    environment->onMouseClick(button, state, x, y);
}

void update(int value) {
    environment->updateScene();
    if (!environment->isRunning) { // scene ended
        if (dynamic_cast<House*>(environment)) {
            environment = new Wood(shaderProgram, projection);
            environment->init();
        } else if (dynamic_cast<Wood*>(environment)) {
            std::cout << "env is an instance of Wood.\n";
        } else {
            std::cout << "env is of unknown type.\n";
        }
    }
    glutTimerFunc(16, update, 0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Escape The Abyss");

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error initializing GLEW: " << glewGetErrorString(err) << std::endl;
        return -1;
    }

    setupOpenGL();

    glutPassiveMotionFunc(mouseMotion);
    glutWarpPointer(WIDTH / 2.0f, HEIGHT / 2.0f);

    glutDisplayFunc(renderScene);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);
    glutPassiveMotionFunc(mouseMotion);
    glutMouseFunc(onMouseClick);
    update(1000);

    glutSetCursor(GLUT_CURSOR_NONE);

    glutMainLoop();
    return 0;
}