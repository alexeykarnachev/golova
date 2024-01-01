#include <stdio.h>

#define RAYGIZMO_IMPLEMENTATION
#include "../src/camera.h"
#include "../src/picking.h"
#include "../src/resources.h"
#include "../src/scene.h"
#include "raygizmo.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#define MAX_N_MODELS 256

Camera3D EDITOR_CAMERA;
Camera3D GAME_CAMERA;
RenderTexture EDITOR_SCREEN;
RenderTexture GAME_SCREEN;
Model* MODELS[MAX_N_MODELS];
size_t N_MODELS = 0;

static int PICKED_MODEL_ID = -1;
static Vector3 PICKED_MODEL_POSITION;

static void draw_scene(void) {
    for (size_t i = 0; i < N_MODELS; ++i) {
        DrawModel(*MODELS[i], (Vector3){0.0, 0.0, 0.0}, 1.0, WHITE);
    }
}

static void draw_editor(void) {
    BeginTextureMode(EDITOR_SCREEN);
    {
        ClearBackground(DARKGRAY);
        BeginMode3D(EDITOR_CAMERA);
        {
            draw_scene();
            rlSetLineWidth(1.0);
            DrawGrid(100.0, 1.0);

            rlSetLineWidth(2.0);
            DrawLine3D(
                (Vector3){-50.0f, 0.0f, 0.0f}, (Vector3){50.0f, 0.0f, 0.0f}, RED
            );
            DrawLine3D(
                (Vector3){0.0f, -50.0f, 0.0f},
                (Vector3){0.0f, 50.0f, 0.0f},
                GREEN
            );
            DrawLine3D(
                (Vector3){0.0f, 0.0f, -50.0f},
                (Vector3){0.0f, 0.0f, 50.0f},
                DARKBLUE
            );
        }
        EndMode3D();

        if (PICKED_MODEL_ID != -1) {
            drawGizmo(EDITOR_CAMERA, PICKED_MODEL_POSITION);
        }
    }
    EndTextureMode();
}

static void draw_game(void) {
    BeginTextureMode(GAME_SCREEN);
    {
        ClearBackground(BLACK);
        BeginMode3D(GAME_CAMERA);
        {
            draw_scene();
            //
        }
        EndMode3D();
    }
    EndTextureMode();
}

static void blit_screens(void) {
    BeginDrawing();
    {
        ClearBackground(BLACK);
        Rectangle editor_rect = {
            0, 0, EDITOR_SCREEN.texture.width, -EDITOR_SCREEN.texture.height};
        DrawTextureRec(
            EDITOR_SCREEN.texture, editor_rect, (Vector2){0, 0}, WHITE
        );

        Rectangle game_rect = {
            0, 0, GAME_SCREEN.texture.width, -GAME_SCREEN.texture.height};
        DrawTextureRec(GAME_SCREEN.texture, game_rect, (Vector2){0, 0}, WHITE);
    }
    EndDrawing();
}

static void update_editor(void) {
    update_editor_camera(&EDITOR_CAMERA);

    int picked_model_id = PICKED_MODEL_ID;
    if (IsMouseButtonReleased(0)) {
        BeginDrawing();
        {
            BeginMode3D(EDITOR_CAMERA);
            { picked_model_id = pick_model(MODELS, 2, GetMousePosition()); }
            EndMode3D();
        }
        EndDrawing();
    }

    if (picked_model_id == -1 && gizmoState == GIZMO_COLD) {
        PICKED_MODEL_ID = -1;
    } else if (picked_model_id != -1) {
        PICKED_MODEL_ID = picked_model_id;
    }

    if (PICKED_MODEL_ID != -1) {
        Matrix* transform = &MODELS[PICKED_MODEL_ID]->transform;
        PICKED_MODEL_POSITION = (Vector3
        ){transform->m12, transform->m13, transform->m14};
        Matrix gizmo_transform = updateGizmo(
            EDITOR_CAMERA, PICKED_MODEL_POSITION
        );
        *transform = MatrixMultiply(*transform, gizmo_transform);
    }
}

int main(void) {
    const int screen_width = 2560;
    const int screen_height = 1440;
    // const int screen_width = 1024;
    // const int screen_height = 768;

    // Window
    InitWindow(screen_width, screen_height, "scene_example");
    SetTargetFPS(60);

    // Editor Camera
    EDITOR_CAMERA.fovy = 45.0f;
    EDITOR_CAMERA.target = (Vector3){0.0f, 0.0f, 0.0f};
    EDITOR_CAMERA.position = (Vector3){5.0f, 5.0f, 5.0f};
    EDITOR_CAMERA.up = (Vector3){0.0f, 1.0f, 0.0f};
    EDITOR_CAMERA.projection = CAMERA_PERSPECTIVE;

    // Scene Camera
    GAME_CAMERA.fovy = 45.0f;
    GAME_CAMERA.target = (Vector3){0.0f, 0.0f, 0.0f};
    GAME_CAMERA.position = (Vector3){0.0f, 5.0f, 5.0f};
    GAME_CAMERA.up = (Vector3){0.0f, 1.0f, 0.0f};
    GAME_CAMERA.projection = CAMERA_PERSPECTIVE;

    // Screens
    EDITOR_SCREEN = LoadRenderTexture(screen_width, screen_height);
    GAME_SCREEN = LoadRenderTexture(screen_width / 3, screen_height / 3);

    // Scene
    MODELS[N_MODELS++] = &SCENE.golova.model;
    MODELS[N_MODELS++] = &SCENE.ground.model;

    load_resources();
    load_scene();
    load_picking();
    loadGizmo();

    // Main loop
    while (!WindowShouldClose()) {
        update_editor();
        draw_editor();
        draw_game();
        blit_screens();
    }

    // Unload resources
    UnloadRenderTexture(EDITOR_SCREEN);
    UnloadRenderTexture(GAME_SCREEN);

    unload_resources();
    unload_picking();
    unloadGizmo();

    CloseWindow();
}
