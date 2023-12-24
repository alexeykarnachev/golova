#include "raylib.h"
#include "raymath.h"
#include "rcamera.h"
#include <rlgl.h>
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

int main(void) {

    const int screen_width = 1024;
    const int screen_height = 768;
    InitWindow(screen_width, screen_height, "Gizmo");
    SetTargetFPS(60);

    Camera3D camera;
    camera.fovy = 45.0f;
    camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    camera.position = (Vector3){5.0f, 5.0f, 5.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.projection = CAMERA_PERSPECTIVE;

    Shader shader = LoadShader("resources/shaders/gizmo.vert", "resources/shaders/gizmo.frag");
    int camera_pos_loc = GetShaderLocation(shader, "cameraPosition");
    int gizmo_pos_loc = GetShaderLocation(shader, "gizmoPosition");
 
    Vector3 center = {0};
    float size = 10.0;

    while (!WindowShouldClose()) {
        float radius = Vector3Distance(camera.position, center) / size;
        camera_update(&camera);

        BeginDrawing();
            ClearBackground(DARKGRAY);

            BeginMode3D(camera);
                rlSetLineWidth(1.0);
                DrawGrid(100.0, 1.0);
            EndMode3D();

            BeginMode3D(camera);
                rlSetLineWidth(2.0);
                DrawLine3D((Vector3){-50.0f, 0.0f, 0.0f}, (Vector3){50.0f, 0.0f, 0.0f}, RED);
                DrawLine3D((Vector3){0.0f, -50.0f, 0.0f}, (Vector3){0.0f, 50.0f, 0.0f}, GREEN);
                DrawLine3D((Vector3){0.0f, 0.0f, -50.0f}, (Vector3){0.0f, 0.0f, 50.0f}, DARKBLUE);
            EndMode3D();

            BeginMode3D(camera);
                rlSetLineWidth(4.0);
                BeginShaderMode(shader);
                    SetShaderValue(shader, camera_pos_loc, &camera.position, SHADER_UNIFORM_VEC3);
                    SetShaderValue(shader, gizmo_pos_loc, &center, SHADER_UNIFORM_VEC3);
                    DrawCircle3D(center, radius, (Vector3){0.0, 1.0, 0.0}, 90.0, RED);
                    DrawCircle3D(center, radius, (Vector3){1.0, 0.0, 0.0}, 90.0, GREEN);
                    DrawCircle3D(center, radius, (Vector3){1.0, 0.0, 0.0}, 0.0, BLUE);
                EndShaderMode();
            EndMode3D();
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
