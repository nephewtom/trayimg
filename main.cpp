#include "raylib.h"
#include "raymath.h" // Needed for matrix and quaternion operations
#include "rlgl.h"

#include <stdio.h>
#include <math.h>

int main(void)
{
    printf("Hello, World!\n");

    // Initialization
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "trayimg - Example");

    // Define Cube transform
    Transform cubeTransform = {
        .translation = {0.5f, 0.5f, 0.5f }, // Starting position with Y at CUBE_HALF_SIZE
        .rotation = QuaternionIdentity(),
        .scale = {1.0f, 1.0f, 1.0f }
    };

    
    // Camera setup with additional orbit parameters
    Camera3D camera;
    Vector3 initialCameraOffset = { 5.0f, 5.0f, 2.0f };
    camera.position = (Vector3) { 
        cubeTransform.translation.x + initialCameraOffset.x,
        initialCameraOffset.y,
        cubeTransform.translation.z + initialCameraOffset.z 
    };
    camera.target = (Vector3) { cubeTransform.translation.x, 0.0f, cubeTransform.translation.z }; // Look at ground level
    camera.up = (Vector3) { 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Camera orbit parameters
    Vector3 cameraOffset = { 
        camera.position.x - camera.target.x,
        camera.position.y - camera.target.y,
        camera.position.z - camera.target.z
    };
    float cameraDistance = sqrtf(cameraOffset.x * cameraOffset.x + 
                               cameraOffset.y * cameraOffset.y + 
                               cameraOffset.z * cameraOffset.z);
    const float MIN_CAMERA_DISTANCE = 2.0f;  // Minimum zoom distance
    const float MAX_CAMERA_DISTANCE = 15.0f;  // Maximum zoom distance
    const float ZOOM_SPEED = 0.5f;           // Zoom sensitivity
    float cameraAngleX = atan2f(cameraOffset.x, cameraOffset.z); // Initial X angle (around Y axis)
    float cameraAngleY = asinf(cameraOffset.y / cameraDistance); // Initial Y angle (around X axis)
    Vector2 previousMousePosition = { 0.0f, 0.0f };

    // Define axis parameters
    const float axisLength = 2.0f;  // Length of each axis
    const float coneLength = 0.3f;  // Length of the cone part
    const float coneRadius = 0.1f;  // Radius of the cone base
    const float lineRadius = 0.02f; // Radius for the axis lines

    // Animation state
    bool isAnimating = false;
    float animationTime = 0.0f;
    const float ANIMATION_WALK_DURATION = 0.5f;
    Quaternion currentRotation = QuaternionIdentity();
    Quaternion targetRotation = QuaternionIdentity();
    Vector3 currentPosition = {0.5f, 0.5f, 0.5f};
    Vector3 targetPosition = {-0.5f, 0.5f, 0.5f};
    
    // Movement queue
    bool hasQueuedMovement = false;
    int queuedKey = 0;  // Store the last pressed key

    // Initial cube position
    cubeTransform.translation.y = 0.5f;

    SetTargetFPS(60);

    while (!WindowShouldClose()) { // Main game loop
        float deltaTime = GetFrameTime();

        // Update camera position when right mouse button is held
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            Vector2 mousePositionDelta = { 0.0f, 0.0f };
            Vector2 mousePosition = GetMousePosition();
            
            if (previousMousePosition.x != 0.0f || previousMousePosition.y != 0.0f) {
                mousePositionDelta.x = mousePosition.x - previousMousePosition.x;
                mousePositionDelta.y = mousePosition.y - previousMousePosition.y;
            }
            
            // Update camera angles based on mouse movement
            cameraAngleX -= mousePositionDelta.x * 0.003f;
            cameraAngleY = fmax(fmin(cameraAngleY + mousePositionDelta.y * 0.003f, PI/2.0f), -PI/2.0f);
            
            previousMousePosition = mousePosition;
        } else {
            previousMousePosition = (Vector2){ 0.0f, 0.0f };
        }

        // Handle mouse wheel for zoom
        float mouseWheel = GetMouseWheelMove();
        if (mouseWheel != 0) {
            cameraDistance -= mouseWheel * ZOOM_SPEED;
            cameraDistance = fmax(MIN_CAMERA_DISTANCE, fmin(cameraDistance, MAX_CAMERA_DISTANCE));
        }

        // Update camera position based on angles and cube position
        camera.target = (Vector3){ cubeTransform.translation.x, 0.0f, cubeTransform.translation.z };  // Camera looks at ground level
        camera.position.x = camera.target.x + cameraDistance * cosf(cameraAngleY) * sinf(cameraAngleX);
        camera.position.y = camera.target.y + cameraDistance * sinf(cameraAngleY);
        camera.position.z = camera.target.z + cameraDistance * cosf(cameraAngleY) * cosf(cameraAngleX);

        // Handle W key press to start animation
        if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_A) || IsKeyPressed(KEY_D)) {
            int pressedKey = IsKeyPressed(KEY_W) ? KEY_W :
                           IsKeyPressed(KEY_S) ? KEY_S :
                           IsKeyPressed(KEY_A) ? KEY_A : KEY_D;
                           
            if (!isAnimating) {
                // Start animation immediately if not already animating
                isAnimating = true;
                animationTime = 0.0f;
                
                // Process the movement
                if (pressedKey == KEY_W) {
                    targetPosition = currentPosition + (Vector3){-1.0f, 0.0f, 0.0f};
                    currentRotation = cubeTransform.rotation;
                    targetRotation = QuaternionFromAxisAngle((Vector3){0, 0, 1}, 90.0f * DEG2RAD);
                }
                else if (pressedKey == KEY_S) {
                    targetPosition = currentPosition + (Vector3){1.0f, 0.0f, 0.0f};
                    currentRotation = cubeTransform.rotation;
                    targetRotation = QuaternionFromAxisAngle((Vector3){0, 0, 1}, -90.0f * DEG2RAD);
                }
                else if (pressedKey == KEY_A) {
                    targetPosition = currentPosition + (Vector3){0.0f, 0.0f, 1.0f};
                    currentRotation = cubeTransform.rotation;
                    targetRotation = QuaternionFromAxisAngle((Vector3){1, 0, 0}, 90.0f * DEG2RAD);
                }
                else if (pressedKey == KEY_D) {
                    targetPosition = currentPosition + (Vector3){0.0f, 0.0f, -1.0f};
                    currentRotation = cubeTransform.rotation;
                    targetRotation = QuaternionFromAxisAngle((Vector3){1, 0, 0}, -90.0f * DEG2RAD);
                }
            } else {
                // Queue the movement for when current animation ends
                hasQueuedMovement = true;
                queuedKey = pressedKey;
            }
        }

        // Update animation
        if (isAnimating) {
            animationTime += deltaTime;
            float t = animationTime / ANIMATION_WALK_DURATION;
            
            if (t >= 1.0f) {
                // Animation complete
                t = 1.0f;
                isAnimating = false;
                currentPosition = targetPosition;
                cubeTransform.rotation = QuaternionIdentity();
                
                // If there's a queued movement, start it immediately
                if (hasQueuedMovement) {
                    isAnimating = true;
                    animationTime = 0.0f;
                    hasQueuedMovement = false;
                    
                    // Process the queued movement
                    if (queuedKey == KEY_W) {
                        targetPosition = currentPosition + (Vector3){-1.0f, 0.0f, 0.0f};
                        currentRotation = cubeTransform.rotation;
                        targetRotation = QuaternionFromAxisAngle((Vector3){0, 0, 1}, 90.0f * DEG2RAD);
                    }
                    else if (queuedKey == KEY_S) {
                        targetPosition = currentPosition + (Vector3){1.0f, 0.0f, 0.0f};
                        currentRotation = cubeTransform.rotation;
                        targetRotation = QuaternionFromAxisAngle((Vector3){0, 0, 1}, -90.0f * DEG2RAD);
                    }
                    else if (queuedKey == KEY_A) {
                        targetPosition = currentPosition + (Vector3){0.0f, 0.0f, 1.0f};
                        currentRotation = cubeTransform.rotation;
                        targetRotation = QuaternionFromAxisAngle((Vector3){1, 0, 0}, 90.0f * DEG2RAD);
                    }
                    else if (queuedKey == KEY_D) {
                        targetPosition = currentPosition + (Vector3){0.0f, 0.0f, -1.0f};
                        currentRotation = cubeTransform.rotation;
                        targetRotation = QuaternionFromAxisAngle((Vector3){1, 0, 0}, -90.0f * DEG2RAD);
                    }
                }
            } else {
                // Interpolate rotation and position
                float smoothT = t * t * (3.0f - 2.0f * t); // Smooth interpolation
                cubeTransform.rotation = QuaternionSlerp(currentRotation, targetRotation, smoothT);
                Vector3 newPos = Vector3Lerp(currentPosition, targetPosition, smoothT);
                cubeTransform.translation = newPos;
            }
        }

        // Convert Transform to Matrix
        Matrix rotationMatrix = QuaternionToMatrix(cubeTransform.rotation);
        Matrix scaleMatrix = MatrixScale(cubeTransform.scale.x,
                                         cubeTransform.scale.y,
                                         cubeTransform.scale.z);
        Matrix translationMatrix = MatrixTranslate(cubeTransform.translation.x,
                                                   cubeTransform.translation.y,
                                                   cubeTransform.translation.z);
        
        // First apply scale, then rotation, then translation
        Matrix transformMatrix = MatrixMultiply(MatrixMultiply(scaleMatrix, rotationMatrix), translationMatrix);
        
        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode3D(camera);

        // Draw grid first
        DrawGrid(10, 1.0f);

        // Draw the cube
        DrawCube({-3.0f, 0.0f, 2.0f}, 1.0f, 1.0f, 1.0f, BLUE);
        
        // Apply transformation matrix before drawing the cube
        rlPushMatrix();
        rlMultMatrixf(MatrixToFloat(transformMatrix));
        DrawCube(Vector3Zero(), 1.0f, 1.0f, 1.0f, RED);
        DrawCubeWires(Vector3Zero(), 1.0f, 1.0f, 1.0f, GREEN);
        rlPopMatrix();

        // Draw coordinate axes with cones
        // X axis (red)
        DrawCylinderEx(Vector3Zero(), (Vector3){axisLength, 0, 0}, lineRadius, lineRadius, 8, RED);
        DrawCylinderEx((Vector3){axisLength, 0, 0}, (Vector3){axisLength + coneLength, 0, 0}, 
                       coneRadius, 0.0f, 8, RED);

        // Y axis (green)
        DrawCylinderEx(Vector3Zero(), (Vector3){0, axisLength, 0}, lineRadius, lineRadius, 8, GREEN);
        DrawCylinderEx((Vector3){0, axisLength, 0}, (Vector3){0, axisLength + coneLength, 0}, 
                       coneRadius, 0.0f, 8, GREEN);

        // Z axis (blue)
        DrawCylinderEx(Vector3Zero(), (Vector3){0, 0, axisLength}, lineRadius, lineRadius, 8, BLUE);
        DrawCylinderEx((Vector3){0, 0, axisLength}, (Vector3){0, 0, axisLength + coneLength}, 
                       coneRadius, 0.0f, 8, BLUE);

        EndMode3D();

        DrawText("Use WASD to roll cube", 10, 10, 20, DARKGRAY);
        DrawText("Hold right mouse button to orbit camera", 10, 40, 20, DARKGRAY);
        DrawText(TextFormat("Current Position: (%.1f, %.1f, %.1f)", 
                            cubeTransform.translation.x, 
                            cubeTransform.translation.y, 
                            cubeTransform.translation.z), 10, 70, 20, DARKGRAY);
        DrawFPS(10, 100);

        EndDrawing();
    }

    // Cleanup
    CloseWindow();
    return 0;
}
