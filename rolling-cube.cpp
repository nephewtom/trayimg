#include "raylib.h"
#include "raymath.h" // Needed for matrix and quaternion operations
#include "rlgl.h"

#include "imgui.h"
#include "rlImGui.h"

#include <math.h>
#include <stdarg.h>

struct Cube {
    Vector3 position;
    Vector3 targetPosition;
    Vector3 direction;

    Vector3 rotationAxis;
    Vector3 rotationOrigin;
    float rotationAngle;
    float targetAngle;
    Matrix transform;
    
    bool isMoving;
    float animationProgress;
    float animationSpeed;
};
Cube cube;

Vector3 cubeInitPos = {0.5f, 0.5f, 0.5f};

void initCube() {
    cube = {
        .position = cubeInitPos,
        .targetPosition = cubeInitPos,
        .direction = {-1.0f, 0.0f, 0.0f},

        .rotationAxis = {0.0f, 0.0f, 1.0f},
        .rotationOrigin = {0.0f, 0.0f, 1.0f},
        .rotationAngle = 0.0f,
        .targetAngle = 0.0f,
        .transform = MatrixIdentity(),
    
        .isMoving = false,
        .animationProgress = 0.0f,
        .animationSpeed = 2.0f
    };
}

struct CubeCamera {
    Camera c3d;
    float distance; // distance to target
    Vector3 direction;
    float angleX;
    float angleY;
};
CubeCamera camera;

void initCamera() {
    camera.c3d = {
		.position = (Vector3){ 6.0f, 6.0f, 2.0f },
		.target = cubeInitPos,
		.up = (Vector3) { 0.0f, 1.0f, 0.0f },
		.fovy = 45.0f,
		.projection = CAMERA_PERSPECTIVE,
	};

	// Camera orbit parameters
    Vector3 cameraOffset = { 
		camera.c3d.position.x - camera.c3d.target.x,
		camera.c3d.position.y - camera.c3d.target.y,
		camera.c3d.position.z - camera.c3d.target.z
	};
	camera.distance = sqrtf(cameraOffset.x * cameraOffset.x +
							cameraOffset.y * cameraOffset.y +
							cameraOffset.z * cameraOffset.z);

	camera.angleX = atan2f(cameraOffset.x, cameraOffset.z);
	camera.angleY = asinf(cameraOffset.y / camera.distance);
}

void updateCamera()  {
    // Update camera position based on angles and cube position
    camera.c3d.target = cubeInitPos;  // Camera looks at ground level
    camera.c3d.position.x = camera.c3d.target.x + camera.distance * cosf(camera.angleY) * sinf(camera.angleX);
    camera.c3d.position.y = camera.c3d.target.y + camera.distance * sinf(camera.angleY);
    camera.c3d.position.z = camera.c3d.target.z + camera.distance * cosf(camera.angleY) * cosf(camera.angleX);
}

const float MIN_CAMERA_DISTANCE = 5.0f;  // Minimum zoom distance
const float MAX_CAMERA_DISTANCE = 20.0f;  // Maximum zoom distance
const float ZOOM_SPEED = 1.0f;           // Zoom sensitivity
Vector2 previousMousePosition = { 0.0f, 0.0f };
void handleMouseButton() {
    // Update camera position when right mouse button is held
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        Vector2 mousePositionDelta = { 0.0f, 0.0f };
        Vector2 mousePosition = GetMousePosition();
            
        if (previousMousePosition.x != 0.0f || previousMousePosition.y != 0.0f) {
            mousePositionDelta.x = mousePosition.x - previousMousePosition.x;
            mousePositionDelta.y = mousePosition.y - previousMousePosition.y;
        }
            
        // Update camera angles based on mouse movement
        camera.angleX -= mousePositionDelta.x * 0.003f;
        camera.angleY = fmax(fmin(camera.angleY + mousePositionDelta.y * 0.003f, PI/2.0f), -PI/2.0f);

        previousMousePosition = mousePosition;
			
        // Calculate camera direction vector (normalized)
        Vector3 cameraDirection = {
            camera.c3d.target.x - camera.c3d.position.x,
            0.0f,
            camera.c3d.target.z - camera.c3d.position.z
        };
        float length = sqrtf(cameraDirection.x * cameraDirection.x + cameraDirection.z * cameraDirection.z);
        cameraDirection.x /= length;
        cameraDirection.z /= length;

        // Calculate dot products with world axes
        float dotX = fabsf(cameraDirection.x);  // Dot product with (1,0,0)
        float dotZ = fabsf(cameraDirection.z);  // Dot product with (0,0,1)

        // Determine movement direction based on camera orientation
        cube.direction = { 0.0f, 0.0f, 0.0f };
        if (dotX > dotZ) {
            // Camera is more aligned with X axis
            cube.direction.x = (cameraDirection.x > 0) ? 1.0f : -1.0f;
        } else {
            // Camera is more aligned with Z axis
            cube.direction.z = (cameraDirection.z > 0) ? 1.0f : -1.0f;
        }
			
    } else {
        previousMousePosition = (Vector2){ 0.0f, 0.0f };
    }
}

void handleMouseWheel() {
    // Handle mouse wheel for zoom
    float mouseWheel = GetMouseWheelMove();
    if (mouseWheel != 0) {
        camera.distance -= mouseWheel * ZOOM_SPEED;
        camera.distance = fmax(MIN_CAMERA_DISTANCE, fmin(camera.distance, MAX_CAMERA_DISTANCE));
    }
}

// Define axis parameters
const float axisLength = 2.0f;  // Length of each axis
const float coneLength = 0.3f;  // Length of the cone part
const float coneRadius = 0.1f;  // Radius of the cone base
const float lineRadius = 0.02f; // Radius for the axis lines    
void drawAxis() {
    // Draw coordinate axes with cones
    // X axis (red)
    DrawCylinderEx(Vector3Zero(), (Vector3){axisLength, 0, 0}, lineRadius, lineRadius, 8, RED);
    DrawCylinderEx((Vector3){axisLength, 0, 0}, (Vector3){axisLength + coneLength, 0, 0}, coneRadius, 0.0f, 8, RED);

    // Y axis (green)
    DrawCylinderEx(Vector3Zero(), (Vector3){0, axisLength, 0}, lineRadius, lineRadius, 8, GREEN);
    DrawCylinderEx((Vector3){0, axisLength, 0}, (Vector3){0, axisLength + coneLength, 0}, coneRadius, 0.0f, 8, GREEN);

    // Z axis (blue)
    DrawCylinderEx(Vector3Zero(), (Vector3){0, 0, axisLength}, lineRadius, lineRadius, 8, BLUE);
    DrawCylinderEx((Vector3){0, 0, axisLength}, (Vector3){0, 0, axisLength + coneLength}, coneRadius, 0.0f, 8, BLUE);
    }

Vector2 fullHD = { 1920, 1080 };
float screenWidth = fullHD.x;
float screenHeight = fullHD.y;

int main()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "Cube!");
    SetWindowPosition(25, 50);

    SetTargetFPS(60);
    initCube();
    initCamera();
    
    rlImGuiSetup(true);

    float rectSize = 400.0f;

    while (!WindowShouldClose()) // Main game loop
    {
        float deltaTime = GetFrameTime();

        handleMouseButton();
        handleMouseWheel();
        updateCamera();

        int pressedKey =
            IsKeyPressed(KEY_W) ? KEY_W :
            IsKeyPressed(KEY_S) ? KEY_S :
            IsKeyPressed(KEY_A) ? KEY_A :
            IsKeyPressed(KEY_D) ? KEY_D : 0;

        if (pressedKey) {
            if (!cube.isMoving) {
                Vector3 direction = { 0.0f, 0.0f, 0.0f };
                
                // Process the movement
                if (pressedKey == KEY_W) {
                    direction.z = -1.0f;
                    cube.rotationAxis = (Vector3){-1.0f, 0.0f, 0.0f};
                    cube.rotationOrigin.x = cube.position.x;
                    cube.rotationOrigin.y = cube.position.y - 0.5f; // Bottom edge
                    cube.rotationOrigin.z = cube.position.z - 0.5f; // Front edge
                } else if (pressedKey == KEY_S) {
                    direction.z = 1.0f;
                    cube.rotationAxis = (Vector3){1.0f, 0.0f, 0.0f};
                    cube.rotationOrigin.x = cube.position.x;
                    cube.rotationOrigin.y = cube.position.y - 0.5f; // Bottom edge
                    cube.rotationOrigin.z = cube.position.z + 0.5f; // Back edge
                } else if (pressedKey == KEY_A) {
                    direction.x = -1.0f;
                    cube.rotationAxis = (Vector3){0.0f, 0.0f, 1.0f};
                    cube.rotationOrigin.x = cube.position.x - 0.5f; // Left edge
                    cube.rotationOrigin.y = cube.position.y - 0.5f; // Bottom edge
                    cube.rotationOrigin.z = cube.position.z;
                } else if (pressedKey == KEY_D) {
                    direction.x = 1.0f;
                    cube.rotationAxis = (Vector3){0.0f, 0.0f, -1.0f};
                    cube.rotationOrigin.x = cube.position.x + 0.5f; // Right edge
                    cube.rotationOrigin.y = cube.position.y - 0.5f; // Bottom edge
                    cube.rotationOrigin.z = cube.position.z;
                }
                cube.targetPosition.x = cube.position.x + direction.x;
                cube.targetPosition.y = cube.position.y + direction.y;
                cube.targetPosition.z = cube.position.z + direction.z;
                
                cube.rotationAngle = 0.0f;
                cube.targetAngle = PI/2.0f; // 90 degrees in radians
                cube.isMoving = true;
                cube.animationProgress = 0.0f;
            }
        }

        // Update animation
        if (cube.isMoving) {
            cube.animationProgress += deltaTime * cube.animationSpeed;

            // Use smooth easing for animation
            float t = cube.animationProgress;
            float smoothT = t * t * (3.0f - 2.0f * t); // Smoothstep formula
                    
            // Calculate current rotation angle
            cube.rotationAngle = cube.targetAngle * smoothT;
                    
            Matrix translateToOrigin = MatrixTranslate(-cube.rotationOrigin.x, -cube.rotationOrigin.y, -cube.rotationOrigin.z);
            Matrix rotation = MatrixRotate(cube.rotationAxis, cube.rotationAngle);
            Matrix translateBack = MatrixTranslate(cube.rotationOrigin.x, cube.rotationOrigin.y, cube.rotationOrigin.z);
                    
            // Combine matrices: first translate to rotation origin, then rotate, then translate back
            cube.transform = MatrixMultiply(translateToOrigin, rotation);
            cube.transform = MatrixMultiply(cube.transform, translateBack);

            if (cube.animationProgress >= 1.0f) {
                cube.position = cube.targetPosition;
                cube.isMoving = false;
                cube.animationProgress = 0.0f;
                cube.rotationAngle = 0.0f;
            }
        }

        
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera.c3d);

        DrawGrid(100, 1.0f);
        drawAxis();
        DrawCube({-3.0f, 0.0f, 2.0f}, 1.0f, 1.0f, 1.0f, BLUE);
        DrawCubeWires({-3.0f, 0.0f, 2.0f}, 1.0f, 1.0f, 1.0f, GREEN);

        DrawCubeWiresV(cube.position, (Vector3){1.0f, 1.0f, 1.0f}, BLUE);
        if (cube.isMoving) {
            rlPushMatrix();
            rlMultMatrixf(MatrixToFloat(cube.transform));
            DrawCube(cube.position, 1.0f, 1.0f, 1.0f, RED);
            DrawCubeWires(cube.position, 1.0f, 1.0f, 1.0f, GREEN);
            rlPopMatrix();
            DrawSphere(cube.rotationOrigin, 0.1f, YELLOW);
        } else {
            DrawCube(cube.position, 1.0f, 1.0f, 1.0f, RED);
            DrawCubeWires(cube.position, 1.0f, 1.0f, 1.0f, GREEN);
        }
        
        EndMode3D();

        rlImGuiBegin();
        ImGui::Begin("ImGui window");
        ImGui::SeparatorText("Rectangle");
        ImGui::DragFloat("rectSize", &rectSize, 1.0f, 100.0f, 400.0f);
        ImGui::End();
        rlImGuiEnd();

        DrawRectangle(1500, 600, rectSize, rectSize, GREEN);
        DrawText(TextFormat("Rectangle rectSize: %.0f", rectSize), 1500, 580, 20, BLACK);

        DrawText(TextFormat("cube.position: {%.2f, %.2f, %.2f}",
                            cube.position.x,
                            cube.position.y,
                            cube.position.z), 10, 10, 20, BLACK);

        DrawText(TextFormat("cube.rotationAxis: {%.2f, %.2f, %.2f}",
                            cube.rotationAxis.x,
                            cube.rotationAxis.y,
                            cube.rotationAxis.z), 10, 30, 20, BLACK);

        DrawText(TextFormat("cube.direction: {%.2f, %.2f, %.2f}",
                            cube.direction.x, cube.direction.y,
                            cube.direction.z), 10, 50, 20, BLACK);

        // DrawText(TextFormat("rotationAngle: %.0f", rotationAngle), 10, 70, 20, BLACK);

        EndDrawing();
    }
    rlImGuiShutdown();

    CloseWindow();
    return 0;
}

