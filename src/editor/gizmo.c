#include "gizmo.h"
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <stdio.h>
#include "../math.h"


static Shader shader;
bool IS_SHADER_LOADED = false;
int CAMERA_POS_LOC = 0;
int GIZMO_POS_LOC = 0;

static float get_gizmo_radius(Camera3D camera, Vector3 position) {
    return Vector3Distance(camera.position, position) / GIZMO_SIZE;
}

Matrix gizmo_update(Gizmo *gizmo, Camera3D camera, Vector3 *position) {

    if (!IsMouseButtonDown(0)) {
        gizmo->state = GIZMO_COLD;
        gizmo->active_axis = Vector3Zero();
    }

    Matrix transform = MatrixIdentity();
    float radius = get_gizmo_radius(camera, *position);
    Ray mouse_ray = GetMouseRay(GetMousePosition(), camera);
    RayCollision mouse_ray_collision = GetRayCollisionSphere(mouse_ray, *position, radius);
    if (gizmo->state != GIZMO_ACTIVE_ROT && mouse_ray_collision.hit) {  
        if (fabsf((mouse_ray_collision.point.x - position->x) / radius) < GIZMO_ROT_CIRCLE_THICKNESS) {
            gizmo->active_axis = (Vector3){1.0, 0.0, 0.0};
            gizmo->state = GIZMO_HOT_ROT;
        } else if (fabsf((mouse_ray_collision.point.y - position->y) / radius) < GIZMO_ROT_CIRCLE_THICKNESS) {
            gizmo->active_axis = (Vector3){0.0, 1.0, 0.0};
            gizmo->state = GIZMO_HOT_ROT;
        } else if (fabsf((mouse_ray_collision.point.z - position->z) / radius) < GIZMO_ROT_CIRCLE_THICKNESS) {
            gizmo->active_axis = (Vector3){0.0, 0.0, 1.0};
            gizmo->state = GIZMO_HOT_ROT;
        }

        if (gizmo->state == GIZMO_HOT_ROT && IsMouseButtonDown(0)) {
            gizmo->state = GIZMO_ACTIVE_ROT;
        }
    } else if (gizmo->state == GIZMO_ACTIVE_ROT) {
        Vector2 rot_center = GetWorldToScreen(*position, camera);
        Vector2 p1 = Vector2Subtract(GetMousePosition(), rot_center);
        Vector2 p0 = Vector2Subtract(p1, GetMouseDelta());
        float angle = vec2_angle(p1, p0);
        transform = MatrixRotate(gizmo->active_axis, angle);
    }

    return transform;
}

void gizmo_draw(Gizmo *gizmo, Camera3D camera, Vector3 position) {
    if (!IS_SHADER_LOADED) {
        shader = LoadShader("resources/shaders/gizmo.vert", "resources/shaders/gizmo.frag");
        CAMERA_POS_LOC = GetShaderLocation(shader, "cameraPosition");
        GIZMO_POS_LOC = GetShaderLocation(shader, "gizmoPosition");
        IS_SHADER_LOADED = true;
    }

    float radius = get_gizmo_radius(camera, position);

    Color x_color = gizmo->active_axis.x == 1.0 ? WHITE : RED;
    Color y_color = gizmo->active_axis.y == 1.0 ? WHITE : GREEN;
    Color z_color = gizmo->active_axis.z == 1.0 ? WHITE : BLUE;

    BeginMode3D(camera);
        rlSetLineWidth(GIZMO_ROT_CIRCLE_DRAW_THICKNESS);
        rlDisableDepthTest();

        BeginShaderMode(shader);
            SetShaderValue(shader, CAMERA_POS_LOC, &camera.position, SHADER_UNIFORM_VEC3);
            SetShaderValue(shader, GIZMO_POS_LOC, &position, SHADER_UNIFORM_VEC3);
            DrawCircle3D(position, radius, (Vector3){0.0, 1.0, 0.0}, 90.0, x_color);
            DrawCircle3D(position, radius, (Vector3){1.0, 0.0, 0.0}, 90.0, y_color);
            DrawCircle3D(position, radius, (Vector3){1.0, 0.0, 0.0}, 0.0, z_color);
        EndShaderMode();
    EndMode3D();

    if (gizmo->state == GIZMO_ACTIVE_ROT) {
        rlSetLineWidth(GIZMO_ROT_HANDLE_DRAW_THICKNESS);
        Vector2 p0 = GetWorldToScreen(position, camera);
        Vector2 p1 = GetMousePosition();
        DrawLineV(p0, p1, WHITE);
    }
}
