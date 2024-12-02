#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "model_loader.h"

const int WIDTH = 2400, HEIGHT = 1800;

// Camera system variables
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

// Mouse movement variables
float yaw   = -90.0f;  // Yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right
float pitch =  0.0f;
float lastX =  WIDTH / 2.0f;
float lastY =  HEIGHT / 2.0f;
bool firstMouse = true;

// Movement speed
float cameraSpeed = 0.05f;

// Keyboard state tracking
bool keys[256] = {false};

// Shader and model loader (global variables)
GLuint shaderProgram;
ModelLoader modelLoader1, modelLoader2;
glm::mat4 projection, view;

/**
 * @brief Load shader from file
 *
 * @param shaderPath Path to the shader file
 * @param shaderType Type of shader to load (vertex or fragment)
 * @return GLuint Shader ID or 0 if loading fails
 *
 * This function reads a shader file, compiles the shader, and checks for errors.
 */
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

/**
 * @brief Create a shader program from vertex and fragment shaders
 *
 * @param vertexPath Path to the vertex shader file
 * @param fragmentPath Path to the fragment shader file
 * @return GLuint Shader program ID
 *
 * This function creates, attaches, and links vertex and fragment shaders into a shader program.
 */
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

/**
 * @brief Setup OpenGL context and load models
 *
 * This function initializes OpenGL settings, such as enabling depth testing,
 * creates the shader program, and loads 3D models using the ModelLoader.
 */
void setupOpenGL() {
    glEnable(GL_DEPTH_TEST); // Enable depth test for 3D rendering
    shaderProgram = createShaderProgram("../src/shaders/vertex_shader.glsl", "../src/shaders/fragment_shader.glsl");

    // Load models
    modelLoader1.loadModel("monster");
    modelLoader2.loadModel("spider_man");
}

/**
 * @brief Handle window resizing
 *
 * @param width New width of the window
 * @param height New height of the window
 *
 * This function adjusts the viewport size and updates the projection matrix
 * to maintain the aspect ratio.
 */
void reshape(int width, int height) {
    glViewport(0, 0, width, height); // Set the viewport size
    projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f); // Adjust projection
}

/**
 * @brief Keyboard key down event handler
 *
 * @param key The key that was pressed
 * @param x The x-coordinate of the mouse pointer
 * @param y The y-coordinate of the mouse pointer
 *
 * This function updates the state of the keyboard when a key is pressed.
 */
void keyboardDown(unsigned char key, int x, int y) {
    keys[key] = true;
}

/**
 * @brief Keyboard key up event handler
 *
 * @param key The key that was released
 * @param x The x-coordinate of the mouse pointer
 * @param y The y-coordinate of the mouse pointer
 *
 * This function updates the state of the keyboard when a key is released.
 */
void keyboardUp(unsigned char key, int x, int y) {
    keys[key] = false;
}

/**
 * @brief Process continuous key press for camera movement
 *
 * This function updates the camera position based on the currently pressed keys.
 */
void processKeyboard() {
    // Movement along camera's front and right vectors
    if (keys['w']) cameraPos += cameraSpeed * cameraFront;
    if (keys['s']) cameraPos -= cameraSpeed * cameraFront;
    if (keys['a']) cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (keys['d']) cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

/**
 * @brief Mouse motion callback for camera control
 *
 * @param x The x-coordinate of the mouse pointer
 * @param y The y-coordinate of the mouse pointer
 *
 * This function updates the camera's yaw and pitch based on mouse movement.
 */
void mouseMotion(int x, int y) {
    if (firstMouse) {
        lastX = x;
        lastY = y;
        firstMouse = false;
    }

    float xoffset = x - lastX;
    float yoffset = lastY - y; // Reversed since y-coordinates go from bottom to top
    lastX = x;
    lastY = y;

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    // Constrain pitch to prevent screen flip
    if (pitch > 89.0f)  pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    // Calculate new front vector
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

/**
 * @brief Render the scene
 *
 * This function clears the screen, sets the matrices for rendering,
 * and draws the loaded 3D models.
 */
void renderScene() {
    // Process continuous keyboard input
    processKeyboard();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgram);

    // Adjust projection with wider aspect ratio
    projection = glm::perspective(glm::radians(45.0f), 2400.0f / 1800.0f, 0.1f, 100.0f);

    // Update view matrix with camera movement
    view = glm::lookAt(
            cameraPos,            // Camera position
            cameraPos + cameraFront,  // Look at point (camera position + front vector)
            cameraUp              // Up vector
    );

    // Position Spiderman on the left
    glm::mat4 spidermanModel = glm::mat4(1.0f);
    spidermanModel = glm::translate(spidermanModel, glm::vec3(-2.0f, 0.0f, 0.0f)); // Move left
    spidermanModel = glm::rotate(spidermanModel, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate to face right
    spidermanModel = glm::scale(spidermanModel, glm::vec3(1.5f, 1.5f, 1.5f));

    // Position Monster on the right
    glm::mat4 monsterModel = glm::mat4(1.0f);
    monsterModel = glm::translate(monsterModel, glm::vec3(2.0f, 0.0f, 0.0f)); // Move right
    monsterModel = glm::rotate(monsterModel, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate to face left
    monsterModel = glm::scale(monsterModel, glm::vec3(1.5f, 1.5f, 1.5f));

    // Set shader uniform variables
    GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");

    // Set projection and view matrices
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // Draw Spiderman
    {
        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(spidermanModel));
        modelLoader2.draw(); // Spiderman model
    }

    // Draw Monster
    {
        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(monsterModel));
        modelLoader1.draw(); // Monster model
    }

    glutSwapBuffers();
}

/**
 * @brief Update function for the render loop
 *
 * @param value Timer value (unused)
 *
 * This function triggers a redraw of the scene and sets up a timer
 * for continuous updates.
 */
void update(int value) {
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

/**
 * @brief Main entry point of the application
 *
 * @param argc Argument count
 * @param argv Argument values
 * @return int Exit status
 *
 * This function initializes GLUT, sets up the rendering context,
 * and enters the main event loop.
 */
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    // Increase window size to 3x
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Escape The Abyss");

    // Initialize GLEW after creating the window
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error initializing GLEW: " << glewGetErrorString(err) << std::endl;
        return -1;
    }

    setupOpenGL(); // Set up OpenGL and load the model

    // Register callbacks
    glutDisplayFunc(renderScene);
    glutReshapeFunc(reshape);
    glutTimerFunc(25, update, 0);

    // Keyboard callbacks
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);

    // Mouse callbacks
    glutPassiveMotionFunc(mouseMotion);

    // Hide cursor and capture it
    glutSetCursor(GLUT_CURSOR_NONE);

    glutMainLoop();
    return 0;
}