// GL liberaries
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
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
    if (!environment) {
        if (key == 27) {
            exit(0);
        }
        return;
    }
    environment->keyboardDown(key, x, y);
}

void keyboardUp(unsigned char key, int x, int y) {
    if (!environment) return;
    environment->keyboardUp(key, x, y);
}

void mouseMotion(int x, int y) {
    if (!environment) return;
    environment->mouseMotion(x, y);
}

void renderScene() {
    if (!environment) return;
    environment->renderScene();
}

void onMouseClick(int button, int state, int x, int y) {
    if (!environment) return;
    environment->onMouseClick(button, state, x, y);
}

int score;
void renderBitmapString(float x, float y, void* font, const std::string& text,
                        float r, float g, float b) {
    // Ensure we're in pure 2D mode
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    int width = glutGet(GLUT_WINDOW_WIDTH);
    int height = glutGet(GLUT_WINDOW_HEIGHT);

    // Use screen coordinates directly
    glOrtho(0, width, 0,
            height, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Disable depth testing
    glDisable(GL_DEPTH_TEST);

    // Disable current shader
    glUseProgram(0);

    // Set text color
    glColor3f(r, g, b);

    // Calculate text width using stroke font
    float totalWidth = 0;
    for (char c : text) totalWidth += glutStrokeWidth(GLUT_STROKE_ROMAN, c);

    // Position text at bottom center
    float scaleFactor = 0.6f;  // Adjust for desired size
    float xPos = (width - totalWidth * scaleFactor) / 2.0f;
    float yPos = height * 0.15f;

    // Translate and scale
    glPushMatrix();
    glTranslatef(xPos, yPos, 0);
    glScalef(scaleFactor, scaleFactor, scaleFactor);

    for (char c : text) {
        glLineWidth(13.0f);
        glutStrokeCharacter(GLUT_STROKE_ROMAN, c);
        glLineWidth(1.0f);
    }

    glPopMatrix();

    // Restore matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void displayWinScreen() {
    // Get window dimensions
    int width = glutGet(GLUT_WINDOW_WIDTH);
    int height = glutGet(GLUT_WINDOW_HEIGHT);

    // Clear with a darker green
    glClearColor(0.0f, 0.2f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Calculate dynamic positioning
    float centerX = width / 2.0f;
    float centerY = height / 2.0f;

    // Large "YOU WIN!" text
    renderBitmapString(
            centerX - 100,  // Adjusted X position
            centerY,
            GLUT_BITMAP_TIMES_ROMAN_24,
            "YOU WIN!\nYour Score is: " + std::to_string(score),
            1.0f, 1.0f, 1.0f  // Bright yellow
    );

    glutSwapBuffers();
}

void displayGameOverScreen() {
    // Get window dimensions
    int width = glutGet(GLUT_WINDOW_WIDTH);
    int height = glutGet(GLUT_WINDOW_HEIGHT);

    // Clear with a darker red
    glClearColor(0.3f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Calculate dynamic positioning
    float centerX = width / 2.0f;
    float centerY = height / 2.0f;

    // Large "GAME OVER" text
    renderBitmapString(
            centerX - 150,  // Adjusted X position
            centerY,
            GLUT_BITMAP_TIMES_ROMAN_24,
            "GAME OVER",
            1.0f, 0.0f, 0.0f  // Bright red
    );

    glutSwapBuffers();
}

void update(int value) {
    if (!environment) {
        glutPostRedisplay();
        glutTimerFunc(16, update, 0);
        return;
    }
    environment->updateScene();
    if (!environment->isRunning) { // scene ended
        if (dynamic_cast<House*>(environment)) {
            environment->cleanUp();
            environment = new Wood(shaderProgram, projection);
            environment->init();
        } else if (dynamic_cast<Wood*>(environment)) {
            environment->cleanUp();
            if (environment->isWinning){
                // game win!
                score = environment->score;
                glutDisplayFunc(displayWinScreen);
            } else {
                // game over!
                glutDisplayFunc(displayGameOverScreen);
            }
            glutPostRedisplay();
            environment = nullptr;
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