#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"

#include <math.h>
#include <stdarg.h>

Vector2 fullHD = { 1280, 720 };
float screenWidth = fullHD.x;
float screenHeight = fullHD.y;

int main()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "Trayimg!");
    SetWindowPosition(25, 50);

    SetTargetFPS(60);

    rlImGuiSetup(true);

    float size = 400.0f;
    while (!WindowShouldClose()) {

        // CheckResize();

        BeginDrawing();
        rlImGuiBegin();

        ClearBackground(RAYWHITE);
        ImGui::Begin("ImGui window");
        ImGui::SeparatorText("Rectangle");
        ImGui::DragFloat("size", &size, 1.0f, 100.0f, 800.0f);
        ImGui::End();

        DrawRectangle(100, 100, size, size, RED);
        rlImGuiEnd();
        EndDrawing();
    }
    rlImGuiShutdown();

    CloseWindow();
    return 0;
}

