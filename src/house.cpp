#include "house.h"

void House::generatePlayer() {
    camera.setCameraHeight(2.0f);
    playerModel.loadModel("spiderman");
    glm::mat4 playerTransform (1.0f);
    glm::vec3 playerPos (0, 0, 0);
    playerTransformation = playerTransform;
    playerPosition = playerPos;
}

void House::renderPlayer() {
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc,
                       1,
                       GL_FALSE,
                       glm::value_ptr(playerTransformation));
    playerModel.draw();
}

void House::init() {
    generatePlayer();
}

void House::updateScene() {
    // Calculate delta time
    static float last_frame_time = 0.0f;
    float current_frame = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float delta_time = current_frame - last_frame_time;
    last_frame_time = current_frame;
//    playerModel.updateAnimation(delta_time);
    processKeyboard();
    glutPostRedisplay();
}

void House::renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgram);

    glm::mat4 view = camera.applyView();
    GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");

    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    renderPlayer();
    glutSwapBuffers();
}

void House::processKeyboard() {
    glm::vec3 newCameraPos = camera.getNewCameraPosition(keys);
    if (keys[27]) exit(0);

    bool collision = false;

//    glm::vec3 treePos = glm::vec3(playerTransformation[3]);
//    if (checkCollision(newCameraPos, treePos)) {
//        collision = true;
//    }

    if (!collision) camera.setCameraPos(newCameraPos);
}