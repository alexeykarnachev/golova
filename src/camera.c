#include "raylib.h"
#include "raymath.h"
#include "rcamera.h"

#define EDITOR_CAMERA_ROT_SPEED 0.003f
#define EDITOR_CAMERA_MOVE_SPEED 0.01f
#define EDITOR_CAMERA_ZOOM_SPEED 1.0f

void update_editor_camera(Camera3D* camera) {
    bool is_mmb_down = IsMouseButtonDown(2);
    bool is_shift_down = IsKeyDown(KEY_LEFT_SHIFT);
    Vector2 mouse_delta = GetMouseDelta();

    // Shift + MMB + mouse move -> change the camera position in the
    // right-direction plane
    if (is_mmb_down && is_shift_down) {
        CameraMoveRight(
            camera, -EDITOR_CAMERA_MOVE_SPEED * mouse_delta.x, true
        );

        Vector3 right = GetCameraRight(camera);
        Vector3 up = Vector3CrossProduct(
            Vector3Subtract(camera->position, camera->target), right
        );
        up = Vector3Scale(
            Vector3Normalize(up), EDITOR_CAMERA_MOVE_SPEED * mouse_delta.y
        );
        camera->position = Vector3Add(camera->position, up);
        camera->target = Vector3Add(camera->target, up);
        // Rotate the camera around the look-at point
    } else if (is_mmb_down) {
        CameraYaw(camera, -EDITOR_CAMERA_ROT_SPEED * mouse_delta.x, true);
        CameraPitch(
            camera, EDITOR_CAMERA_ROT_SPEED * mouse_delta.y, true, true, false
        );
    }

    // Bring camera closer (or move away), to the look-at point
    CameraMoveToTarget(camera, -GetMouseWheelMove() * EDITOR_CAMERA_ZOOM_SPEED);
}
