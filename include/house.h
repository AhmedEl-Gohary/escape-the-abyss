#ifndef HOUSE_H
#define HOUSE_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <chrono>
#include "environment.h"
#include "model_loader.h"

class Wall {
public:
    glm::vec3 lowerPoint, upperPoint;

    Wall(const glm::vec3& lower, const glm::vec3& upper)
            : lowerPoint(lower), upperPoint(upper) {}

    bool checkCollision(const glm::vec3& point, float collisionRadius) const {
        // Check if the point (with radius) is within the wall's bounds
        return (point.x + collisionRadius >= lowerPoint.x &&
                point.x - collisionRadius <= upperPoint.x &&
                point.y + collisionRadius >= lowerPoint.y &&
                point.y - collisionRadius <= upperPoint.y &&
                point.z + collisionRadius >= lowerPoint.z &&
                point.z - collisionRadius <= upperPoint.z);
    }
};

class House : public Environment {
private:
    ModelLoader floorModel, wallModel, flashlightModel, lightbulbModel, keyModel, doorModel;
    glm::mat4 flashlightTransformation, lightbulbTransformation,
        keyTransformation, doorTransformation;

    std::vector<Wall> walls;
    std::vector<glm::mat4> floorTransformations;
    bool isKeyEquipped;
    bool isFlashlightEquipped;
    bool isNearCollectible;
    bool isShowingPickupPrompt;

    // Spawn and management methods
    void generateWalls();
    void generateKey();
    void generateDoor();
    void generateFlashlight();
    void generateLightbulb();
    void generateFloor();
    void renderWall(Wall &wall);
    void renderWalls();
    void renderKey();
    void renderDoor();
    void renderFlashlight();
    void renderLightbulb();
    void renderFloor();
    void renderEquippedFlashlight();
    void renderPickupPrompt(const std::string& text);

    // Collision and interaction methods
    bool checkCollision(const glm::vec3& cameraPosition, const glm::vec3& treePos);
    bool checkCollectibleCollision(const glm::vec3& cameraPosition);
    void toggleFlashlight();

public:
    // Constants
    static constexpr float WOODS_SIZE = 100.0f;
    static constexpr int TREE_COUNT = 30;
    static constexpr float TREE_RADIUS = 1.0f;
    static constexpr int MONSTER_COUNT = 4;
    static constexpr int COLLECTIBLE_COUNT = 2;
    static constexpr float MONSTER_RADIUS = 1.5f;
    static constexpr float MONSTER_ATTACK_RANGE = 3.0f;
    static constexpr float COLLECTIBLE_PICKUP_RANGE = 2.0f;
    static constexpr float SWORD_ATTACK_RANGE = 5.0f;
    static constexpr float TORCH_SPEED_REDUCTION = 0.05f;

    int playerLives = 3;

    static constexpr float MIN_X = -25.0f;
    static constexpr float MAX_X = 25.0f;
    static constexpr float MIN_Z = -25.0f;
    static constexpr float MAX_Z = 25.0f;

    std::chrono::steady_clock::time_point lastAttackTime;
    bool isBeingAttacked = false;
    float attackPushbackDistance = 2.f;

    House(GLuint shaderProgram, glm::mat4 &projection);

    void init() override;
    void renderScene() override;
    void updateScene() override;
    void processKeyboard() override;
    void onMouseClick(int button, int state, int x, int y) override;
};

#endif
