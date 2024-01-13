#include "game.h"

#include "drawing.h"
#include "editor.h"
#include "resources.h"
#include "rlgl.h"
#include "scene.h"
#include <stdio.h>

static void draw_game(void);

void start_game(void) {
    load_drawing();
    load_resources();
    load_scene();
    load_editor();

    Entity* golova = create_texture_sprite_entity("resources/textures/golova.png");
    golova->transform.translation.x += 4.0;

    while (!WindowShouldClose()) {
        // ---------------------------------------------------------------
        // Update
        update_editor();
        update_scene();

        // ---------------------------------------------------------------
        // Drawing

        // Draw editor
        BeginTextureMode(*FULL_SCREEN);
        ClearBackground(DARKGRAY);
        draw_editor();
        EndTextureMode();

        // Draw scene preview
        BeginTextureMode(*THIRD_SCREEN);
        ClearBackground(BLACK);
        draw_game();
        EndTextureMode();

        // Blit screen
        BeginDrawing();
        ClearBackground(BLANK);
        draw_screen(FULL_SCREEN);
        draw_screen_top_right(THIRD_SCREEN);
        EndDrawing();
    }

    unload_editor();
    unload_scene();
    unload_resources();
    unload_drawing();
}

static void draw_game(void) {
    rlEnableBackfaceCulling();

    BeginMode3D(SCENE.camera_shell.camera);
    draw_scene();
    EndMode3D();
}
