#ifndef CAMERA_H
#define CAMERA_H

#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

enum CameraMode {
    FIRST_PERSON,
    THIRD_PERSON
};

class Camera {
private:
    glm::vec3 cameraPos, cameraFront, cameraUp;
    CameraMode currentCameraMode = FIRST_PERSON;
    float yaw, pitch, centerX, centerY;
    const float CAMERA_SPEED = 0.5f, CAMERA_HEIGHT = -1.5f;

public:
    Camera() = default;
    Camera(glm::vec3 cameraPos, glm::vec3 cameraFront, glm::vec3 cameraUp,
           float yaw, float pitch, float centerX, float centerY) {
        cameraPos.y = CAMERA_HEIGHT;
        this->cameraPos = cameraPos;
        this->cameraFront = cameraFront;
        this->cameraUp = cameraUp;
        this->yaw = yaw;
        this->pitch = pitch;
        this->centerX = centerX;
        this->centerY = centerY;
    }

    glm::vec3 getNewCameraPosition(bool keys[256]);
    void adjustCameraCenter(int x, int y);

    glm::vec3 getCameraPos();
    glm::vec3 getCameraCenter();
    glm::vec3 getCameraUp();
    float getCameraHeight();
    float getCameraSpeed();

    void setCameraPos(glm::vec3 cameraPos);
};
#endif
