del .\trayimg.exe

rem -Wextra
g++ -g -Wall trayimg.cpp -o trayimg.exe -I./rlImGui/src -I ./rlImGui/imgui -I ./rlImGui/imgui/backends -I ./raylib/src -L./rlImGui/src -L./raylib/src -lrlImGui -lraylib -lgdi32 -lwinmm

.\trayimg.exe
