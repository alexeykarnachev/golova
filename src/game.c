#include "game.h"

#include "drawing.h"
#include "entities.h"
#include "raylib.h"
#include "resources.h"

Camera3D GAME_CAMERA;
Camera3D EDITOR_CAMERA;

void start_game(void) {
    load_drawing();
    load_resources();

    GAME_CAMERA.fovy = 60.0;
    GAME_CAMERA.projection = CAMERA_PERSPECTIVE;
    GAME_CAMERA.up = (Vector3){0.0, 1.0, 0.0};
    EDITOR_CAMERA = GAME_CAMERA;
    EDITOR_CAMERA.position = (Vector3){5.0, 5.0, 5.0};

    Entity* golova = create_sprite_entity("resources/textures/golova.png");

    while (!WindowShouldClose()) {
        // Draw scene
        BeginTextureMode(*FULL_SCREEN);
        ClearBackground(DARKGRAY);
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
