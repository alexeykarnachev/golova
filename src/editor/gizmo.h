#include <raylib.h>


#define GIZMO_SIZE 10.0f
#define GIZMO_ROT_CIRCLE_THICKNESS 0.1f
#define GIZMO_ROT_CIRCLE_DRAW_THICKNESS 4.0f
#define GIZMO_ROT_HANDLE_DRAW_THICKNESS 2.0f

#define X_AXIS (Vector3){1.0, 0.0, 0.0}
#define Y_AXIS (Vector3){0.0, 1.0, 0.0}
#define Z_AXIS (Vector3){0.0, 0.0, 1.0}

typedef enum GizmoState {
    GIZMO_COLD,

    GIZMO_HOT_ROT,
    GIZMO_ACTIVE_ROT,
} GizmoState;

Matrix GizmoUpdate(Camera3D camera, Vector3 position);
float Vector2WrapAngle(Vector2 v1, Vector2 v2);


#define RAYGIZMO_IMPLEMENTATION
#ifdef RAYGIZMO_IMPLEMENTATION

#include "raymath.h"
#include <math.h>
#include <rlgl.h>
#include <stdio.h>

const char *SHADER_ROT_HANDLE_COLOR_VERT = "\
#version 330\n\
in vec3 vertexPosition; \
in vec4 vertexColor; \
out vec4 fragColor; \
out vec3 fragPosition; \
uniform mat4 mvp; \
void main() { \
    fragColor = vertexColor; \
    fragPosition = vertexPosition; \
    gl_Position = mvp * vec4(vertexPosition, 1.0); \
} \
";

const char *SHADER_ROT_HANDLE_COLOR_FRAG = "\
#version 330\n\
in vec4 fragColor; \
in vec3 fragPosition; \
uniform vec3 cameraPosition; \
uniform vec3 gizmoPosition; \
out vec4 finalColor; \
void main() { \
    vec3 r = normalize(fragPosition - gizmoPosition); \
    vec3 c = normalize(fragPosition - cameraPosition); \
    if (dot(r, c) > 0.1) discard; \
    finalColor = fragColor; \
} \
";

GizmoState GIZMO_STATE;
Vector3 GIZMO_ACTIVE_AXIS;

static Shader SHADER_ROT_HANDLE;
static int SHADER_ROT_HANDLE_CAMERA_POS_LOC;
static int SHADER_ROT_HANDLE_GIZMO_POS_LOC;
static bool SHADER_ROT_HANDLE_LOADED;

Matrix GizmoUpdate(Camera3D camera, Vector3 position) {
    // -------------------------------------------------------------------
    // Initialize gizmo
    if (!SHADER_ROT_HANDLE_LOADED) {
        SHADER_ROT_HANDLE = LoadShaderFromMemory(SHADER_ROT_HANDLE_COLOR_VERT, SHADER_ROT_HANDLE_COLOR_FRAG);
        SHADER_ROT_HANDLE_CAMERA_POS_LOC = GetShaderLocation(SHADER_ROT_HANDLE, "cameraPosition");
        SHADER_ROT_HANDLE_GIZMO_POS_LOC = GetShaderLocation(SHADER_ROT_HANDLE, "gizmoPosition");
        SHADER_ROT_HANDLE_LOADED = true;
    }

    float radius = Vector3Distance(camera.position, position) / GIZMO_SIZE;
    float axis_handle_length = radius * 1.2;
    float axis_handle_tip_length = radius * 0.3;
    float axis_handle_tip_radius = radius * 0.1;

    Vector3 axis_handle_tip_start_x = Vector3Add(position, Vector3Scale(X_AXIS, axis_handle_length));
    Vector3 axis_handle_tip_start_y = Vector3Add(position, Vector3Scale(Y_AXIS, axis_handle_length));
    Vector3 axis_handle_tip_start_z = Vector3Add(position, Vector3Scale(Z_AXIS, axis_handle_length));

    Vector3 axis_handle_tip_end_x = Vector3Add(axis_handle_tip_start_x, Vector3Scale(X_AXIS, axis_handle_tip_length));
    Vector3 axis_handle_tip_end_y = Vector3Add(axis_handle_tip_start_y, Vector3Scale(Y_AXIS, axis_handle_tip_length));
    Vector3 axis_handle_tip_end_z = Vector3Add(axis_handle_tip_start_z, Vector3Scale(Z_AXIS, axis_handle_tip_length));

    // -------------------------------------------------------------------
    // Update gizmo CPU logic
    Matrix transform = MatrixIdentity();

    if (!IsMouseButtonDown(0)) {
        GIZMO_STATE = GIZMO_COLD;
        GIZMO_ACTIVE_AXIS = Vector3Zero();
    }

    Ray mouse_ray = GetMouseRay(GetMousePosition(), camera);
    RayCollision mouse_ray_collision = GetRayCollisionSphere(mouse_ray, position, radius);
    if (GIZMO_STATE != GIZMO_ACTIVE_ROT && mouse_ray_collision.hit) {  
        if (fabsf((mouse_ray_collision.point.x - position.x) / radius) < GIZMO_ROT_CIRCLE_THICKNESS) {
            GIZMO_ACTIVE_AXIS = X_AXIS;
            GIZMO_STATE = GIZMO_HOT_ROT;
        } else if (fabsf((mouse_ray_collision.point.y - position.y) / radius) < GIZMO_ROT_CIRCLE_THICKNESS) {
            GIZMO_ACTIVE_AXIS = Y_AXIS;
            GIZMO_STATE = GIZMO_HOT_ROT;
        } else if (fabsf((mouse_ray_collision.point.z - position.z) / radius) < GIZMO_ROT_CIRCLE_THICKNESS) {
            GIZMO_ACTIVE_AXIS = Z_AXIS;
            GIZMO_STATE = GIZMO_HOT_ROT;
        }

        if (GIZMO_STATE == GIZMO_HOT_ROT && IsMouseButtonDown(0)) {
            GIZMO_STATE = GIZMO_ACTIVE_ROT;
        }
    } else if (GIZMO_STATE == GIZMO_ACTIVE_ROT) {
        Vector2 rot_center = GetWorldToScreen(position, camera);
        Vector2 p1 = Vector2Subtract(GetMousePosition(), rot_center);
        Vector2 p0 = Vector2Subtract(p1, GetMouseDelta());
        float angle = Vector2WrapAngle(p1, p0);
        transform = MatrixRotate(GIZMO_ACTIVE_AXIS, angle);
    }

    // -------------------------------------------------------------------
    // Draw gizmo
    Color rot_handle_color_x = GIZMO_ACTIVE_AXIS.x == 1.0 ? WHITE : RED;
    Color rot_handle_color_y = GIZMO_ACTIVE_AXIS.y == 1.0 ? WHITE : GREEN;
    Color rot_handle_color_z = GIZMO_ACTIVE_AXIS.z == 1.0 ? WHITE : BLUE;

    Color axis_handle_color_x = RED;
    Color axis_handle_color_y = GREEN;
    Color axis_handle_color_z = BLUE;

    BeginMode3D(camera);
        rlSetLineWidth(GIZMO_ROT_CIRCLE_DRAW_THICKNESS);
        rlDisableDepthTest();

        BeginShaderMode(SHADER_ROT_HANDLE);
            SetShaderValue(SHADER_ROT_HANDLE, SHADER_ROT_HANDLE_CAMERA_POS_LOC, &camera.position, SHADER_UNIFORM_VEC3);
            SetShaderValue(SHADER_ROT_HANDLE, SHADER_ROT_HANDLE_GIZMO_POS_LOC, &position, SHADER_UNIFORM_VEC3);
            DrawCircle3D(position, radius, Y_AXIS, 90.0, rot_handle_color_x);
            DrawCircle3D(position, radius, X_AXIS, 90.0, rot_handle_color_y);
            DrawCircle3D(position, radius, X_AXIS, 0.0, rot_handle_color_z);
        EndShaderMode();
 
        DrawLine3D(position, axis_handle_tip_start_x, axis_handle_color_x);
        DrawLine3D(position, axis_handle_tip_start_y, axis_handle_color_y);
        DrawLine3D(position, axis_handle_tip_start_z, axis_handle_color_z);

        DrawCylinderEx(axis_handle_tip_start_x, axis_handle_tip_end_x, axis_handle_tip_radius, 0.0, 16, axis_handle_color_x);
        DrawCylinderEx(axis_handle_tip_start_y, axis_handle_tip_end_y, axis_handle_tip_radius, 0.0, 16, axis_handle_color_y);
        DrawCylinderEx(axis_handle_tip_start_z, axis_handle_tip_end_z, axis_handle_tip_radius, 0.0, 16, axis_handle_color_z);
    EndMode3D();

    if (GIZMO_STATE == GIZMO_ACTIVE_ROT) {
        rlSetLineWidth(GIZMO_ROT_HANDLE_DRAW_THICKNESS);

        Vector3 half_axis_line = Vector3Scale(GIZMO_ACTIVE_AXIS, 1000.0);
        BeginMode3D(camera);
            DrawLine3D(Vector3Subtract(position, half_axis_line), Vector3Add(position, half_axis_line), WHITE);
        EndMode3D();

        DrawLineV(GetWorldToScreen(position, camera), GetMousePosition(), WHITE);
    }

    return transform;
}


float Vector2WrapAngle(Vector2 v1, Vector2 v2) {
    static float eps = 0.00001;
    Vector2 v1_norm = Vector2Normalize(v1);
    Vector2 v2_norm = Vector2Normalize(v2);
    float dot = Vector2DotProduct(v1_norm, v2_norm);
    if (1.0 - fabs(dot) < eps) {
        return 0.0;
    }
    float angle = acos(dot);
    float z = v1.x * v2.y - v1.y * v2.x;
    if (fabs(z) < eps) {
        return 0.0;
    } else if (z > 0) {
        return angle;
    } else {
        return -angle;
    }
}

#endif
