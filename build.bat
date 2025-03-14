del .\main.exe

rem -Wextra
g++ -g -Wall main.cpp -o main.exe -I./rlImGui/src -I ./rlImGui/imgui -I ./rlImGui/imgui/backends -I ./raylib/src -L./rlImGui/src -L./raylib/src -lrlImGui -lraylib -lgdi32 -lwinmm

.\main.exe


rem del .\trayimg.exe

rem rem -Wextra
rem g++ -g -Wall trayimg.cpp -o trayimg.exe -I./rlImGui/src -I ./rlImGui/imgui -I ./rlImGui/imgui/backends -I ./raylib/src -L./rlImGui/src -L./raylib/src -lrlImGui -lraylib -lgdi32 -lwinmm

rem .\trayimg.exe
