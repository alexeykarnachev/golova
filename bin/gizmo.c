#include "raylib.h"
#include "raymath.h"
#include "rcamera.h"
#include <math.h>
#include <rlgl.h>
#include <stddef.h>
#include <stdio.h>

#include "../src/editor/gizmo.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#undef RAYGUI_IMPLEMENTATION


void camera_update(Camera3D *camera) {
    float rot_speed = 0.15f;
    float move_speed = 0.5f;
    float zoom_speed = 50.0f;
    float dt = GetFrameTime();

    bool is_middle = IsMouseButtonDown(2);
    bool is_shift = IsKeyDown(KEY_LEFT_SHIFT);
    Vector2 mouse_delta = GetMouseDelta();

    if (is_middle && is_shift) {
        CameraMoveRight(camera, -move_speed * dt * mouse_delta.x, true);

        Vector3 right = GetCameraRight(camera);
        Vector3 up = Vector3CrossProduct(Vector3Subtract(camera->position, camera->target), right);
        up = Vector3Scale(Vector3Normalize(up), move_speed * dt * mouse_delta.y);
        camera->position = Vector3Add(camera->position, up);
        camera->target = Vector3Add(camera->target, up);
    } else if (is_middle) {
        CameraYaw(camera, -rot_speed * dt * mouse_delta.x, true);
        CameraPitch(camera, rot_speed * dt * mouse_delta.y, true, true, false);
    }

    CameraMoveToTarget(camera, -GetMouseWheelMove() * dt * zoom_speed);
}

Model golova_create() {
    Texture texture = LoadTexture("resources/golova.png");
    float texture_aspect = (float)texture.width / texture.height;

    Model model = LoadModelFromMesh(GenMeshPlane(texture_aspect, 1.0, 2, 2));
    model.materials[0].maps[0].texture = texture;
    model.materials[0].shader = LoadShader(0, "resources/shaders/sprite.frag");

    return model;
}

void editor_draw_grid() {
    DrawGrid(100.0, 1.0);
    DrawLine3D((Vector3){-50.0f, 0.0f, 0.0f}, (Vector3){50.0f, 0.0f, 0.0f}, RED);
    DrawLine3D((Vector3){0.0f, -50.0f, 0.0f}, (Vector3){0.0f, 50.0f, 0.0f}, GREEN);
    DrawLine3D((Vector3){0.0f, 0.0f, -50.0f}, (Vector3){0.0f, 0.0f, 50.0f}, DARKBLUE);
}

int main(void) {

    const int screen_width = 1024;
    const int screen_height = 768;
    InitWindow(screen_width, screen_height, "Golova");
    rlEnableDepthTest();
    SetTargetFPS(60);

    GuiLoadStyleDefault();

    Camera3D camera;
    camera.fovy = 45.0f;
    camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    camera.position = (Vector3){5.0f, 5.0f, 5.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.projection = CAMERA_PERSPECTIVE;

    Shader ground_shader = LoadShader(0, "resources/shaders/ground.frag");
    Model golova = golova_create();

    float xr = 0.0;
    float yr = 0.0;
    float zr = 0.0;

    float xt = 0.0;
    float yt = 0.0;
    float zt = 0.0;

    while (!WindowShouldClose()) {
        camera_update(&camera);

        // ---------------------------------------------------------------
        // Draw scene
        BeginDrawing();
        ClearBackground(DARKGRAY);

        BeginMode3D(camera);
            editor_draw_grid();
            DrawModel(golova, (Vector3){0.0, 0.0, 0.0}, 1.0, WHITE);
        EndMode3D();


        BoundingBox bb = GetMeshBoundingBox(golova.meshes[0]);

        GuiDrawRectangle((Rectangle){20, 20, 340, 500}, 2, BLANK, RAYWHITE);
        GuiSlider((Rectangle){ 30, 30, 300, 15 }, NULL, TextFormat("%0.2f", xt), &xt, -1.0f, 1.0f);
        GuiSlider((Rectangle){ 30, 50, 300, 15 }, NULL, TextFormat("%0.2f", yt), &yt, -1.0f, 1.0f);
        GuiSlider((Rectangle){ 30, 70, 300, 15 }, NULL, TextFormat("%0.2f", zt), &zt, -1.0f, 1.0f);

        GuiSlider((Rectangle){ 30, 100, 300, 15 }, NULL, TextFormat("%0.2f", xr), &xr, -90.0f, 90.0f);
        GuiSlider((Rectangle){ 30, 120, 300, 15 }, NULL, TextFormat("%0.2f", yr), &yr, -90.0f, 90.0f);
        GuiSlider((Rectangle){ 30, 140, 300, 15 }, NULL, TextFormat("%0.2f", zr), &zr, -90.0f, 90.0f);

        Matrix t = MatrixTranslate(xt, yt, zt);
        Matrix r = MatrixRotateXYZ((Vector3){ DEG2RAD*xr, DEG2RAD*yr, DEG2RAD*zr });
        golova.transform = MatrixMultiply(r, t);

        EndDrawing();
    }

    UnloadShader(ground_shader);
    UnloadModel(golova);
    CloseWindow();

    return 0;
}
