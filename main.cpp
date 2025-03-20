#include <stdio.h>

#define SUPPORT_TRACELOG
#define SUPPORT_TRACELOG_DEBUG
#include "raylib.h"
#include "raymath.h" // Needed for matrix and quaternion operations
#include "rlgl.h"
#include "utils.h"

// Define axis parameters
const float axisLength = 2.0f;  // Length of each axis
const float coneLength = 0.3f;  // Length of the cone part
const float coneRadius = 0.1f;  // Radius of the cone base
const float lineRadius = 0.02f; // Radius for the axis lines

void DrawAxis() {
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

struct Mouse {
    Vector2 position;
    Vector2 prevPosition;
    Vector2 deltaPosition;
};
Mouse mouse;

struct Cube {
	Vector3 position;
	Vector3 direction;
	Transform transform;
	
	Vector3 rotationAxis;
	float rotationAmount;
	Vector3 translate;
};
Cube cube;
Vector3 cubeInitPos = {0.5f, 0.5f, 0.5f};

struct CubeCamera {
	Camera c3d;
	float distance;
	Vector3 direction;
    
	float angleX;
	float angleY;
};
CubeCamera camera;

void initCubeAndCamera() {

	cube = {
		.position = cubeInitPos,
		.direction = {-1.0, 0.0f, 0.0f},
		.transform =
		{
            .translation = cubeInitPos,
            .rotation = QuaternionIdentity(),
            .scale = Vector3One(),
        },
	
		.rotationAxis = {0, 0, 0},
		.rotationAmount = 0.0f,
		.translate = {0, 0, 0},
	};

	camera.c3d = {
		.position = (Vector3){6.0f, 6.0f, 2.0f},
		.target = (Vector3){cube.transform.translation.x, 0.0f,
			cube.transform.translation.z}, // Look at ground level
		.up = (Vector3){0.0f, 1.0f, 0.0f},
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
    camera.angleY = fmax(fmin(camera.angleY + mouse.deltaPosition.y * 0.003f, PI/2.5f), -PI/2.5f);
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


// First, let's complete the updateCubeRotation function
void updateCubeRotation(int key) {
	Vector3 xAxis = {1.0f, 0.0f, 0.0f};
	Vector3 xAxisNeg = {-1.0f, 0.0f, 0.0f};
	Vector3 zAxis = {0.0f, 0.0f, 1.0f};
	Vector3 zAxisNeg = {0.0f, 0.0f, -1.0f};

	Vector3 rotationAxis = {0, 0, 0};
	float rotationAmount = 0;
		
	cube.transform.translation = cube.position;
	cube.translate = Vector3Subtract(cubeInitPos, cube.position);					

	if (Vector3Equals(cube.direction, xAxis)) {
		if (key == KEY_W) {
			rotationAxis = zAxis;
			rotationAmount = -90.0f;
		} else if (key == KEY_S) {
			rotationAxis = (Vector3){0.0f, 0.0f, 1.0f}; // Z axis
			rotationAmount = -90.0f;
		} else if (key == KEY_A) {
			rotationAxis = (Vector3){0.0f, 1.0f, 0.0f}; // Y axis
			rotationAmount = 90.0f;
		} else if (key == KEY_D) {
			rotationAxis = (Vector3){0.0f, 1.0f, 0.0f}; // Y axis
			rotationAmount = -90.0f;
		}
	} else if (Vector3Equals(cube.direction, xAxisNeg)) {
		if (key == KEY_W) {
			rotationAxis = zAxis;
			rotationAmount = 90.0f;
		} else if (key == KEY_S) {
			rotationAxis = zAxis;
			rotationAmount = -90.0f;
		} else if (key == KEY_A) {
			rotationAxis = xAxis;
			rotationAmount = 90.0f;
		} else if (key == KEY_D) {
			rotationAxis = xAxis;
			rotationAmount = 90.0f;
		}

	} else if (Vector3Equals(cube.direction, zAxis)) {
		// Cube facing +Z direction
		if (key == KEY_W) {
			rotationAxis = (Vector3){1.0f, 0.0f, 0.0f}; // X axis
			rotationAmount = 90.0f;
		} else if (key == KEY_S) {
			rotationAxis = (Vector3){1.0f, 0.0f, 0.0f}; // X axis
			rotationAmount = -90.0f;
		} else if (key == KEY_A) {
			rotationAxis = (Vector3){0.0f, 1.0f, 0.0f}; // Y axis
			rotationAmount = -90.0f;
		} else if (key == KEY_D) {
			rotationAxis = (Vector3){0.0f, 1.0f, 0.0f}; // Y axis
			rotationAmount = 90.0f;
		}
	} else if (Vector3Equals(cube.direction, zAxisNeg)) {
		// Cube facing -Z direction
		if (key == KEY_W) {
			rotationAxis = (Vector3){1.0f, 0.0f, 0.0f}; // X axis
			rotationAmount = -90.0f;
		} else if (key == KEY_S) {
			rotationAxis = (Vector3){1.0f, 0.0f, 0.0f}; // X axis
			rotationAmount = 90.0f;
		} else if (key == KEY_A) {
			rotationAxis = (Vector3){0.0f, 1.0f, 0.0f}; // Y axis
			rotationAmount = 90.0f;
		} else if (key == KEY_D) {
			rotationAxis = (Vector3){0.0f, 1.0f, 0.0f}; // Y axis
			rotationAmount = -90.0f;
		}
	}
    
	// Store the rotation info
	cube.rotationAxis = rotationAxis;
	cube.rotationAmount = rotationAmount;
}


void handleMouseButton() {
	mouseUpdateCameraAngles();
	mouseUpdateCubeDirection();
}

void updateCamera() {
// Update camera position based on angles and cube position
	camera.c3d.target = (Vector3){ cube.transform.translation.x, 0.0f, cube.transform.translation.z };  // Camera looks at ground level
	camera.c3d.position.x = camera.c3d.target.x + camera.distance * cosf(camera.angleY) * sinf(camera.angleX);
	camera.c3d.position.y = camera.c3d.target.y + camera.distance * sinf(camera.angleY);
	camera.c3d.position.z = camera.c3d.target.z + camera.distance * cosf(camera.angleY) * cosf(camera.angleX);
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

int main(void)
{
	// Initialization
	const int screenWidth = 1280;
	const int screenHeight = 720;

    SetTraceLogLevel(LOG_ALL); // Enable all logs
	TRACELOGD("*** Started main ***");

	InitWindow(screenWidth, screenHeight, "trayimg - Example");
	SetTargetFPS(60);

	initCubeAndCamera();

	bool isMoving = false;
	float rotationAngle = 0.0f;
	float rotationSpeed = 180.f; // per second

	while (!WindowShouldClose()) { // Main game loop
		float deltaTime = GetFrameTime();

		// Update camera position when right mouse button is held
		if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
			handleMouseButton();
		} else {
			mouse.prevPosition = (Vector2){ 0.0f, 0.0f };
		}
		handleMouseWheel();
	
		updateCamera();

		// Handle key presses to start animation
		if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_A) || IsKeyPressed(KEY_D)) {
			int pressedKey =
				IsKeyPressed(KEY_W) ? KEY_W :
				IsKeyPressed(KEY_S) ? KEY_S :
				IsKeyPressed(KEY_A) ? KEY_A : KEY_D;
                           
			if (!isMoving) {
				// Start animation immediately if not already animating
				isMoving = true;
				rotationAngle = 0.0f;
                
				updateCubeRotation(pressedKey);

			} else {
			
				// queuedKey = pressedKey;
			}
		}

		if (isMoving) {
			if (fabs(rotationAngle) >= fabs(cube.rotationAmount)) {
				rotationAngle = cube.rotationAmount;
				isMoving = false;
			} else {
				rotationAngle += cube.rotationAmount * deltaTime;
			}						
		}
		
		// Draw
		BeginDrawing();
		{
			ClearBackground(RAYWHITE);

			Matrix rotation;
			BeginMode3D(camera.c3d);
			{
				DrawGrid(10, 1.0f);

				Vector3 bluePos = {-3.0f, 0.5f, 2.0f};
				DrawCube(bluePos, 1.0f, 1.0f, 1.0f, BLUE);
				DrawCubeWires(bluePos, 1.0f, 1.0f, 1.0f, GREEN);

				rlPushMatrix();
				rlTranslatef(cube.translate.x, cube.translate.y, cube.translate.z);

				rotation = MatrixRotate(cube.rotationAxis, rotationAngle * DEG2RAD);
				rlMultMatrixf(MatrixToFloat(rotation));
				rlTranslatef(-cube.translate.x, -cube.translate.y, -cube.translate.z);

				DrawCube(cube.transform.translation, 1.0f, 1.0f, 1.0f, RED);
				DrawCubeWires(cube.transform.translation, 1.0f, 1.0f, 1.0f, GREEN);

				rlPopMatrix();
				cube.position = Vector3Transform(cube.transform.translation, rotation);
				Vector3 orangePos = {3.0f, 0.5f, 2.0f};
				DrawCube(orangePos, 1.0f, 1.0f, 1.0f, ORANGE);
				DrawCubeWires(orangePos, 1.0f, 1.0f, 1.0f, GREEN);
								
				DrawAxis();

			}
			EndMode3D();

			DrawFPS(10, 10);
			DrawText("Use WASD to roll cube", 10, 30, 20, DARKGRAY);
			DrawText("Hold right mouse button to orbit camera", 10, 70, 20, DARKGRAY);
			DrawText(TextFormat("Mouse: angleX=%.2f | angleY=%.2f",
								camera.angleX * RAD2DEG,
								camera.angleY * RAD2DEG),
					 10, 90, 20, DARKGRAY);

			DrawText(TextFormat("Cube Position: (%.2f, %.2f, %.2f)",
								cube.position.x, 
								cube.position.y, 
								cube.position.z), 10, 130, 20, DARKGRAY);											

			DrawText(TextFormat("Cube Direction: (%.2f, %.2f, %.2f)",
								cube.direction.x, 
								cube.direction.y, 
								cube.direction.z), 10, 150, 20, DARKGRAY);

			DrawText(TextFormat("Cube Translate: (%.2f, %.2f, %.2f)",
								cube.translate.x, 
								cube.translate.y, 
								cube.translate.z), 10, 170, 20, DARKGRAY);

			DrawText(TextFormat("Cube Transform Traslation: (%.2f, %.2f, %.2f)",
								cube.transform.translation.x, 
								cube.transform.translation.y, 
								cube.transform.translation.z), 10, 190, 20, DARKGRAY);		


		}
		EndDrawing();
	}

	// Cleanup
	CloseWindow();
	return 0;
}
