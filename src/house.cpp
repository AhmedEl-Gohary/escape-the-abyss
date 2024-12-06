#include "house.h"

House::House(GLuint shaderProgram, glm::mat4 &projection) :
    Environment(shaderProgram, projection), isNearCollectible(false),
    isShowingPickupPrompt(false){}

void House::generateFloor() {
    floorModel.loadModel("floor");

    const float floorSpacing = 2.0f;
    for (float x = MIN_X; x <= MAX_X; x += floorSpacing) {
        for (float z = MIN_Z; z <= MAX_Z; z += floorSpacing) {
            glm::mat4 floorTransform = glm::mat4(1.0f);
            glm::vec3 floorPos = glm::vec3(x, -5, z);
            floorTransform = glm::translate(floorTransform, floorPos);
            floorTransform = glm::rotate(floorTransform, glm::radians(90.0f), glm::vec3 (0.0f, 0.0f, 1.0f));
            floorTransform = glm::scale(floorTransform, glm::vec3(2.0f, 2.0f, 2.0f));
            floorTransformations.push_back(floorTransform);
        }
    }
}

void House::generateWalls() {
    wallModel.loadModel("wall");

    // wall 1
    glm::vec3 lower1 (MIN_X - WALL_DEPTH, -5.0f, MIN_Z - WALL_DEPTH);
    glm::vec3 upper1 (MAX_X + WALL_DEPTH, 5.0f, MIN_Z + WALL_DEPTH);

    Wall wall1 (lower1, upper1);
    walls.push_back(wall1);

    // wall 2
    glm::vec3 lower2 (MIN_X - WALL_DEPTH, -5.0f, MAX_Z - WALL_DEPTH);
    glm::vec3 upper2 (MAX_X + WALL_DEPTH, 5.0f, MAX_Z + WALL_DEPTH);

    Wall wall2 (lower2, upper2);
    walls.push_back(wall2);

    // wall 3
    glm::vec3 lower3 (MIN_X - WALL_DEPTH, -5.0f, MIN_Z - WALL_DEPTH);
    glm::vec3 upper3 (MIN_X + WALL_DEPTH, 5.0f, MAX_Z + WALL_DEPTH);

    Wall wall3 (lower3, upper3);
    walls.push_back(wall3);

    // wall 4
    glm::vec3 lower4 (MAX_X - WALL_DEPTH, -5.0f, MIN_Z - WALL_DEPTH);
    glm::vec3 upper4 (MAX_X + WALL_DEPTH, 5.0f, MAX_Z + WALL_DEPTH);

    Wall wall4 (lower4, upper4);
    walls.push_back(wall4);
}

void House::renderFloor() {
    for (const auto& transform : floorTransformations) {
        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(transform));
        floorModel.draw();
    }
}

void House::renderWall(const Wall& wall) {
    glm::mat4 wallTransform(1.0f); // Initialize to identity matrix

    // Compute the wall's dimensions
    float width = std::abs(wall.upperPoint.x - wall.lowerPoint.x);
    float height = std::abs(wall.upperPoint.y - wall.lowerPoint.y);
    float depth = std::abs(wall.upperPoint.z - wall.lowerPoint.z);

    // Compute the wall's center position
    glm::vec3 wallCenter = (wall.lowerPoint + wall.upperPoint) * 0.5f;

    // Determine the orientation of the wall (horizontal or vertical)
    bool isHorizontal = width > depth; // If width > depth, wall is horizontal along the X-axis

    // Set up the transformations
    wallTransform = glm::translate(wallTransform, wallCenter); // Translate to center position

    // Apply rotation if the wall is vertical (aligned along the Z-axis)
    if (!isHorizontal) {
        wallTransform = glm::rotate(wallTransform, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        std::swap(width, depth); // Swap width and depth for scaling
    }

    // Scale the wall to its dimensions
    wallTransform = glm::scale(wallTransform, glm::vec3(width, height, depth));

    // Pass the transformation to the shader
    GLuint wallModelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(wallModelLoc, 1, GL_FALSE, glm::value_ptr(wallTransform));

    // Render the wall model
    wallModel.draw();
}


void House::renderWalls() {
    for (auto wall : walls) {
        renderWall(wall);
    }
}

void House::init() {
    generateFloor();
    generateWalls();
}

void House::renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgram);

    // Define the projection matrix
    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
                                            (float)WIDTH / (float)HEIGHT,
                                            0.1f, 1000.0f);

    // Apply the camera view
    glm::mat4 view = camera.applyView();

    // Set the projection and view matrices in the shader
    GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE,
                       glm::value_ptr(projection));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // Get and set the camera position
    glm::vec3 cameraPos = camera.getCameraPos(); // Use getCameraPos() instead of getNewCameraPosition
    GLuint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
    glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPos));

    // Render scene elements

    renderFloor();
    renderWalls();

    if (isShowingPickupPrompt) {
//        renderPickupPrompt("Press E to pickup");
    }

    glutSwapBuffers();
}

void House::updateScene() {
    processKeyboard();
    glutPostRedisplay();
}

void House::processKeyboard() {
    glm::vec3 newCameraPos = camera.getNewCameraPosition(keys);
    if (keys[27]) exit(0);
    bool collidesWithWall = false;
    for (auto wall : walls){
        collidesWithWall |= wall.checkCollision(newCameraPos, COLLISION_RADIUS);
    }
    if (!collidesWithWall) {
        camera.setCameraPos(newCameraPos);
    }
}

void House::onMouseClick(int button, int state, int x, int y) {

}