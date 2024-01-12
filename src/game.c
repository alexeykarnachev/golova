#include "game.h"

#include "drawing.h"
#include "editor.h"
#include "resources.h"
#include "scene.h"
#include <stdio.h>

void start_game(void) {
    load_drawing();
    load_resources();
    load_scene();
    load_editor();

    Entity* golova = create_texture_sprite_entity(
        "resources/textures/golova.png"
    );
    Entity* ground = create_shader_sprite_entity("resources/shaders/ground.frag"
    );
    golova->transform.translation.x += 4.0;

    while (!WindowShouldClose()) {
        // ---------------------------------------------------------------
        // Update
        update_editor();
        update_scene();

        // ---------------------------------------------------------------
        // Drawing
        // Draw scene
        BeginTextureMode(*FULL_SCREEN);
        ClearBackground(DARKGRAY);
        draw_editor();
        EndTextureMode();

        // Blit screen
        BeginDrawing();
        draw_screen(FULL_SCREEN);
        EndDrawing();
    }

    unload_editor();
    unload_scene();
    unload_resources();
    unload_drawing();
}
