#include "game.h"

#include "drawing.h"
#include "raylib.h"
#include "resources.h"

Camera3D GAME_CAMERA;
Camera3D EDITOR_CAMERA;

void start_game(void) {
    load_drawing();
    load_resources();

    GAME_CAMERA.fovy = 60.0;
    EDITOR_CAMERA = GAME_CAMERA;
    EDITOR_CAMERA.position = (Vector3){5.0, 5.0, 5.0};

    while (!WindowShouldClose()) {
        // Draw scene
        BeginTextureMode(*FULL_SCREEN);
        BeginMode3D(EDITOR_CAMERA);

        draw_entities();

        EndMode3D();
        EndTextureMode();

        // Blit screen
        BeginDrawing();
        draw_screen(FULL_SCREEN);
        EndDrawing();
    }

    unload_resources();
    unload_drawing();
}
