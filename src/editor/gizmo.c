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

void gizmo_update(Gizmo *gizmo, Camera3D camera, Vector3 *position, Vector3 *rotation) {

    if (!IsMouseButtonDown(0)) {
        gizmo->state = GIZMO_IDLE;
    }

    float radius = get_gizmo_radius(camera, *position);
    Ray mouse_ray = GetMouseRay(GetMousePosition(), camera);
    RayCollision mouse_ray_collision = GetRayCollisionSphere(mouse_ray, *position, radius);
    if (gizmo->state != GIZMO_ROT && mouse_ray_collision.hit) {
        if (fabsf(mouse_ray_collision.point.x / radius) < GIZMO_ROT_HANDLE_THICKNESS) {
            gizmo->active_axis = GIZMO_AXIS_X;
            gizmo->state = GIZMO_HOVER_ROT;
            gizmo->start_angle = rotation->x;
        } else if (fabsf(mouse_ray_collision.point.y / radius) < GIZMO_ROT_HANDLE_THICKNESS) {
            gizmo->active_axis = GIZMO_AXIS_Y;
            gizmo->state = GIZMO_HOVER_ROT;
            gizmo->start_angle = rotation->y;
        } else if (fabsf(mouse_ray_collision.point.z / radius) < GIZMO_ROT_HANDLE_THICKNESS) {
            gizmo->active_axis = GIZMO_AXIS_Z;
            gizmo->state = GIZMO_HOVER_ROT;
            gizmo->start_angle = rotation->z;
        }


        if (gizmo->state == GIZMO_HOVER_ROT && IsMouseButtonDown(0)) {
            gizmo->state = GIZMO_ROT;
            gizmo->start_rot_handle = Vector3Subtract(mouse_ray_collision.point, *position);
        }
    } else if (gizmo->state == GIZMO_ROT) {
        printf("rotation!\n");
        // RayCollision collision = get_ray_collision_plane(mouse_ray, position, rotation_axis);
        // if (collision.hit) {
        //     Vector3 rotation_handle_end = Vector3Subtract(collision.point, center);
        //     *current_angle = start_angle + Vector3Angle(rotation_handle_start, rotation_handle_end); 
        //     printf("%f\n", *current_angle);
        // }
    }
}

void gizmo_draw(Gizmo *gizmo, Camera3D camera, Vector3 position) {
    if (!IS_SHADER_LOADED) {
        shader = LoadShader("resources/shaders/gizmo.vert", "resources/shaders/gizmo.frag");
        CAMERA_POS_LOC = GetShaderLocation(shader, "cameraPosition");
        GIZMO_POS_LOC = GetShaderLocation(shader, "gizmoPosition");
        IS_SHADER_LOADED = true;
    }

    float radius = get_gizmo_radius(camera, position);

    Color x_color = RED;
    Color y_color = GREEN;
    Color z_color = BLUE;

    if (gizmo->state == GIZMO_HOVER_ROT) {
        if (gizmo->active_axis == GIZMO_AXIS_X) x_color = WHITE;
        else if (gizmo->active_axis == GIZMO_AXIS_Y) y_color = WHITE;
        else if (gizmo->active_axis == GIZMO_AXIS_Z) z_color = WHITE;
    }

    BeginMode3D(camera);
        rlSetLineWidth(GIZMO_ROT_HANDLE_DRAW_THICKNESS);
        rlDisableDepthTest();
        BeginShaderMode(shader);
            SetShaderValue(shader, CAMERA_POS_LOC, &camera.position, SHADER_UNIFORM_VEC3);
            SetShaderValue(shader, GIZMO_POS_LOC, &position, SHADER_UNIFORM_VEC3);
            DrawCircle3D(position, radius, (Vector3){0.0, 1.0, 0.0}, 90.0, x_color);
            DrawCircle3D(position, radius, (Vector3){1.0, 0.0, 0.0}, 90.0, y_color);
            DrawCircle3D(position, radius, (Vector3){1.0, 0.0, 0.0}, 0.0, z_color);
        EndShaderMode();
    EndMode3D();
}
