#ifndef CAMERA_H
#define CAMERA_H

#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "game.h"

// Camera system variables
const glm::vec3 initialCameraPos   = glm::vec3(0.0f, 0.0f, 5.0f);
const glm::vec3 initialCameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
const glm::vec3 initialCameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

// Mouse movement variables
const float initialYaw = -90.0f, initialPitch = 0.0f;
const float initialCenterX = WIDTH / 2.0f;
const float initialCenterY = HEIGHT / 2.0f;

enum CameraMode {
    FIRST_PERSON,
    THIRD_PERSON
};

class Camera {
private:
    glm::vec3 cameraPos{}, cameraFront{}, cameraUp{};
    CameraMode currentCameraMode = FIRST_PERSON;
    float yaw, pitch, centerX, centerY;
    float CAMERA_SPEED = 0.05f, CAMERA_HEIGHT = -1.5f, THIRD_PERSON_DISTANCE = 5.0f;

public:
    Camera() {
        this->cameraPos = initialCameraPos;
        this->cameraPos.y = CAMERA_HEIGHT;
        this->cameraFront = initialCameraFront;
        this->cameraUp = initialCameraUp;
        this->yaw = initialYaw;
        this->pitch = initialPitch;
        this->centerX = initialCenterX;
        this->centerY = initialCenterY;
    }

    glm::vec3 getNewCameraPosition(bool keys[256]);
    glm::mat4 applyView();
    void adjustCameraCenter(int x, int y);

    glm::vec3 getCameraPos();
    glm::vec3 getCameraCenter();
    glm::vec3 getCameraUp();
    float getCameraHeight();
    float getCameraSpeed();

    void setCameraPos(glm::vec3 cameraPos);
    void setCameraHeight(float height);
};
#endif
