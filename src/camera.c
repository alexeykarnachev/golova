#include "camera.h"

#include "raylib.h"
#include "raymath.h"
#include "rcamera.h"

void update_orbital_camera(
    Camera3D* camera, bool is_mmb_down, bool is_shift_down, Vector2 mouse_delta
) {
    static float rot_speed = 0.003f;
    static float move_speed = 0.01f;
    static float zoom_speed = 1.0f;

    if (is_mmb_down && is_shift_down) {
        // Shift + MMB + mouse move -> change the camera position in the
        // right-direction plane
        CameraMoveRight(camera, -move_speed * mouse_delta.x, true);

        Vector3 right = GetCameraRight(camera);
        Vector3 up = Vector3CrossProduct(
            Vector3Subtract(camera->position, camera->target), right
        );
        up = Vector3Scale(Vector3Normalize(up), move_speed * mouse_delta.y);
        camera->position = Vector3Add(camera->position, up);
        camera->target = Vector3Add(camera->target, up);
    } else if (is_mmb_down) {
        // Rotate the camera around the look-at point
        CameraYaw(camera, -rot_speed * mouse_delta.x, true);
        CameraPitch(camera, rot_speed * mouse_delta.y, true, true, false);
    }

    // Bring camera closer (or move away), to the look-at point
    CameraMoveToTarget(camera, -GetMouseWheelMove() * zoom_speed);
}
