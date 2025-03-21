@echo off
del .\rolling-cube.exe
@echo on
g++ -g -Wall rolling-cube.cpp -o rolling-cube.exe -I./rlImGui/src -I ./rlImGui/imgui -I ./rlImGui/imgui/backends -I ./raylib/src -L./rlImGui/src -L./raylib/src -lrlImGui -lraylib -lgdi32 -lwinmm
@echo off
.\rolling-cube.exe
rem -Wextra

rem del .\main.exe
rem g++ -g -Wall main.cpp -o main.exe -I./rlImGui/src -I ./rlImGui/imgui -I ./rlImGui/imgui/backends -I ./raylib/src -L./rlImGui/src -L./raylib/src -lrlImGui -lraylib -lgdi32 -lwinmm
rem .\main.exe


rem del .\trayimg.exe
rem g++ -g -Wall trayimg.cpp -o trayimg.exe -I./rlImGui/src -I ./rlImGui/imgui -I ./rlImGui/imgui/backends -I ./raylib/src -L./rlImGui/src -L./raylib/src -lrlImGui -lraylib -lgdi32 -lwinmm
rem .\trayimg.exe
