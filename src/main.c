#include "raylib.h"
#include "raymath.h"
#include "raygui.h"
#include "rcamera.h"
#include <math.h>
#include <rlgl.h>
#include <stddef.h>
#include <stdio.h>

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

    // setup initial camera data
    Camera3D camera;

    camera.fovy = 45.0f;
    camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    camera.position = (Vector3){5.0f, 5.0f, 5.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.projection = CAMERA_PERSPECTIVE;

    Vector2 cursorPos = GetMousePosition(); // save off current position so we have a start point


    Shader ground_shader = LoadShader(0, "resources/shaders/ground.frag");

    Model golova = golova_create();


    while (!WindowShouldClose()) {
        camera_update(&camera);

        // ---------------------------------------------------------------
        // Draw scene
        BeginDrawing();
        ClearBackground(DARKGRAY);

        BeginMode3D(camera);
            editor_draw_grid();
            DrawModel(golova, (Vector3){0.0, 0.0, 0.0}, 1.0, WHITE);

            // -----------------------------------------------------------
            // Draw ground:
            // BeginShaderMode(ground_shader);
            // rlPushMatrix();
            //     rlTranslatef(0.0f, -5.0f, 0.0f);
            //     rlRotatef(-45.0f, 1.0f, 0.0f, 0.0f);
            //     rlScalef(5.0, 6.0f, 1.0f);
            //     rlBegin(RL_QUADS);
            //         rlColor4ub(255, 0, 0, 255);
            //         rlTexCoord2f(0.0f, 0.0f); rlVertex3f(-1.0, -1.0, 0.0);  // BL
            //         rlTexCoord2f(1.0f, 0.0f); rlVertex3f(1.0, -1.0, 0.0);  // BR
            //         rlTexCoord2f(1.0f, 1.0f); rlVertex3f(1.0, 1.0, 0.0);  // TR
            //         rlTexCoord2f(0.0f, 1.0f); rlVertex3f(-1.0, 1.0, 0.0);  // TL
            //     rlEnd();
            // rlPopMatrix();
            // EndShaderMode();
        
        EndMode3D();

        // DrawText("TEST", 640, 10, 20, RED);

        EndDrawing();
    }

    UnloadShader(ground_shader);
    UnloadModel(golova);
    CloseWindow();

    return 0;
}
