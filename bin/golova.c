#include "../src/scene.h"
#include "raylib.h"

// #define SCREEN_WIDTH 1024
// #define SCREEN_HEIGHT 768
#define SCREEN_WIDTH 2560
#define SCREEN_HEIGHT 1440

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Editor");
    SetTargetFPS(60);

    load_scene("resources/scenes/tmp.scn");

    while (!WindowShouldClose()) {
        update_scene();

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode3D(SCENE.camera);
        draw_scene();
        EndMode3D();
        EndDrawing();
    }

    return 0;
}
