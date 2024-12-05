#ifndef WOOD_H
#define WOOD_H

#include <GL/glew.h>
#include <GL/glut.h>
#include "environment.h"
#include "model_loader.h"
#include <vector>
#include <ctime>
#include <chrono>

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
    ModelLoader treeModel, monsterModel, grassModel, skyModel;

    glm::mat4 skyTransformation;
    std::vector<glm::mat4> treeTransformations, grassTransformations;
    std::vector<Monster> monsters;
    std::chrono::steady_clock::time_point lastMonsterSpawnTime;

    void generateForest(int treeCount);
    void generateMonsters(int monsterCount);

    void renderMonsters();
    void renderGrass();
    void renderForest();
    void renderSky();
    void renderHearts();
    void renderAttackOverlay();

    bool checkCollision(const glm::vec3& cameraPosition, const glm::vec3& treePos);
    bool checkMonsterCollision(const glm::vec3& cameraPosition);
    void processMonsterAttack();

public:
    const float WOODS_SIZE = 100.0f;
    const int TREE_COUNT = 30;
    const float TREE_RADIUS = 1.0f;
    const int MONSTER_COUNT = 4;
    const float MONSTER_RADIUS = 1.5f;
    const float MONSTER_ATTACK_RANGE = 3.0f;
    int playerLives = 3;
    const float FOREST_MIN_X = -50.0f;
    const float FOREST_MAX_X = 50.0f;
    const float FOREST_MIN_Z = -50.0f;
    const float FOREST_MAX_Z = 50.0f;

    std::chrono::steady_clock::time_point lastAttackTime;
    bool isBeingAttacked = false;
    float attackPushbackDistance = 2.f;

    Wood(GLuint shaderProgram, glm::mat4 &projection);
    void init() override;
    void renderScene() override;
    void updateScene() override;
    void processKeyboard() override;

};

#endif