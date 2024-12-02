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
 * @brief Load and compile a shader from a file
 *
 * @param shaderPath Path to the shader file
 * @param shaderType The type of shader (e.g., GL_VERTEX_SHADER, GL_FRAGMENT_SHADER)
 * @return GLuint The shader ID
 *
 * This function loads a shader file from disk, compiles it, and returns the
 * compiled shader ID. If compilation fails, an error message is printed.
 */
GLuint loadShader(const char* shaderPath, GLenum shaderType) {
    // Open shader file
    std::ifstream shaderFile(shaderPath);
    if (!shaderFile.is_open()) {
        std::cerr << "Failed to load shader file: " << shaderPath << std::endl;
        return 0;
    }

    // Read shader code from the file
    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    std::string shaderCode = shaderStream.str();
    const char* shaderSource = shaderCode.c_str();

    // Create shader object
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, nullptr);
    glCompileShader(shader);

    // Check shader compilation status
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
 * @brief Load a shader program by linking vertex and fragment shaders
 *
 * @param vertexShaderPath Path to the vertex shader file
 * @param fragmentShaderPath Path to the fragment shader file
 * @return GLuint The shader program ID
 *
 * This function loads the vertex and fragment shaders, compiles them, and links
 * them into a shader program. The program ID is returned. If linking fails, an
 * error message is printed.
 */
GLuint loadShaderProgram(const char* vertexShaderPath, const char* fragmentShaderPath) {
    GLuint vertexShader = loadShader(vertexShaderPath, GL_VERTEX_SHADER);
    GLuint fragmentShader = loadShader(fragmentShaderPath, GL_FRAGMENT_SHADER);

    // Create shader program and link shaders
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check linking status
    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
    }

    // Clean up shaders as they're no longer needed after being linked
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

/**
 * @brief Initialize OpenGL settings
 *
 * This function sets up the viewport, configures the camera projection matrix,
 * and initializes the shader program.
 */
void initializeOpenGL() {
    // Initialize shaders
    shaderProgram = loadShaderProgram("../shaders/vertex_shader.glsl", "../shaders/fragment_shader.glsl");

    // Set up projection matrix
    projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);

    // Set up view matrix (camera position and orientation)
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    // Initialize model loaders
    modelLoader1.loadModel("model1");
    modelLoader2.loadModel("model2");
}

/**
 * @brief Process keyboard inputs for camera movement
 *
 * This function updates the camera position based on keyboard inputs.
 * WASD keys are used for movement, and R and F keys control the speed.
 */
void processKeyboardInput() {
    if (keys['W']) {
        cameraPos += cameraSpeed * cameraFront;
    }
    if (keys['S']) {
        cameraPos -= cameraSpeed * cameraFront;
    }
    if (keys['A']) {
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    if (keys['D']) {
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    if (keys['R']) {
        cameraSpeed += 0.01f;
    }
    if (keys['F']) {
        cameraSpeed -= 0.01f;
    }
}

/**
 * @brief Process mouse movement to update camera direction
 *
 * This function updates the yaw and pitch based on mouse movements, and updates
 * the camera front vector to reflect changes in direction.
 *
 * @param xpos Current x-position of the mouse
 * @param ypos Current y-position of the mouse
 */
void processMouseMovement(float xpos, float ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xOffset = xpos - lastX;
    float yOffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.05f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    yaw   += xOffset;
    pitch += yOffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

/**
 * @brief Render the scene with updated camera and model
 *
 * This function clears the screen, updates the view matrix, and renders
 * the models using the loaded shader program.
 */
void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use the shader program
    glUseProgram(shaderProgram);

    // Update view matrix based on camera position and orientation
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Draw models
    modelLoader1.draw();
    modelLoader2.draw();

    glutSwapBuffers();
}
