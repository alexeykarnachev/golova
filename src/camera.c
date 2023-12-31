#include "raylib.h"
#include "rcamera.h"
#include "raymath.h"


#define EDITOR_CAMERA_ROT_SPEED 0.003f
#define EDITOR_CAMERA_MOVE_SPEED 0.01f
#define EDITOR_CAMERA_ZOOM_SPEED 1.0f


void updateEditorCamera(Camera3D* camera) {
    bool isMMBDown = IsMouseButtonDown(2);
    bool isShiftDown = IsKeyDown(KEY_LEFT_SHIFT);
    Vector2 mouseDelta = GetMouseDelta();

    // Shift + MMB + mouse move -> change the camera position in the right-direction plane
    if (isMMBDown && isShiftDown) {
        CameraMoveRight(camera, -EDITOR_CAMERA_MOVE_SPEED * mouseDelta.x, true);

        Vector3 right = GetCameraRight(camera);
        Vector3 up = Vector3CrossProduct(
            Vector3Subtract(camera->position, camera->target), right
        );
        up = Vector3Scale(Vector3Normalize(up), EDITOR_CAMERA_MOVE_SPEED * mouseDelta.y);
        camera->position = Vector3Add(camera->position, up);
        camera->target = Vector3Add(camera->target, up);
    // Rotate the camera around the look-at point
    } else if (isMMBDown) {
        CameraYaw(camera, -EDITOR_CAMERA_ROT_SPEED * mouseDelta.x, true);
        CameraPitch(camera, EDITOR_CAMERA_ROT_SPEED * mouseDelta.y, true, true, false);
    }

    // Bring camera closer (or move away), to the look-at point 
    CameraMoveToTarget(camera, -GetMouseWheelMove() * EDITOR_CAMERA_ZOOM_SPEED);
}
