#ifndef WOOD_H
#define WOOD_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <chrono>
#include "environment.h"
#include "model_loader.h"

class Collectible {
public:
    enum Type {
        SWORD,
        TORCH
    };

    glm::vec3 position;
    Type type;
    bool isEquipped;
    bool isVisible;

    Collectible(const glm::vec3& pos, Type t)
            : position(pos), type(t), isEquipped(false), isVisible(true) {}
};

class Monster {
public:
    glm::vec3 position;
    float speed, rotationAngle;
    bool isAlive, isMoving;

    Monster(const glm::vec3& spawnPos);
    void update(const glm::vec3& playerPosition, const std::vector<Monster>& allMonsters);
    bool checkCollisionWithPlayer(const glm::vec3& playerPosition, float collisionRadius, float fieldOfViewAngle);
    float generateRandomSpeed();
    float calculateRotationAngle(const glm::vec3&);
};

class Wood : public Environment {
private:
    glm::vec3 playerPosition;
    float playerRotation = 0;
    bool isJumping = false;
    float speed = 0, gravity = 0.16, playerY = -3.5;
    ModelLoader treeModel, monsterModel, playerModel,
            swordModel, torchModel, grassModel, skyModel;

    glm::mat4 skyTransformation;
    std::vector<glm::mat4> treeTransformations;
    std::vector<glm::mat4> grassTransformations;

    std::vector<Monster> monsters;
    std::vector<Collectible> collectibles;

    Collectible* equippedSword;
    Collectible* equippedTorch;
    bool isNearCollectible;
    bool isShowingPickupPrompt;
    bool isSwordSwinging;
    float swordSwingProgress;
    float swordSwingAngle;

    // Spawn and management methods
    void generateForest(int treeCount);
    void generateMonsters(int monsterCount);
    void generateCollectibles(int collectibleCount);

    void renderPlayer();
    void renderMonsters();
    void renderCollectibles();
    void renderGrass();
    void renderForest();
    void renderSky();
    void renderCrosshair();
    void renderHearts();
    void renderAttackOverlay();
    void renderSwordSwing();
    void renderEquippedCollectibles();
    void renderPickupPrompt(const std::string& text);

    // Collision and interaction methods
    bool checkCollision(const glm::vec3& cameraPosition, const glm::vec3& treePos);
    bool checkMonsterCollision(const glm::vec3& cameraPosition);
    Collectible* checkCollectibleCollision(const glm::vec3& cameraPosition);
    void processMonsterAttack();
    void processCollectiblePickup(Collectible* collectible);
    void processSwordAttack();

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

    static constexpr float FOREST_MIN_X = -50.0f;
    static constexpr float FOREST_MAX_X = 50.0f;
    static constexpr float FOREST_MIN_Z = -50.0f;
    static constexpr float FOREST_MAX_Z = 50.0f;

    std::chrono::steady_clock::time_point lastAttackTime;
    bool isBeingAttacked = false;
    float attackPushbackDistance = 2.f;

    Wood(GLuint shaderProgram, glm::mat4 &projection);
    void init() override;
    void renderScene() override;
    void updateScene() override;
    void processKeyboard() override;
    void onMouseClick(int button, int state, int x, int y) override;
};

#endif