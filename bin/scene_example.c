#define RAYGIZMO_IMPLEMENTATION
#include "../src/camera.h"
#include "../src/resources.h"
#include "../src/scene.h"
#include "raygizmo.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

static void draw_editor_grid(void) {
    rlSetLineWidth(1.0);
    DrawGrid(100.0, 1.0);

    rlSetLineWidth(2.0);
    DrawLine3D(
        (Vector3){-50.0f, 0.0f, 0.0f}, (Vector3){50.0f, 0.0f, 0.0f}, RED
    );
    DrawLine3D(
        (Vector3){0.0f, -50.0f, 0.0f}, (Vector3){0.0f, 50.0f, 0.0f}, GREEN
    );
    DrawLine3D(
        (Vector3){0.0f, 0.0f, -50.0f}, (Vector3){0.0f, 0.0f, 50.0f}, DARKBLUE
    );
}

static void draw_scene(void) {
    DrawModel(SCENE.golova.model, (Vector3){0.0, 0.0, 0.0}, 1.0, WHITE);
}

int main(void) {
    const int screenWidth = 2560 / 2;
    const int screenHeight = 1440 / 2;

    // Window
    InitWindow(screenWidth, screenHeight, "scene_example");
    SetTargetFPS(60);

    // Editor Camera
    Camera3D editor_camera;
    editor_camera.fovy = 45.0f;
    editor_camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    editor_camera.position = (Vector3){5.0f, 5.0f, 5.0f};
    editor_camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    editor_camera.projection = CAMERA_PERSPECTIVE;

    // Scene Camera
    Camera3D scene_camera;
    scene_camera.fovy = 45.0f;
    scene_camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    scene_camera.position = (Vector3){5.0f, 5.0f, 5.0f};
    scene_camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    scene_camera.projection = CAMERA_PERSPECTIVE;

    // Screens
    RenderTexture editor_screen = LoadRenderTexture(screenWidth, screenHeight);
    RenderTexture scene_screen = LoadRenderTexture(
        screenWidth / 5, screenHeight / 5
    );
    Rectangle editor_rect = {
        0, 0, editor_screen.texture.width, -editor_screen.texture.height};
    Rectangle scene_rect = {
        0, 0, scene_screen.texture.width, -scene_screen.texture.height};

    // Scene
    load_resources();
    load_scene();

    loadGizmo();
    // Main loop
    while (!WindowShouldClose()) {
        update_editor_camera(&editor_camera);

        Matrix* transform = &SCENE.golova.model.transform;
        Vector3 position = {transform->m12, transform->m13, transform->m14};
        Matrix gizmo_transform = updateGizmo(editor_camera, position);
        *transform = MatrixMultiply(*transform, gizmo_transform);

        rlEnableDepthTest();

        BeginTextureMode(editor_screen);
        {
            ClearBackground(DARKGRAY);
            BeginMode3D(editor_camera);
            {
                draw_scene();
                draw_editor_grid();
            }
            EndMode3D();
            drawGizmo(editor_camera, position);
        }
        EndTextureMode();

        BeginTextureMode(scene_screen);
        {
            ClearBackground(BLACK);
            BeginMode3D(scene_camera);
            {
                draw_scene();
                //
            }
            EndMode3D();
        }
        EndTextureMode();

        BeginDrawing();
        {
            ClearBackground(BLACK);
            DrawTextureRec(
                editor_screen.texture, editor_rect, (Vector2){0, 0}, WHITE
            );
            DrawTextureRec(
                scene_screen.texture, scene_rect, (Vector2){0, 0}, WHITE
            );
        }
        EndDrawing();
    }

    // Unload resources
    UnloadRenderTexture(editor_screen);
    UnloadRenderTexture(scene_screen);

    unload_resources();
    unloadGizmo();

    CloseWindow();
}
