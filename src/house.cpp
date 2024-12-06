#include "house.h"

House::House(GLuint shaderProgram, glm::mat4 &projection) :
    Environment(shaderProgram, projection), isNearCollectible(false),
    isShowingPickupPrompt(false){}

bool House::checkDoorCollision(const glm::vec3& playerPosition, float playerRadius) {
    // Extract door's position from the transformation matrix
    glm::vec3 doorPos = glm::vec3(doorTransformation[3]);

    // Get the scaling factors from the transformation matrix
    float scaleX = glm::length(glm::vec3(doorTransformation[0]));
    float scaleY = glm::length(glm::vec3(doorTransformation[1]));
    float scaleZ = glm::length(glm::vec3(doorTransformation[2]));

    // Define the door's approximate bounding box
    float halfWidth = 0.5f * scaleX;
    float halfHeight = 0.5f * scaleY;
    float halfDepth = 0.5f * scaleZ;

    // Calculate the door's min and max points
    glm::vec3 doorMin = doorPos - glm::vec3(halfWidth, halfHeight, halfDepth);
    glm::vec3 doorMax = doorPos + glm::vec3(halfWidth, halfHeight, halfDepth);

    // Perform AABB (Axis-Aligned Bounding Box) collision check
    bool collisionX = (playerPosition.x + playerRadius > doorMin.x) &&
                      (playerPosition.x - playerRadius < doorMax.x);
    bool collisionY = (playerPosition.y + playerRadius > doorMin.y) &&
                      (playerPosition.y - playerRadius < doorMax.y);
    bool collisionZ = (playerPosition.z + playerRadius > doorMin.z) &&
                      (playerPosition.z - playerRadius < doorMax.z);

    // Return true if collision occurs in all axes
    return collisionX && collisionY && collisionZ;
}

bool House::checkFlashlightCollision(const glm::vec3 &cameraPosition) {
    if (isFlashlightEquipped) return false;

    float distance = glm::length(cameraPosition - glm::vec3(flashlightTransformation[3]));
    return distance < COLLECTIBLE_PICKUP_RANGE;
}

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

void House::generateDoor() {
    doorModel.loadModel("door");

    glm::vec3 doorPosition (MAX_X - 3 * WALL_DEPTH, -2.5f, 0.0f);
    glm::mat4 doorTransform (1.0f);
    doorTransform = glm::translate(doorTransform, doorPosition);
    doorTransform = glm::scale(doorTransform, glm::vec3 (1.0f, 2.0f, 2.0f));
    doorTransformation = doorTransform;
}

void House::generateKey() {
    keyModel.loadModel("key");

    glm::vec3 keyPosition (-MAX_X + 2, -2, 0.0f);
    glm::mat4 keyTransform (1.0f);
    keyTransform = glm::translate(keyTransform, keyPosition);
    keyTransform = glm::rotate(keyTransform, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    keyTransform = glm::rotate(keyTransform, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    keyTransform = glm::rotate(keyTransform, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
//    keyTransform = glm::scale(keyTransform, glm::vec3 (0.5f, 0.5f, 0.5f));
    keyTransformation = keyTransform;
}

void House::renderPickupPrompt(const std::string& text) {
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

void House::renderWhiteboard() {
    if (!whiteboard.isVisible) return;
    renderPickupPrompt("3 + 3 = ?");
}

void House::generateLightbulb() {
    lightbulbModel.loadModel("lightbulb");
    glm::vec3 lightbulbPos (MAX_X - 3 * WALL_DEPTH, 4.0f, 0.0f);
    glm::mat4 lightbulbTransform (1.0f);
    lightbulbTransform = glm::translate(lightbulbTransform, lightbulbPos);
    lightbulbTransform = glm::rotate(lightbulbTransform, glm::radians(90.0f),
                                     glm::vec3 (1.0f, 0.0f, 0.0f));
    lightbulbTransform = glm::rotate(lightbulbTransform, glm::radians(90.0f),
                                     glm::vec3 (0.0f, 0.0f, 1.0f));
    lightbulbTransform = glm::scale(lightbulbTransform, glm::vec3 (0.2f, 0.2f, 0.2f));
    lightbulbTransformation = lightbulbTransform;
}

void House::generateFlashlight() {
    flashlightModel.loadModel("flashlight");
    glm::vec3 flashlightPos (MAX_X - 10 * WALL_DEPTH, -3.0f, MAX_Z - 10 * WALL_DEPTH);
    glm::mat4 flashlightTransform (1.0f);
    flashlightTransform = glm::translate(flashlightTransform, flashlightPos);
    flashlightTransform = glm::scale(flashlightTransform, glm::vec3 (0.2f, 0.2f, 0.2f));
    flashlightTransformation = flashlightTransform;
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

void House::renderDoor() {
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE,
                       glm::value_ptr(doorTransformation));
    doorModel.draw();
}

void House::renderKey() {
    if (isKeyEquipped) return;
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE,
                       glm::value_ptr(keyTransformation));
    keyModel.draw();
}

bool House::checkKeyCollision(const glm::vec3& cameraPosition) {
    if (isKeyEquipped) return false;
    float distance = glm::length(cameraPosition - glm::vec3(keyTransformation[3]));
    return distance < abs(camera.getCameraHeight()) + COLLECTIBLE_PICKUP_RANGE;
}

void House::renderLightbulb() {
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE,
                       glm::value_ptr(lightbulbTransformation));
    lightbulbModel.draw();
}

void House::renderFlashlight() {
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE,
                       glm::value_ptr(flashlightTransformation));
    flashlightModel.draw();
}

void House::renderEquippedFlashlight() {
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");

    // Get camera properties
    glm::vec3 cameraPos = camera.getCameraPos();
    glm::vec3 cameraFront = camera.getFrontVector();
    glm::vec3 cameraUp = camera.getCameraUp();

    // Calculate flashlight position (slightly below the camera position)
    glm::vec3 flashlightPos = cameraPos - glm::vec3(1.0f, 2.0f, 0.0f);

    // Create transformation matrix for flashlight
    glm::mat4 flashlightTransform = glm::mat4(1.0f);
    flashlightTransform = glm::translate(flashlightTransform, flashlightPos);

    // Calculate the flashlight's orientation to align with the camera's direction
    glm::vec3 flashlightDirection = glm::normalize(cameraFront);
    glm::vec3 right = glm::normalize(glm::cross(cameraUp, flashlightDirection));
    glm::vec3 adjustedUp = glm::normalize(glm::cross(flashlightDirection, right));

    flashlightTransform[0] = glm::vec4(right, 0.0f);
    flashlightTransform[1] = glm::vec4(adjustedUp, 0.0f);
    flashlightTransform[2] = glm::vec4(flashlightDirection, 0.0f);

    // Send the transformation matrix to the shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(flashlightTransform));
    flashlightTransformation = flashlightTransform;
    // Render the flashlight model
    flashlightModel.draw();
}

void House::toggleFlashlight() {
    isFlashlightOn = !isFlashlightOn;
}

void House::init() {
    generateFloor();
    generateWalls();
    generateDoor();
    generateKey();
    generateLightbulb();
    generateFlashlight();
}

float generateFlickerIntensity() {
    // Simple random flicker
    float randomFlicker = static_cast<float>(rand()) / RAND_MAX;

    // You can adjust these parameters to control flicker behavior
    float flickerFrequency = 10.0f;  // How often it flickers
    float flickerStrength = 0.5f;   // Intensity of the flicker

    return sin(randomFlicker * flickerFrequency) * flickerStrength;
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

    // moon
    glUniform1f(glGetUniformLocation(shaderProgram, "isMoonOn"), false);

    // lightbulb
    glUniform1i(glGetUniformLocation(shaderProgram, "isLightBulbOn"), true);
    glUniform3f(glGetUniformLocation(shaderProgram, "lightBulbColor"),
                1.0f, 1.0f, 1.0f);
    glUniform3fv(glGetUniformLocation(shaderProgram, "lightBulbPos"), 1,
                glm::value_ptr(glm::vec3(lightbulbTransformation[3]) +
                glm::vec3(1.0f, 0.0f, 0.0f)));
    glUniform1f(glGetUniformLocation(shaderProgram, "lightBulbFlicker"),
                generateFlickerIntensity() + 0.6);
    glUniform1f(glGetUniformLocation(shaderProgram, "lightBulbAmbientStrength"),
                4.0f);

    // flashlight
    glUniform1f(glGetUniformLocation(shaderProgram, "isFlashlightOn"), isFlashlightOn);
    glUniform3f(glGetUniformLocation(shaderProgram, "flashlightColor"), 1.0f, 1.0f, 1.0f);
    glUniform3fv(glGetUniformLocation(shaderProgram, "flashlightPos"), 1,
                 glm::value_ptr(glm::vec3(flashlightTransformation[3])));
    glUniform3fv(glGetUniformLocation(shaderProgram, "flashlightDirection"), 1,
                 glm::value_ptr(glm::normalize(camera.getFrontVector())));
    glUniform1f(glGetUniformLocation(shaderProgram, "flashlightCutOff"),
                cos(glm::radians(12.5f)));
    glUniform1f(glGetUniformLocation(shaderProgram, "flashlightOuterCutOff"),
                cos(glm::radians(17.5f)));

    // Render scene elements
    renderFloor();
    renderWalls();
    renderDoor();
    renderWhiteboard();
    renderKey();
    renderLightbulb();

    if (isFlashlightEquipped) {
        renderEquippedFlashlight();
    } else {
        renderFlashlight();
    }

    if (checkFlashlightCollision(camera.getCameraPos())) {
        renderPickupPrompt("Press E to pickup");
    }
    if (isShowingPickupPrompt && !whiteboard.isVisible) {
        renderPickupPrompt("Press E to pickup");
    }

    if (whiteboard.solved){
        isKeyEquipped = true;
    }

    glutSwapBuffers();
}

void House::updateScene() {
    processKeyboard();
    if (checkDoorCollision(camera.getCameraPos(), COLLISION_RADIUS * 0.8f)){
        if (isKeyEquipped) {
            isRunning = false;
            return;
        }
    }
    glutPostRedisplay();
}

void House::processKeyboard() {
    glm::vec3 newCameraPos = camera.getNewCameraPosition(keys);
    if (keys[27]) exit(0);

    // Whiteboard toggle
    if (whiteboard.isVisible) {
        if (keys['e']){
            whiteboard.isVisible = false;
            keys['e'] = false;
        }
        if (keys['6']) {
            whiteboard.isVisible = false;
            whiteboard.solved = true;
        }
    }

    if (whiteboard.isVisible) return;

    if (isShowingPickupPrompt && keys['e']){
        isShowingPickupPrompt = false;
        whiteboard.isVisible = true;
        keys['e'] = false;
    }

    if (!isFlashlightEquipped && checkFlashlightCollision(camera.getCameraPos()) && keys['e']) {
        isFlashlightEquipped = true;
        keys['e'] = false;
    }

    bool collidesWithWall = false;
    for (auto wall : walls){
        collidesWithWall |= wall.checkCollision(newCameraPos, COLLISION_RADIUS);
    }
    if (!collidesWithWall) {
        camera.setCameraPos(newCameraPos);
    }
    isShowingPickupPrompt = checkKeyCollision(newCameraPos);
}

void House::onMouseClick(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && isFlashlightEquipped) {
        toggleFlashlight();
    }
}