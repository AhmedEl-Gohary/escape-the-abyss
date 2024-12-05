#include "wood.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <random>
#include <iostream>

Monster::Monster(const glm::vec3& spawnPos)
        : position(spawnPos),
          speed(generateRandomSpeed()),
          isAlive(true),
          isMoving(true) {}

float Monster::generateRandomSpeed() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.1f, 0.15f);
    return dis(gen);
}

void Monster::update(const glm::vec3& playerPosition, const std::vector<Monster>& allMonsters) {
    if (!isAlive || !isMoving) return;

    calculateRotationAngle(playerPosition);

    glm::vec3 currentPosition = position;
    glm::vec3 targetPosition = playerPosition;
    currentPosition.y = targetPosition.y;

    // Calculate direction to player
    glm::vec3 directionToPlayer = glm::normalize(targetPosition - currentPosition);

    // Separation vector to avoid other monsters
    glm::vec3 separationVector(0.0f, 0.0f, 0.0f);
    float separationRadius = 5.0f;  // Minimum distance to maintain between monsters
    int nearbyMonsters = 0;

    for (const auto& otherMonster : allMonsters) {
        if (&otherMonster == this) continue;  // Skip self

        glm::vec3 distanceVector = currentPosition - otherMonster.position;
        float distance = glm::length(distanceVector);

        if (distance < separationRadius) {
            // Normalize and inverse the distance vector
            glm::vec3 separationDir = glm::normalize(distanceVector);
            float weight = 1.0f - (distance / separationRadius);
            separationVector += separationDir * weight;
            nearbyMonsters++;
        }
    }

    // Average out separation vector if there are nearby monsters
    if (nearbyMonsters > 0) {
        separationVector /= nearbyMonsters;
    }

    // Combine player-seeking and separation behaviors
    glm::vec3 finalMovement = directionToPlayer + separationVector * 0.5f;
    finalMovement = glm::normalize(finalMovement);

    glm::vec3 movement = finalMovement * speed;
    movement.y = 0.0f;

    position += movement;
}

float Monster::calculateRotationAngle(const glm::vec3& playerPosition) {
    glm::vec3 currentPosition = position;
    glm::vec3 targetPosition = playerPosition;

    currentPosition.y = 0.0f;
    targetPosition.y = 0.0f;

    glm::vec3 direction = glm::normalize(targetPosition - currentPosition);

    float angle = atan2(direction.x, direction.z);

    rotationAngle = glm::degrees(angle);

    return rotationAngle;
}

bool Monster::checkCollisionWithPlayer(const glm::vec3& playerPosition, float collisionRadius, float fieldOfViewAngle) {
    float distance = glm::length(playerPosition - position);

    // Calculate direction to player
    glm::vec3 directionToPlayer = glm::normalize(playerPosition - position);

    // Calculate angle
    float angle = calculateRotationAngle(playerPosition);
    float absoluteAngle = std::abs(angle);

    // Check if player is within the specified field of view
    return (distance < collisionRadius) && (absoluteAngle <= fieldOfViewAngle / 2.0f);
}

Wood::Wood(GLuint shaderProgram, glm::mat4 &projection)
        : Environment(shaderProgram, projection),
          equippedSword(nullptr),
          equippedTorch(nullptr),
          isNearCollectible(false),
          isShowingPickupPrompt(false),
          isSwordSwinging(false),
          swordSwingProgress(0.0f),
          swordSwingAngle(0.0f) {}


void Wood::generateCollectibles(int collectibleCount) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> disX(-WOODS_SIZE/2, WOODS_SIZE/2);
    std::uniform_real_distribution<> disZ(-WOODS_SIZE/2, WOODS_SIZE/2);

    swordModel.loadModel("sword");
    torchModel.loadModel("torch");

    for (int i = 0; i < collectibleCount; ++i) {
        glm::vec3 collectiblePos;
        Collectible::Type type = (i == 0) ? Collectible::SWORD : Collectible::TORCH;
        bool validPosition;

        do {
            validPosition = true;
            collectiblePos = glm::vec3(disX(gen), -3, disZ(gen));

            for (const auto& treeTransform : treeTransformations) {
                glm::vec3 treePos = glm::vec3(treeTransform[3]);
                if (glm::length(collectiblePos - treePos) < TREE_RADIUS * 10) {
                    validPosition = false;
                    break;
                }
            }
        } while (!validPosition);

        collectibles.emplace_back(collectiblePos, type);
    }
}

void Wood::renderCollectibles() {
    // Get the elapsed time in seconds
    float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f; // Convert milliseconds to seconds

    for (const auto& collectible : collectibles) {
        if (!collectible.isVisible) continue;

        glm::mat4 collectibleTransform = glm::mat4(1.0f);

        // Apply translation to position the collectible
        collectibleTransform = glm::translate(collectibleTransform, collectible.position);

        // Apply periodic rotation around the Y-axis
        float rotationAngle = glm::radians(currentTime * 50.0f); // 50 degrees per second
        collectibleTransform = glm::rotate(collectibleTransform, rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));

        // Apply scaling
        collectibleTransform = glm::scale(collectibleTransform, glm::vec3(1.0f, 1.0f, 1.0f));

        // Set the transformation matrix in the shader
        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(collectibleTransform));

        // Draw the appropriate model based on the collectible type
        if (collectible.type == Collectible::SWORD) {
            swordModel.draw();
        } else {
            torchModel.draw();
        }
    }
}


Collectible* Wood::checkCollectibleCollision(const glm::vec3& cameraPosition) {
    for (auto& collectible : collectibles) {
        if (!collectible.isVisible || collectible.isEquipped) continue;

        float distance = glm::length(cameraPosition - collectible.position);
        if (distance < COLLECTIBLE_PICKUP_RANGE) {
            return &collectible;
        }
    }
    return nullptr;
}

void Wood::processCollectiblePickup(Collectible* collectible) {
    if (collectible->type == Collectible::SWORD) {
        equippedSword = collectible;
        collectible->isEquipped = true;
        collectible->isVisible = false;
    }
    else if (collectible->type == Collectible::TORCH) {
        equippedTorch = collectible;
        collectible->isEquipped = true;
        collectible->isVisible = false;
        for (auto& monster : monsters) monster.speed -= TORCH_SPEED_REDUCTION;
    }
}

void Wood::processSwordAttack() {
    if (!isSwordSwinging && equippedSword) {
        isSwordSwinging = true;
        swordSwingProgress = 0.0f;

        glm::vec3 playerPos = camera.getNewCameraPosition(keys);
        glm::vec3 playerFront = camera.getFrontVector();

        for (auto& monster : monsters) {
            glm::vec3 monsterToPlayer = glm::normalize(monster.position - playerPos);
            float angle = glm::degrees(std::acos(glm::dot(playerFront, monsterToPlayer)));

            float distanceToMonster = glm::length(monster.position - playerPos);

            if (distanceToMonster <= SWORD_ATTACK_RANGE && angle <= 45.0f) {
                monster.isAlive = false;
            }
        }
    }
}

void Wood::renderSwordSwing() {
    if (!isSwordSwinging) return;

    swordSwingProgress += 0.2f;
    swordSwingAngle = std::sin(swordSwingProgress) * 45.0f;

    if (swordSwingProgress >= M_PI) {
        isSwordSwinging = false;
        swordSwingProgress = 0.0f;
        swordSwingAngle = 0.0f;
    }
}

void Wood::renderEquippedCollectibles() {
    // Early exit if neither collectible is equipped
    if (!equippedSword && !equippedTorch) return;

    // Get camera properties
    glm::vec3 cameraPos = camera.getCameraPos();
    glm::vec3 cameraFront = camera.getFrontVector();
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));

    // Set transformations for both collectibles
    glm::mat4 swordTransform = glm::mat4(1.0f);
    glm::mat4 torchTransform = glm::mat4(1.0f);

    // Offsets for holding sword and torch
    float swordOffsetX = 0.2f; // Right hand
    float swordOffsetY = -0.2f; // Below the camera
    float swordOffsetZ = 0.3f;  // Forward

    float torchOffsetX = -0.2f; // Left hand
    float torchOffsetY = -0.2f; // Below the camera
    float torchOffsetZ = 0.3f;  // Forward

    // Positioning the sword
    if (equippedSword) {
        swordTransform = glm::translate(swordTransform,
                                        cameraPos + cameraFront * swordOffsetZ +
                                        cameraUp * swordOffsetY +
                                        cameraRight * swordOffsetX
        );

        // Rotation to face forward
        swordTransform = glm::rotate(swordTransform, glm::radians(-90.0f), glm::vec3(0, 1, 0));

        // Apply swing animation if swinging
        if (isSwordSwinging) {
            swordTransform = glm::rotate(swordTransform,
                                         glm::radians(swordSwingAngle),
                                         glm::vec3(1, 0, 0)
            );
        }

        GLuint swordModelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(swordModelLoc, 1, GL_FALSE, glm::value_ptr(swordTransform));
        swordModel.draw();
    }

    // Positioning the torch
    if (equippedTorch) {
        torchTransform = glm::translate(torchTransform,
                                        cameraPos + cameraFront * torchOffsetZ +
                                        cameraUp * torchOffsetY +
                                        cameraRight * torchOffsetX
        );

        // Rotate torch to face forward
        torchTransform = glm::rotate(torchTransform, glm::radians(90.0f), glm::vec3(0, 1, 0));

        GLuint torchModelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(torchModelLoc, 1, GL_FALSE, glm::value_ptr(torchTransform));
        torchModel.draw();
    }
}

void Wood::generateForest(int treeCount) {
    treeModel.loadModel("tree");
    grassModel.loadModel("grass2");
    skyModel.loadModel("skydome");
    srand(static_cast<unsigned int>(time(nullptr)));

    std::vector<glm::vec3> treePositions;
    float minDistance = TREE_RADIUS * 15.0f;

    while (treePositions.size() < treeCount) {
        glm::mat4 treeTransform = glm::mat4(1.0f);
        float x = static_cast<float>(rand()) / (float)RAND_MAX * (FOREST_MAX_X - FOREST_MIN_X) + FOREST_MIN_X;
        float z = static_cast<float>(rand()) / (float)RAND_MAX * (FOREST_MAX_Z - FOREST_MIN_Z) + FOREST_MIN_Z;
        glm::vec3 newTreePos = glm::vec3(x, 0, z);

        bool tooClose = false;
        for (const auto& pos : treePositions) {
            if (glm::length(newTreePos - pos) < minDistance) {
                tooClose = true;
                break;
            }
        }

        if (!tooClose) {
            treeTransform = glm::translate(treeTransform, newTreePos);
            treeTransform = glm::scale(treeTransform, glm::vec3(4, 4, 4));

            treeTransformations.push_back(treeTransform);
            treePositions.push_back(newTreePos);
        }
    }

    const float grassSpacing = 1.5f;
    for (float x = FOREST_MIN_X; x <= FOREST_MAX_X; x += grassSpacing) {
        for (float z = FOREST_MIN_Z; z <= FOREST_MAX_Z; z += grassSpacing) {
            glm::mat4 grassTransform = glm::mat4(1.0f);
            glm::vec3 grassPos = glm::vec3(x, -5, z);

            grassTransform = glm::translate(grassTransform, grassPos);
            grassTransform = glm::scale(grassTransform, glm::vec3(0.002, 0.002, 0.002)); // Adjust size as needed
            grassTransformations.push_back(grassTransform);
        }
    }

    glm::mat4 skyTransform = glm::mat4(1.0f);
    glm::vec3 grassPos = glm::vec3(0, -5, 0);

    skyTransform = glm::translate(skyTransform, grassPos);
    skyTransform = glm::scale(skyTransform, glm::vec3(0.015, 0.015, 0.015));
    skyTransformation = skyTransform;
}

void Wood::renderGrass() {
    for (const auto& transform : grassTransformations) {
        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(transform));
        grassModel.draw();
    }
}

void Wood::renderSky() {
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(skyTransformation));
    skyModel.draw();
}

void Wood::generateMonsters(int monsterCount) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> disX(-WOODS_SIZE/2, WOODS_SIZE/2);
    std::uniform_real_distribution<> disZ(-WOODS_SIZE/2, WOODS_SIZE/2);

    monsterModel.loadModel("monster");

    for (int i = 0; i < monsterCount; ++i) {
        glm::vec3 monsterPos;
        bool validPosition;
        do {
            validPosition = true;
            monsterPos = glm::vec3(disX(gen), -2.8, disZ(gen));

            for (const auto& treeTransform : treeTransformations) {
                glm::vec3 treePos = glm::vec3(treeTransform[3]);
                if (glm::length(monsterPos - treePos) < TREE_RADIUS * 10) {
                    validPosition = false;
                    break;
                }
            }
        } while (!validPosition);

        monsters.emplace_back(monsterPos);
    }
}

void Wood::renderForest() {
    for (const auto& transform : treeTransformations) {
        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc,
                           1,
                           GL_FALSE,
                           glm::value_ptr(transform));
        treeModel.draw();
    }
}

void Wood::renderMonsters() {
    glm::vec3 playerPosition = camera.getNewCameraPosition(keys);

    for (const auto& monster : monsters) {
        if (!monster.isAlive) continue;
        glm::mat4 monsterTransform = glm::mat4(1.0f);
        glm::vec3 monsterPos = monster.position;
        monsterTransform = glm::translate(monsterTransform, monsterPos);

        glm::vec3 monsterToCamera = glm::normalize(playerPosition - monsterPos);
        monsterToCamera.y = 0.0f;

        float angle = atan2(monsterToCamera.x, monsterToCamera.z);

        angle += glm::radians(90.0f);

        monsterTransform = glm::rotate(monsterTransform, angle, glm::vec3(0, 1, 0));

        monsterTransform = glm::scale(monsterTransform, glm::vec3(1.0f, 1.0f, 1.0f));

        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(monsterTransform));

        monsterModel.draw();
    }
}


void Wood::init() {
    generateForest(TREE_COUNT);
    generateMonsters(MONSTER_COUNT);
    generateCollectibles(COLLECTIBLE_COUNT);
}

void Wood::updateScene() {
    glm::vec3 playerPosition = camera.getNewCameraPosition(keys);

    // Check for nearby collectibles
    Collectible* nearbyCollectible = checkCollectibleCollision(playerPosition);
    isNearCollectible = (nearbyCollectible != nullptr);
    isShowingPickupPrompt = isNearCollectible;

    // Render sword swing
    if (isSwordSwinging) {
        renderSwordSwing();
    }

    // Monster update logic
    for (Monster& monster : monsters) {
        if (!monster.isAlive) continue;

        float distanceToPlayer = glm::length(monster.position - playerPosition);

        if (distanceToPlayer < MONSTER_ATTACK_RANGE) {
            monster.isMoving = false;
            processMonsterAttack();
        } else {
            monster.isMoving = true;
            monster.update(playerPosition, monsters);
        }
    }

    // Check game over condition
    if (playerLives <= 0) {
        exit(0); // Or show game over screen
    }

    processKeyboard();
    glutPostRedisplay();
}

void Wood::renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgram);

    // Define the projection matrix
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 1000.0f);

    // Apply the camera view
    glm::mat4 view = camera.applyView();

    // Set the projection and view matrices in the shader
    GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // Get and set the camera position
    glm::vec3 cameraPos = camera.getCameraPos(); // Use getCameraPos() instead of getNewCameraPosition
    GLuint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
    glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPos));

    // Render scene elements
    renderForest();
    renderMonsters();
    renderCollectibles();
    renderGrass();
    renderSky();

    renderEquippedCollectibles();

    if (isShowingPickupPrompt) {
        renderPickupPrompt("Press E to pickup");
    }

    if (isBeingAttacked) {
        renderAttackOverlay();
        isBeingAttacked = false;
    }

    renderHearts();
    if (equippedSword) renderCrosshair();

    glutSwapBuffers();
}

bool Wood::checkCollision(const glm::vec3 &cameraPosition, const glm::vec3 &treePos) {
    float distance = glm::length(cameraPosition - treePos);
    return distance < abs(camera.getCameraHeight()) + TREE_RADIUS;
}

void Wood::processKeyboard() {
    glm::vec3 newCameraPos = camera.getNewCameraPosition(keys);
    if (keys[27]) exit(0);

    if (isNearCollectible && keys['e']) {
        Collectible* nearbyCollectible = checkCollectibleCollision(camera.getNewCameraPosition(keys));
        if (nearbyCollectible) {
            processCollectiblePickup(nearbyCollectible);
        }
    }

    if (newCameraPos.x < FOREST_MIN_X) newCameraPos.x = FOREST_MIN_X;
    if (newCameraPos.x > FOREST_MAX_X) newCameraPos.x = FOREST_MAX_X;
    if (newCameraPos.z < FOREST_MIN_Z) newCameraPos.z = FOREST_MIN_Z;
    if (newCameraPos.z > FOREST_MAX_Z) newCameraPos.z = FOREST_MAX_Z;

    bool collision = checkMonsterCollision(newCameraPos) ||
                     [&]() {
                         for (const auto& transform : treeTransformations) {
                             auto treePos = glm::vec3(transform[3]);
                             if (checkCollision(newCameraPos, treePos)) {
                                 return true;
                             }
                         }
                         return false;
                     }();

    if (!collision) camera.setCameraPos(newCameraPos);
}

bool Wood::checkMonsterCollision(const glm::vec3& cameraPosition) {
    const float FIELD_OF_VIEW_ANGLE = 90.0f; // 90-degree field of view

    for (Monster& monster : monsters) {
        if (monster.checkCollisionWithPlayer(cameraPosition, MONSTER_RADIUS, FIELD_OF_VIEW_ANGLE)) {
            return true;
        }
    }
    return false;
}

void Wood::processMonsterAttack() {
    auto currentTime = std::chrono::steady_clock::now();

    if (std::chrono::duration_cast<std::chrono::seconds>(
            currentTime - lastAttackTime).count() >= 3) {

        playerLives--;
        lastAttackTime = currentTime;
        isBeingAttacked = true;

        glm::vec3 currentPos = camera.getNewCameraPosition(keys);
        glm::vec3 frontVector = camera.getFrontVector();
        camera.setCameraPos(currentPos - frontVector * attackPushbackDistance);
    }
}

void draw_heart(float x, float y, float size) {
    glColor3f(1.0, 0.0, 0.0); // Set color to red

    float radius = size / 5.0f;

    // Left circle
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x - radius, y);
    for (float angle = 0; angle <= 2 * M_PI; angle += 0.01) {
        float dx = radius * cos(angle);
        float dy = radius * sin(angle);
        glVertex2f(x - radius + dx, y + dy);
    }
    glEnd();

    // Right circle
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x + radius, y);
    for (float angle = 0; angle <= 2 * M_PI; angle += 0.01) {
        float dx = radius * cos(angle);
        float dy = radius * sin(angle);
        glVertex2f(x + radius + dx, y + dy);
    }
    glEnd();

    // Draw the bottom triangle part
    glBegin(GL_TRIANGLES);
    glVertex2f(x - 2.12f * radius, y);
    glVertex2f(x + 2.12f * radius, y);
    glVertex2f(x, y - 2.8f * radius);
    glEnd();
}

void Wood::renderHearts() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1); // Set up orthographic projection

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glUseProgram(0);
    glDisable(GL_DEPTH_TEST);

    float heartSize = 0.045f;
    float padding = 0.03f;

    for (int i = 0; i < playerLives; i++) {
        float x = 0.9f - (heartSize + padding) * i; // Calculate x position
        float y = 0.9f; // Fixed y position

        draw_heart(x, y, heartSize); // Draw the heart at the calculated position
    }

    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glUseProgram(shaderProgram);
}

void Wood::renderAttackOverlay() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glUseProgram(0);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(1.0f, 0.0f, 0.0f, 0.5f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(1, 0);
    glVertex2f(1, 1);
    glVertex2f(0, 1);
    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glUseProgram(shaderProgram);
}

void Wood::renderPickupPrompt(const std::string& text) {
    // Store current matrix states
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    // Set up orthographic projection
    int width = glutGet(GLUT_WINDOW_WIDTH);
    int height = glutGet(GLUT_WINDOW_HEIGHT);
    glOrtho(0, width, 0, height, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Disable current shader
    glUseProgram(0);

    // Set rendering color with transparency
    glColor4f(1.0f, 1.0f, 1.0f, 0.8f);

    // Enable blending for semi-transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Calculate text width using stroke font
    float totalWidth = 0;
    for (char c : text) totalWidth += glutStrokeWidth(GLUT_STROKE_ROMAN, c);

    // Position text at bottom center
    float scaleFactor = 1.2f;  // Adjust for desired size
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

    glDisable(GL_BLEND);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glUseProgram(shaderProgram);
}

void Wood::renderCrosshair() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glUseProgram(0);
    glDisable(GL_DEPTH_TEST);

    // Set crosshair color (white)
    glColor3f(1.0f, 1.0f, 1.0f);

    // Draw crosshair lines
    float centerX = 0.5f;
    float centerY = 0.5f;
    float length = 0.02f;  // Length of crosshair lines

    // Horizontal line
    glBegin(GL_LINES);
    glVertex2f(centerX - length, centerY);
    glVertex2f(centerX + length, centerY);
    glEnd();

    // Vertical line
    glBegin(GL_LINES);
    glVertex2f(centerX, centerY - length);
    glVertex2f(centerX, centerY + length);
    glEnd();

    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glUseProgram(shaderProgram);
}

void Wood::onMouseClick(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && equippedSword) {
        processSwordAttack();
    }
}