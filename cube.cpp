#include <stdio.h>

#include "raylib.h"
#include "raymath.h" // Needed for matrix and quaternion operations
#include "rlgl.h"

#include <math.h>

struct Cube {
    Transform transform;
    
    Vector3 position;
    Vector3 nextPosition;
    Vector3 direction;

    Quaternion rotation;
    Quaternion nextRotation;
    Vector3 rotationAxis;
    float rotationAmount;
    Vector3 perpendicularAxis;
};
Cube cube;

struct CubeCamera {
    Camera c3d;
    float distance;
    Vector3 direction;
    
    float angleX;
    float angleY;
};
CubeCamera camera;

struct Mouse {
    Vector2 position;
    Vector2 prevPosition;
    Vector2 deltaPosition;
};
Mouse mouse;

void initCubeAndCamera() {

    cube = {
        .transform =
        {
            .translation = {0.5f, 0.5f, 0.5f},
            .rotation = QuaternionIdentity(),
            .scale = Vector3One(),
        },

        .position = {0.5f, 0.5f, 0.5f},
        .nextPosition = {0.5f, 0.5f, 0.5f},
        .direction = {-1.0, 0.0f, 0.0f},

        .rotation = QuaternionIdentity(),
        .nextRotation = QuaternionIdentity(),
        .rotationAxis = {0.0f, 0.0f, 1.0f},
        .rotationAmount = 90.0f,
        .perpendicularAxis = {1.0f, 0.0f, 0.0f},
    };
        
    camera.c3d = {
        .position = (Vector3){ 6.0f, 6.0f, 2.0f },
        .target = (Vector3) { cube.transform.translation.x, 0.0f, cube.transform.translation.z }, // Look at ground level
        .up = (Vector3) { 0.0f, 1.0f, 0.0f },
        .fovy = 45.0f,
        .projection = CAMERA_PERSPECTIVE,
    };

    // Camera orbit parameters
    Vector3 cubeCameraOffset = { 
        camera.c3d.position.x - camera.c3d.target.x,
        camera.c3d.position.y - camera.c3d.target.y,
        camera.c3d.position.z - camera.c3d.target.z
    };
    camera.distance = sqrtf(cubeCameraOffset.x * cubeCameraOffset.x +
                            cubeCameraOffset.y * cubeCameraOffset.y +
                            cubeCameraOffset.z * cubeCameraOffset.z);

    camera.angleX = atan2f(cubeCameraOffset.x, cubeCameraOffset.z);
    camera.angleY = asinf(cubeCameraOffset.y / camera.distance);
}

void mouseUpdateCameraAngles() {
    mouse.deltaPosition = { 0.0f, 0.0f };
    mouse.position = GetMousePosition();
            
    if (mouse.prevPosition.x != 0.0f || mouse.prevPosition.y != 0.0f) {
        mouse.deltaPosition.x = mouse.position.x - mouse.prevPosition.x;
        mouse.deltaPosition.y = mouse.position.y - mouse.prevPosition.y;
    }
    mouse.prevPosition = mouse.position;
            
    // Update camera angles based on mouse movement
    camera.angleX -= mouse.deltaPosition.x * 0.003f;
    camera.angleY = fmax(fmin(camera.angleY + mouse.deltaPosition.y * 0.003f, PI/2.0f), -PI/2.0f);  
}

void mouseUpdateCubeDirection() {
    // Calculate camera direction vector (normalized)
    camera.direction = {
        camera.c3d.target.x - camera.c3d.position.x,
        0.0f,
        camera.c3d.target.z - camera.c3d.position.z
    };
    float length = sqrtf(camera.direction.x * camera.direction.x + camera.direction.z * camera.direction.z);
    camera.direction.x /= length;
    camera.direction.z /= length;

    // Calculate dot products with world axes
    float dotX = fabsf(camera.direction.x);  // Dot product with (1,0,0)
    float dotZ = fabsf(camera.direction.z);  // Dot product with (0,0,1)

    // Determine cube move direction based on camera orientation
    cube.direction = {};
    if (dotX > dotZ) {
        // Camera is more aligned with X axis
        cube.direction.x = (camera.direction.x > 0) ? 1.0f : -1.0f;
    } else {
        // Camera is more aligned with Z axis
        cube.direction.z = (camera.direction.z > 0) ? 1.0f : -1.0f;
    }
}

void mouseUpdateCubeRotation() {

    if (Vector3Equals(cube.direction, { 1.0f, 0.0f, 0.0f })) {
        cube.rotationAxis = {0.0f, 0.0f, 1.0f};
        cube.rotationAmount = -90.0f;
        cube.perpendicularAxis = {1.0f, 0.0f, 0.0f};
    } else if (Vector3Equals(cube.direction, {-1.0f, 0.0f, 0.0f})) {
        cube.rotationAxis = {0.0f, 0.0f, 1.0f};
        cube.rotationAmount = 90.0f;        
        cube.perpendicularAxis = {1.0f, 0.0f, 0.0f};
    } else if (Vector3Equals(cube.direction, { 0.0f, 0.0f, 1.0f })) {
        cube.rotationAxis = {1.0f, 0.0f, 0.0f};
        cube.rotationAmount = 90.0f;        
        cube.perpendicularAxis = {0.0f, 0.0f, 1.0f};
    } else if (Vector3Equals(cube.direction, { 0.0f, 0.0f, -1.0f })) {
        cube.rotationAxis = {1.0f, 0.0f, 0.0f};
        cube.rotationAmount = -90.0f;       
        cube.perpendicularAxis = {0.0f, 0.0f, 1.0f};
    }
}

void handleMouseButton() {
    mouseUpdateCameraAngles();
    mouseUpdateCubeDirection();
    mouseUpdateCubeRotation();
}

const float MIN_CAMERA_DISTANCE = 5.0f;  // Minimum zoom distance
const float MAX_CAMERA_DISTANCE = 20.0f;  // Maximum zoom distance
const float ZOOM_SPEED = 1.0f;           // Zoom sensitivity
void handleMouseWheel() {
    // Handle mouse wheel for zoom
    float mouseWheel = GetMouseWheelMove();
    if (mouseWheel != 0) {
        camera.distance -= mouseWheel * ZOOM_SPEED;
        camera.distance = fmax(MIN_CAMERA_DISTANCE, fmin(camera.distance, MAX_CAMERA_DISTANCE));
    }   
}

void updateCameraPosition() {
    // Update camera position based on angles and cube position
    camera.c3d.target = (Vector3){ cube.transform.translation.x, 0.0f, cube.transform.translation.z };  // Camera looks at ground level
    camera.c3d.position.x = camera.c3d.target.x + camera.distance * cosf(camera.angleY) * sinf(camera.angleX);
    camera.c3d.position.y = camera.c3d.target.y + camera.distance * sinf(camera.angleY);
    camera.c3d.position.z = camera.c3d.target.z + camera.distance * cosf(camera.angleY) * cosf(camera.angleX);
}

int main(void)
{
    const int screenWidth = 1280;
    const int screenHeight = 720;

	InitWindow(screenWidth, screenHeight, "trayimg - Example");
	SetTargetFPS(60);
    SetTraceLogLevel(LOG_ALL); // Enable all logs

	initCubeAndCamera();
	TRACELOG(LOG_DEBUG, "***** Started!");
	printf("This is a test message!\n");
	fflush(stdout); // Ensure output is flushed
// Define axis parameters
	const float axisLength = 2.0f;  // Length of each axis
	const float coneLength = 0.3f;  // Length of the cone part
	const float coneRadius = 0.1f;  // Radius of the cone base
	const float lineRadius = 0.02f; // Radius for the axis lines    

// Animation walk state
	bool isAnimating = false;
	float animationTime = 0.0f;
	const float ANIMATION_WALK_DURATION = 0.5f;

// Movement queue
	bool hasQueuedMovement = false;
	int queuedKey = 0;  // Store the last pressed key

	while (!WindowShouldClose()) { // Main game loop
        
		float deltaTime = GetFrameTime();

		// Update camera position when right mouse button is held
		if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
			handleMouseButton();
		} else {
			mouse.prevPosition = (Vector2){ 0.0f, 0.0f };
		}

		handleMouseWheel();

		// Handle key presses to start animation
		if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_A) || IsKeyPressed(KEY_D)) {
			int pressedKey =
				IsKeyPressed(KEY_W) ? KEY_W :
				IsKeyPressed(KEY_S) ? KEY_S :
				IsKeyPressed(KEY_A) ? KEY_A : KEY_D;
                           
			if (!isAnimating) {
				// Start animation immediately if not already animating
				isAnimating = true;
				animationTime = 0.0f;

				// Process the movement
				// if (queuedKey == KEY_W) {
				//  cube.nextPosition = cube.position + cube.direction;
				//  cube.rotation = cube.transform.rotation;
				//  cube.nextRotation = QuaternionFromAxisAngle((Vector3){0, 0, 1}, 90.0f * DEG2RAD);
				// }
				// else if (queuedKey == KEY_S) {
				//  cube.nextPosition = cube.position + Vector3Scale(cube.direction, -1.0f);
				//  cube.rotation = cube.transform.rotation;
				//  cube.nextRotation = QuaternionFromAxisAngle((Vector3){0, 0, 1}, -90.0f * DEG2RAD);
				// }
				// else if (queuedKey == KEY_A) {
				//  cube.nextPosition = cube.position + Vector3Scale(Vector3Perpendicular(cube.direction), -1.0f);
				//  cube.rotation = cube.transform.rotation;
				//  cube.nextRotation = QuaternionFromAxisAngle((Vector3){1, 0, 0}, 90.0f * DEG2RAD);
				// }
				// else if (queuedKey == KEY_D) {
				//  cube.nextPosition = cube.position + Vector3Perpendicular(cube.direction);
				//  cube.rotation = cube.transform.rotation;
				//  cube.nextRotation = QuaternionFromAxisAngle((Vector3){1, 0, 0}, -90.0f * DEG2RAD);
				// }
                
				if (queuedKey == KEY_W) {
					cube.nextPosition = cube.position + cube.direction;
					cube.nextRotation = QuaternionFromAxisAngle(cube.rotationAxis, cube.rotationAmount * DEG2RAD);
				}
				else if (queuedKey == KEY_S) {
					cube.nextPosition = cube.position + Vector3Scale(cube.direction, -1.0f);
					cube.nextRotation = QuaternionFromAxisAngle(cube.rotationAxis, -cube.rotationAmount * DEG2RAD);
				}
				else if (queuedKey == KEY_A) {
					cube.nextPosition = cube.position + Vector3Scale(Vector3Perpendicular(cube.direction), -1.0f); 
					cube.nextRotation = QuaternionFromAxisAngle(cube.perpendicularAxis, cube.rotationAmount * DEG2RAD);
				}
				else if (queuedKey == KEY_D) {
					cube.nextPosition = cube.position + Vector3Perpendicular(cube.direction);
					cube.nextRotation = QuaternionFromAxisAngle(cube.perpendicularAxis, -cube.rotationAmount * DEG2RAD);
				}
                
			} else {
				// Queue the movement for when current animation ends
				hasQueuedMovement = true;
				queuedKey = pressedKey;
			}
		}

		updateCameraPosition();
                
		// Update animation
		if (isAnimating) {
			animationTime += deltaTime;
			float t = animationTime / ANIMATION_WALK_DURATION;
            
			if (t >= 1.0f) {
				// Animation complete
				t = 1.0f;
				isAnimating = false;
				cube.position = cube.nextPosition;
				cube.transform.rotation = QuaternionIdentity();
                
				// If there's a queued movement, start it immediately
				if (hasQueuedMovement) {
					isAnimating = true;
					animationTime = 0.0f;
					hasQueuedMovement = false;
                    
					// Process the queued movement
					if (queuedKey == KEY_W) {
						cube.nextPosition = cube.position + cube.direction;
						cube.rotation = cube.transform.rotation;
						cube.nextRotation = QuaternionFromAxisAngle((Vector3){0, 0, 1}, 90.0f * DEG2RAD);
					}
					else if (queuedKey == KEY_S) {
						cube.nextPosition = cube.position + Vector3Scale(cube.direction, -1.0f);
						cube.rotation = cube.transform.rotation;
						cube.nextRotation = QuaternionFromAxisAngle((Vector3){0, 0, 1}, -90.0f * DEG2RAD);
					}
					else if (queuedKey == KEY_A) {
                        cube.nextPosition = cube.position + Vector3Scale(Vector3Perpendicular(cube.direction), -1.0f);
                        cube.rotation = cube.transform.rotation;
                        cube.nextRotation = QuaternionFromAxisAngle((Vector3){1, 0, 0}, 90.0f * DEG2RAD);
                    }
                    else if (queuedKey == KEY_D) {
                        cube.nextPosition = cube.position + Vector3Perpendicular(cube.direction);
                        cube.rotation = cube.transform.rotation;
                        cube.nextRotation = QuaternionFromAxisAngle((Vector3){1, 0, 0}, -90.0f * DEG2RAD);
                    }
                }
            } else {
                // Interpolate rotation and position
                float smoothT = t * t * (3.0f - 2.0f * t);
                cube.transform.rotation = QuaternionSlerp(cube.rotation, cube.nextRotation, smoothT);
                cube.transform.translation = Vector3Lerp(cube.position, cube.nextPosition, smoothT);
            }
        }

        // Convert Transform to Matrix
        Matrix rotationMatrix = QuaternionToMatrix(cube.transform.rotation);
        Matrix scaleMatrix = MatrixScale(cube.transform.scale.x,
                                         cube.transform.scale.y,
                                         cube.transform.scale.z);
        Matrix translationMatrix = MatrixTranslate(cube.transform.translation.x,
                                                   cube.transform.translation.y,
                                                   cube.transform.translation.z);
        
        // First apply scale, then rotation, then translation
        Matrix transformMatrix = MatrixMultiply(MatrixMultiply(scaleMatrix, rotationMatrix), translationMatrix);
        
        BeginDrawing();
        {
            ClearBackground(RAYWHITE);
            BeginMode3D(camera.c3d);
            {

                // Draw grid first
                DrawGrid(100, 1.0f);

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

            }
            EndMode3D();

            DrawFPS(10, 10);
            DrawText("Use WASD to roll cube", 10, 40, 20, DARKGRAY);
            DrawText("Hold right mouse button to orbit camera", 10, 70, 20, DARKGRAY);
            DrawText(TextFormat("Cube Position: (%.1f, %.1f, %.1f)", 
                                cube.transform.translation.x, 
                                cube.transform.translation.y, 
                                cube.transform.translation.z), 10, 100, 20, DARKGRAY);
            DrawText(TextFormat("Cube Direction: (%.1f, %.1f, %.1f)",
                                cube.direction.x,
                                cube.direction.y,
                                cube.direction.z), 10, 130, 20, DARKGRAY);
            DrawText(TextFormat("Cube Rotation Axis: (%.1f, %.1f, %.1f)",
                                cube.rotationAxis.x,
                                cube.rotationAxis.y,
                                cube.rotationAxis.z), 10, 160, 20, DARKGRAY);
            DrawText(TextFormat("Cube Rotation Amount: %.1f degrees", 
                                cube.rotationAmount), 10, 190, 20, DARKGRAY);
            DrawText(TextFormat("Cube Perpendicular Axis: (%.1f, %.1f, %.1f)",
                                cube.perpendicularAxis.x,
                                cube.perpendicularAxis.y,
                                cube.perpendicularAxis.z), 10, 220, 20, DARKGRAY);
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
