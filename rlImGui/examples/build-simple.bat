rem simple.cpp
g++ ./simple.cpp -DDEBUG -DPLATFORM_DESKTOP -DGRAPHICS_API_OPENGL_33 -I../src -I../../raylib/src -I../imgui -std=c++17  -o ./simple.exe -L../src -L../../raylib/src -lrlImGui -lraylib -lwinmm -lgdi32 -m64 -g -std=c++17
