#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "model_loader.h"
#include "camera.h"
#include <vector>
#include <cstdlib>
#include <ctime>

const float WOODS_SIZE = 100.0f;  // Size of the woods area
const int TREE_COUNT = 30;        // Number of trees to generate
const float TREE_RADIUS = 1.0f;    // Collision radius for trees

Camera camera;

// Keyboard state tracking
bool keys[256] = {false};

// Shader and model loader (global variables)
GLuint shaderProgram;
glm::mat4 projection, view;

std::vector<glm::mat4> treeTransformations; // Store tree transformations
ModelLoader treeModel;  // Model loader for tree models

// Load shader from file
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

// Create a shader program from vertex and fragment shaders
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

// Generate a forest of trees with random positions
void generateForest(int treeCount) {
    treeModel.loadModel("tree");
    srand(static_cast<unsigned int>(time(nullptr))); // Seed for randomness

    std::vector<glm::vec3> treePositions; // Store positions of trees
    float minDistance = TREE_RADIUS * 15.0f; // Increase this value for more spacing

    while (treePositions.size() < treeCount) {
        glm::mat4 treeTransform = glm::mat4(1.0f);
        float x = static_cast<float>(rand()) / (float) RAND_MAX * WOODS_SIZE - (WOODS_SIZE / 2);
        float z = static_cast<float>(rand()) / (float) RAND_MAX * WOODS_SIZE - (WOODS_SIZE / 2);
        glm::vec3 newTreePos = glm::vec3(x, 0, z);

        // Check if the new position is too close to existing trees
        bool tooClose = false;
        for (const auto& pos : treePositions) {
            if (glm::length(newTreePos - pos) < minDistance) {
                tooClose = true;
                break;
            }
        }

        // If the position is not too close, add it
        if (!tooClose) {
            treeTransform = glm::translate(treeTransform, newTreePos);
            treeTransform = glm::scale(treeTransform, glm::vec3(4, 4, 4));

            treeTransformations.push_back(treeTransform);
            treePositions.push_back(newTreePos); // Store the position
        }
    }
}

// Render all trees in the forest
void renderForest() {
    for (const auto& transform : treeTransformations) {
        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(transform));
        treeModel.draw();
    }
}

// Setup OpenGL context and load models
void setupOpenGL() {
    glEnable(GL_DEPTH_TEST); // Enable depth test for 3D rendering
    shaderProgram = createShaderProgram("../src/shaders/vertex_shader.glsl", "../src/shaders/fragment_shader.glsl");
    generateForest(TREE_COUNT); // Generate trees
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    projection = glm::perspective(glm::radians(45.0f),
                                  (float)width / (float)height,
                                  0.1f, 100.0f); // Adjust projection
}

void keyboardDown(unsigned char key, int x, int y) {
    keys[key] = true;
}

void keyboardUp(unsigned char key, int x, int y) {
    keys[key] = false;
}

bool checkCollision(const glm::vec3& cameraPosition, const glm::vec3& treePos) {
    float distance = glm::length(cameraPosition - treePos);
    return distance < abs(camera.getCameraHeight()) + TREE_RADIUS;
}

void processKeyboard() {
    glm::vec3 newCameraPos = camera.getNewCameraPosition(keys);
    if (keys[27]) exit(0);

    bool collision = false;
    for (const auto& transform : treeTransformations) {
        glm::vec3 treePos = glm::vec3(transform[3]);
        if (checkCollision(newCameraPos, treePos)) {
            collision = true;
            break;
        }
    }

    if (!collision) camera.setCameraPos(newCameraPos);
}

void mouseMotion(int x, int y) {
    camera.adjustCameraCenter(x, y);
}

void renderScene() {
    processKeyboard();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgram);

    view = camera.applyView();

    GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");

    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    renderForest();
    glutSwapBuffers();
}

void update(int value) {
    glutPostRedisplay();
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
    update(1000);
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);
    glutPassiveMotionFunc(mouseMotion);

    glutSetCursor(GLUT_CURSOR_NONE);

    glutMainLoop();
    return 0;
}