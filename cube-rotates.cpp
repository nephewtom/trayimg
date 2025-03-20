#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

typedef struct {
    Transform transform;
} Cube;

// Global variables
Cube cube;
Camera3D camera;

void initCubeAndCamera() {
    // Initialize cube
    cube.transform.translation = (Vector3){ 0.5f, 0.5f, 0.5f };
    cube.transform.rotation = QuaternionIdentity();
    cube.transform.scale = (Vector3){ 1.0f, 1.0f, 1.0f };
    
    // Initialize camera
    camera.position = (Vector3){ 3.0f, 3.0f, 3.0f };
    camera.target = (Vector3){ 0.5f, 0.5f, 0.5f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
}

int main() {
    // Initialization
    const int screenWidth = 800;
    const int screenHeight = 600;
    
    InitWindow(screenWidth, screenHeight, "Cube 360° Rotation");
    SetTargetFPS(60);
    
    initCubeAndCamera();

	// Create a mesh for the cube
    Mesh cubeMesh = GenMeshCube(1.0f, 1.0f, 1.0f);
    // Create a material with a red color
    Material material = LoadMaterialDefault();
    material.maps[MATERIAL_MAP_DIFFUSE].color = RED;


    // Create a model for the cube
    Model cubeModel = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
    // Set the cube model material color to red
    cubeModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = RED;

    
    // Animation variables
    float animationTime = 0.0f;
    float animationDuration = 2.0f;  // Time for a rotation segment
    float targetAngle = 180.0f;       // Initial target angle
    Quaternion startRotation = QuaternionIdentity();
    Quaternion targetRotation = QuaternionFromAxisAngle((Vector3){0, 1, 0}, targetAngle * DEG2RAD);

	// Using simple rotation
	float totalRotation = 0.0f;        // Tracks total rotation in degrees
	float rotationSpeed = 90.0f;      // Degrees per second (adjust as needed)
	bool isRotating = true;            // Control flag for rotation
    
    // Main game loop
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        
        // Handle rotation animation
        float t = animationTime / animationDuration;
        animationTime += deltaTime;
        
        // If we've reached the end of the current animation segment
        if (t >= 1.0f) {
            // Reset animation timer
            animationTime = 0.0f;
            
            // Store current rotation as the starting point for next segment
            startRotation = cube.transform.rotation;
            
            // Advance target angle by 90 degrees
            targetAngle += 180.0f;
            if (targetAngle >= 360.0f) {
                targetAngle -= 360.0f;
            }
            
            // Create new target rotation
            targetRotation = QuaternionFromAxisAngle((Vector3){0, 1, 0}, targetAngle * DEG2RAD);
        } else {
            // Use smooth interpolation factor (ease-in-out)
            float smoothT = t * t * (3.0f - 2.0f * t);
            
            // Perform slerp between start and target rotations
            cube.transform.rotation = QuaternionSlerp(startRotation, targetRotation, smoothT);
        }

		// // Handle rotation
		// if (isRotating) {
		// 	// Update total rotation based on time
		// 	totalRotation += rotationSpeed * deltaTime;
        
		// 	// Keep the angle in the 0-360 range
		// 	if (totalRotation >= 360.0f) {
		// 		totalRotation -= 360.0f;
		// 	}
        
		// 	// Create a quaternion for the current rotation
		// 	Quaternion yRotation = QuaternionFromAxisAngle(
		// 		(Vector3){1, 0, 0},  // Y-axis
		// 		totalRotation * DEG2RAD  // Current angle in radians
		// 		);
        
		// 	// Apply the rotation directly (no interpolation needed)
		// 	cube.transform.rotation = yRotation;
		// }
    
		// // Optional: Add control to toggle rotation with a key
		// if (IsKeyPressed(KEY_SPACE)) {
		// 	isRotating = !isRotating;
		// }		


        // Draw
        BeginDrawing();
		ClearBackground(RAYWHITE);
            
		BeginMode3D(camera);
		// Create transformation matrix
		Matrix transform = MatrixIdentity();
                
		// Apply scale
		transform = MatrixMultiply(transform, 
								   MatrixScale(cube.transform.scale.x, 
											   cube.transform.scale.y, 
											   cube.transform.scale.z));
                
		// Apply rotation (quaternion to matrix)
		Matrix rotationMatrix = QuaternionToMatrix(cube.transform.rotation);
		transform = MatrixMultiply(transform, rotationMatrix);
                
		// Apply translation
		transform = MatrixMultiply(transform, 
								   MatrixTranslate(cube.transform.translation.x, 
												   cube.transform.translation.y, 
												   cube.transform.translation.z));



		// SOLUTION 3

		// Apply transformation matrix to OpenGL
		rlPushMatrix();
		rlMultMatrixf(MatrixToFloat(transform));

		// Draw the solid cube with the model
		DrawModel(cubeModel, Vector3Zero(), 1.0f, WHITE);

		// Draw wireframe cube on top of the solid cube
		DrawCubeWires(Vector3Zero(), 1.0f, 1.0f, 1.0f, BLACK);
		rlPopMatrix();



		
		// SOLUTION 2
		// // Draw the mesh with the transformation matrix and colored material
		// DrawMesh(cubeMesh, material, transform);
		// // Draw wireframe on top of the mesh
		// // DrawMeshWires(cubeMesh, material, transform);



		// SOLUTION 1
		// // Draw the cube with the transformation matrix
		// DrawMesh(GenMeshCube(1.0f, 1.0f, 1.0f), LoadMaterialDefault(), transform);



        
		// Draw coordinate system for reference
		DrawGrid(10, 1.0f);
		EndMode3D();
            
		// Draw UI
		DrawFPS(10, 10);
		DrawText("Rotation Angle: ", 10, 40, 20, DARKGRAY);
		DrawText(TextFormat("%.1f°", t * 90.0f + (targetAngle - 90.0f)), 170, 40, 20, RED);
            
        EndDrawing();
    }
    
    // De-Initialization
    CloseWindow();
    
    return 0;
}
