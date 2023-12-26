#include "raylib.h"
#include "raymath.h"
#include "rcamera.h"
#include <math.h>
#include <rlgl.h>
#include <stdio.h>

#define RAYGIZMO_IMPLEMENTATION
#include "../src/editor/gizmo.h"
#undef RAYGIZMO_IMPLEMENTATION

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

    Vector3 position = {1.0, 1.0, 1.0};
    Vector3 rotation = {0};
    Model cube = LoadModelFromMesh(GenMeshCube(1.0, 1.0, 1.0));
 
    while (!WindowShouldClose()) {
        BoundingBox bbox = GetMeshBoundingBox(cube.meshes[0]);
        Vector3 center = Vector3Scale(Vector3Add(bbox.min, bbox.max), 0.5);

        camera_update(&camera);

        BeginDrawing();
            ClearBackground(DARKGRAY);
            rlEnableDepthTest();

            BeginMode3D(camera);
                DrawModel(cube, position, 1.0, ORANGE);
            EndMode3D();

            Matrix transform = GizmoUpdate(camera, position);
            cube.transform = MatrixMultiply(cube.transform, transform);

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

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
