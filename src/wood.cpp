#include "wood.h"

void Wood::generateForest(int treeCount) {
    treeModel.loadModel("tree");
    srand(static_cast<unsigned int>(time(nullptr))); // Seed for randomness

    std::vector<glm::vec3> treePositions; // Store positions of trees
    float minDistance = TREE_RADIUS * 15.0f; // Increase this value for more spacing

    while (treePositions.size() < treeCount) {
        glm::mat4 treeTransform = glm::mat4(1.0f);
        float x = static_cast<float>(rand()) / (float) RAND_MAX * WOODS_SIZE - (WOODS_SIZE / 2);
        float z = static_cast<float>(rand()) / (float) RAND_MAX * WOODS_SIZE - (WOODS_SIZE / 2);
        glm::vec3 newTreePos = glm::vec3(x, 0, z);

        // Check if the new position is too close to existing trees
        bool tooClose = false;
        for (const auto& pos : treePositions) {
            if (glm::length(newTreePos - pos) < minDistance) {
                tooClose = true;
                break;
            }
        }

        // If the position is not too close, add it
        if (!tooClose) {
            treeTransform = glm::translate(treeTransform, newTreePos);
            treeTransform = glm::scale(treeTransform, glm::vec3(4, 4, 4));

            treeTransformations.push_back(treeTransform);
            treePositions.push_back(newTreePos); // Store the position
        }
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

bool Wood::checkCollision(const glm::vec3 &cameraPosition, const glm::vec3 &treePos) {
    float distance = glm::length(cameraPosition - treePos);
    return distance < abs(camera.getCameraHeight()) + TREE_RADIUS;
}

void Wood::init() {
    generateForest(TREE_COUNT);
}

void Wood::updateScene() {
    processKeyboard();
    glutPostRedisplay();
}

void Wood::renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgram);

    glm::mat4 view = camera.applyView();

    GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");

    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    renderForest();
    glutSwapBuffers();
}

void Wood::processKeyboard() {
    glm::vec3 newCameraPos = camera.getNewCameraPosition(keys);
    if (keys[27]) exit(0);

    bool collision = false;
    for (const auto& transform : treeTransformations) {
        glm::vec3 treePos = glm::vec3(transform[3]);
        if (checkCollision(newCameraPos, treePos)) {
            collision = true;
            break;
        }
    }

    if (!collision) camera.setCameraPos(newCameraPos);
}

