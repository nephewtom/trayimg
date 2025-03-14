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
	const float ANIMATION_WALK_DURATION = 0.5f; // Doubled from 2.0f to 4.0f for even slower movement
	Quaternion currentRotation = QuaternionIdentity();
	Quaternion targetRotation = QuaternionIdentity();
	Vector3 currentPosition = {0.5f, 0.5f, 0.5f};  // Starting position
	Vector3 targetPosition = {-0.5f, 0.5f, 0.5f};  // Initial target position

	// Jump mechanics
	const float JUMP_FORCE = 8.0f;
	const float GRAVITY = -8.0f;
	float verticalVelocity = 0.0f;
	const float GROUND_Y = 0.0f;        // Ground plane is at 0.0f (the grid)
	const float CUBE_HALF_SIZE = 0.5f;  // Half the size of the cube
	bool isJumping = false;
	bool hasMovedInAir = false;

	// Squash and stretch parameters
	const float SQUASH_AMOUNT = 0.3f;     // Reduced squash for less extreme compression
	const float STRETCH_AMOUNT = 0.8f;     // Increased for more dramatic stretch
	const float SQUASH_DURATION = 0.15f;   // Slightly faster squash
	const float VERTICAL_STRETCH_MULT = 3.0f;  // More vertical stretch
	const float HORIZONTAL_SQUEEZE = 0.4f;     // Less horizontal squeeze
	float squashStretchTimer = 0.0f;
	bool isSquashing = false;
	Vector3 originalScale = { 1.0f, 1.0f, 1.0f };
	Vector3 currentScale = { 1.0f, 1.0f, 1.0f };  // Add this to track current scale
	const float SCALE_TRANSITION_SPEED = 10.0f;    // Speed of scale transitions
	float targetYPosition = 0.5f;                  // Target Y position when standing

	// Initial cube position should have its base on the ground
	cubeTransform.translation.y = GROUND_Y + CUBE_HALF_SIZE;

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

		// Handle jump and gravity
		if (!isAnimating) { // Only allow jumping when not moving horizontally
			if (IsKeyPressed(KEY_SPACE) && !isJumping) {
				verticalVelocity = JUMP_FORCE;
				isJumping = true;
				hasMovedInAir = false;
				isSquashing = true;
				squashStretchTimer = 0.0f;
			}
		}

		// Update squash and stretch animation
		Vector3 targetScale = originalScale;  // Default target scale

		if (isSquashing) {
			squashStretchTimer += deltaTime;
			if (squashStretchTimer < SQUASH_DURATION) {
				// Initial squash when jumping
				float squashProgress = squashStretchTimer / SQUASH_DURATION;
				float squashFactor = 1.0f - SQUASH_AMOUNT * (1.0f - squashProgress);
				float stretchFactor = 1.0f + SQUASH_AMOUNT * (1.0f - squashProgress);
				targetScale = (Vector3){ stretchFactor, squashFactor, stretchFactor };
			} else {
				isSquashing = false;
				squashStretchTimer = 0.0f;
			}
		} else if (isJumping) {
			// Dynamic stretch based on vertical velocity
			float velocityFactor = fabsf(verticalVelocity) / JUMP_FORCE;
			float stretchAmount = STRETCH_AMOUNT * velocityFactor;
			
			if (verticalVelocity > 0) {
				// Stretching upward during ascent - more dramatic vertical stretch
				targetScale = (Vector3){ 
					1.0f - (stretchAmount * HORIZONTAL_SQUEEZE),      // Subtle horizontal squeeze
					1.0f + (stretchAmount * VERTICAL_STRETCH_MULT),   // Much taller vertically
					1.0f - (stretchAmount * HORIZONTAL_SQUEEZE)       // Subtle horizontal squeeze
				};
			} else {
				// Stretching horizontally during descent - more subtle
				targetScale = (Vector3){ 
					1.0f + (stretchAmount * 0.5f),          // Less horizontal stretch
					1.0f - (stretchAmount * 0.8f),          // Less vertical squash
					1.0f + (stretchAmount * 0.5f)           // Less horizontal stretch
				};
			}
		}

		// This code smoothly transitions the cube's scale between different states
		// transitionSpeed controls how fast the scale changes based on time
		float transitionSpeed = SCALE_TRANSITION_SPEED * deltaTime;
		
		// Gradually move each scale component (x,y,z) towards its target value
		// The difference between target and current is multiplied by transition speed
		currentScale.x += (targetScale.x - currentScale.x) * transitionSpeed;
		currentScale.y += (targetScale.y - currentScale.y) * transitionSpeed; 
		currentScale.z += (targetScale.z - currentScale.z) * transitionSpeed;
		
		// Apply the interpolated scale to the cube's transform
		cubeTransform.scale = currentScale;

		if (isJumping) {
			// Apply gravity and update position
			verticalVelocity += GRAVITY * deltaTime;
			float nextY = cubeTransform.translation.y + verticalVelocity * deltaTime;
			
			// Calculate the position of the cube's base (center position - half height)
			float baseY = nextY - (0.5f * cubeTransform.scale.y);
			
			// Check if the base of the cube would go below the ground
			if (baseY <= 0.0f) {
				// Position the cube so its base is exactly at ground level
				cubeTransform.translation.y = 0.5f * cubeTransform.scale.y;
				verticalVelocity = 0.0f;
				isJumping = false;
				hasMovedInAir = false;
				isSquashing = true; // Start landing squash
				squashStretchTimer = 0.0f;
				// Set target Y position based on current scale
				targetYPosition = 0.5f * cubeTransform.scale.y;
			} else {
				// Update position normally
				cubeTransform.translation.y = nextY;
			}
		} else if (!isSquashing) {
			// Reset scale when not jumping or squashing
			cubeTransform.scale = originalScale;
			// Update target position based on current scale
			targetYPosition = 0.5f * cubeTransform.scale.y;
			
			// Smoothly interpolate to the correct height while keeping base at ground level
			if (!isJumping) {
				float yDiff = targetYPosition - cubeTransform.translation.y;
				// Use smoother interpolation
				float smoothFactor = 1.0f - powf(0.001f, deltaTime); // Exponential smoothing
				float nextY = cubeTransform.translation.y + yDiff * smoothFactor;
				
				// Ensure the base never goes below ground
				float baseY = nextY - (0.5f * cubeTransform.scale.y);
				if (baseY < 0.0f) {
					nextY = 0.5f * cubeTransform.scale.y;
				}
				
				cubeTransform.translation.y = nextY;
			}
		}

		// Handle W key press to start animation
		if (!isAnimating && !isJumping) {  // Only allow movement when not jumping and not already animating
			if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_A) || IsKeyPressed(KEY_D)) {
				isAnimating = true;
				animationTime = 0.0f;
			}
			if (IsKeyPressed(KEY_W)){
				targetPosition = currentPosition + (Vector3){-1.0f, 0.0f, 0.0f};
				currentRotation = cubeTransform.rotation;
				
				// Rotate +90 degrees around Z axis to roll towards -X
				targetRotation = QuaternionFromAxisAngle((Vector3){0, 0, 1}, 90.0f * DEG2RAD);
			}
			if (IsKeyPressed(KEY_S)){
				targetPosition = currentPosition + (Vector3){1.0f, 0.0f, 0.0f};
				currentRotation = cubeTransform.rotation;
				
				// Rotate +90 degrees around Z axis to roll towards -X
				targetRotation = QuaternionFromAxisAngle((Vector3){0, 0, 1}, -90.0f * DEG2RAD);
			}
			if (IsKeyPressed(KEY_A)){
				targetPosition = currentPosition + (Vector3){0.0f, 0.0f, 1.0f};
				currentRotation = cubeTransform.rotation;
				
				targetRotation = QuaternionFromAxisAngle((Vector3){1, 0, 0}, 90.0f * DEG2RAD);
			}
			if (IsKeyPressed(KEY_D)){
				targetPosition = currentPosition + (Vector3){0.0f, 0.0f, -1.0f};
				currentRotation = cubeTransform.rotation;
				
				targetRotation = QuaternionFromAxisAngle((Vector3){1, 0, 0}, -90.0f * DEG2RAD);
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
