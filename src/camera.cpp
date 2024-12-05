#include "camera.h"

glm::vec3 Camera::getNewCameraPosition(bool keys[256]) {
    glm::vec3 newCameraPos = cameraPos;
    glm::vec3 movementDirection = cameraFront;

    if (currentCameraMode == THIRD_PERSON) {
        movementDirection.y = 0;
        movementDirection = glm::normalize(movementDirection);
    }

    if (keys['w']) newCameraPos += CAMERA_SPEED * movementDirection;
    if (keys['s']) newCameraPos -= CAMERA_SPEED * movementDirection;
    if (keys['a']) newCameraPos -= glm::normalize(glm::cross(movementDirection, cameraUp)) * CAMERA_SPEED;
    if (keys['d']) newCameraPos += glm::normalize(glm::cross(movementDirection, cameraUp)) * CAMERA_SPEED;

    // Toggle camera mode
    if (keys['c']) {
        currentCameraMode = (currentCameraMode == FIRST_PERSON) ? THIRD_PERSON : FIRST_PERSON;
        keys['c'] = false;
    }

    // Maintain camera height based on mode
    if (currentCameraMode == FIRST_PERSON) {
        newCameraPos.y = CAMERA_HEIGHT;
    }

    return newCameraPos;
}

glm::mat4 Camera::applyView() {
    glm::vec3 actualCameraPos = cameraPos;
    if (currentCameraMode == THIRD_PERSON) {
        // Calculate third-person camera position
        glm::vec3 offset = -cameraFront * THIRD_PERSON_DISTANCE;
        offset.y += 2.0f;  // Lift the camera up a bit
        actualCameraPos = cameraPos + offset;
    }

    return glm::lookAt(actualCameraPos, cameraPos + cameraFront, cameraUp);
}

void Camera::adjustCameraCenter(int x, int y) {
    // Static variables to track mouse movement deltas
    static int lastX = 0;
    static int lastY = 0;
    static bool firstMouse = true;

    if (firstMouse) {
        lastX = x;
        lastY = y;
        firstMouse = false;
        return;
    }

    // Calculate the offset since last mouse movement
    float xoffset = x - lastX;
    float yoffset = lastY - y;  // Reversed to match your original code

    // Update last positions
    lastX = x;
    lastY = y;

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    // Clamp pitch to prevent camera flipping
    if (pitch > 89.0f)  pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    // Calculate new camera front vector
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

glm::vec3 Camera::getFrontVector() {
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    return glm::normalize(front);
}

glm::vec3 Camera::getCameraPos() {
    return cameraPos;
}

glm::vec3 Camera::getCameraCenter() {
    return cameraPos + cameraFront;
}

glm::vec3 Camera::getCameraUp() {
    return cameraUp;
}

float Camera::getCameraHeight() {
    return CAMERA_HEIGHT;
}

float Camera::getCameraSpeed() {
    return CAMERA_SPEED;
}

void Camera::setCameraPos(glm::vec3 cameraPos) {
    this->cameraPos = cameraPos;
}
