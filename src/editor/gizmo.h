#include <raylib.h>
#include <stdlib.h>

typedef enum GizmoState {
    GIZMO_COLD,

    GIZMO_HOT_ROT,
    GIZMO_ACTIVE_ROT,

    GIZMO_HOT_AXIS,
    GIZMO_ACTIVE_AXIS,
} GizmoState;

void GizmoLoad();
Matrix GizmoUpdate(Camera3D camera, Vector3 position);


#define RAYGIZMO_IMPLEMENTATION
#ifdef RAYGIZMO_IMPLEMENTATION

#include "raymath.h"
#include <math.h>
#include <rlgl.h>
#include <stdio.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define MASK_FRAMEBUFFER_WIDTH 500.0
#define MASK_FRAMEBUFFER_HEIGHT 500.0

#define GIZMO_SIZE 0.12f
#define GIZMO_ROT_CIRCLE_THICKNESS 0.1f
#define GIZMO_ROT_CIRCLE_DRAW_THICKNESS 5.0f
#define GIZMO_ROT_HANDLE_DRAW_THICKNESS 2.0f

#define X_AXIS (Vector3){1.0, 0.0, 0.0}
#define Y_AXIS (Vector3){0.0, 1.0, 0.0}
#define Z_AXIS (Vector3){0.0, 0.0, 1.0}


static const char *SHADER_COLOR_VERT = "\
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

static const char *SHADER_ROT_HANDLE_COLOR_FRAG = "\
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

static Shader SHADER_ROT_HANDLE_COLOR;
static int SHADER_ROT_HANDLE_CAMERA_POS_LOC;
static int SHADER_ROT_HANDLE_GIZMO_POS_LOC;

static bool GIZMO_LOADED;

static GizmoState GIZMO_STATE;
static Vector3 GIZMO_CURRENT_AXIS;

static unsigned int MASK_FRAMEBUFFER;
static unsigned int MASK_TEXTURE;

static float Vector2WrapAngle(Vector2 v1, Vector2 v2);
static RayCollision GetRayCollisionPlane(Ray ray, Vector3 plane_point, Vector3 plane_normal);
static int isect_line_plane(
    Vector3 *out_p,
    Vector3 line_p0,
    Vector3 line_p1,
    Vector3 plane_p,
    Vector3 plane_normal
);
static int get_two_vecs_nearest_point(
    Vector3 *vec0_out_nearest_point,
    Vector3 vec0_p0,
    Vector3 vec0_p1,
    Vector3 vec1_p0,
    Vector3 vec1_p1
);

void GizmoLoad() {
    if (GIZMO_LOADED) {
        TraceLog(LOG_ERROR, "Gizmo can be loaded only once");
        exit(1);
    }

    SHADER_ROT_HANDLE_COLOR = LoadShaderFromMemory(SHADER_COLOR_VERT, SHADER_ROT_HANDLE_COLOR_FRAG);
    SHADER_ROT_HANDLE_CAMERA_POS_LOC = GetShaderLocation(SHADER_ROT_HANDLE_COLOR, "cameraPosition");
    SHADER_ROT_HANDLE_GIZMO_POS_LOC = GetShaderLocation(SHADER_ROT_HANDLE_COLOR, "gizmoPosition");

    MASK_FRAMEBUFFER = rlLoadFramebuffer(MASK_FRAMEBUFFER_WIDTH, MASK_FRAMEBUFFER_HEIGHT);
    if (!MASK_FRAMEBUFFER) {
        TraceLog(LOG_ERROR, "Failed to create gizmo's mask framebuffer");
        exit(1);
    }
    rlEnableFramebuffer(MASK_FRAMEBUFFER);
    MASK_TEXTURE = rlLoadTexture(NULL, MASK_FRAMEBUFFER_WIDTH, MASK_FRAMEBUFFER_HEIGHT, RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32, 1);
    rlActiveDrawBuffers(1);
    rlFramebufferAttach(MASK_FRAMEBUFFER, MASK_TEXTURE, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
    if (!rlFramebufferComplete(MASK_FRAMEBUFFER)) {
        TraceLog(LOG_ERROR, "Gizmo's framebuffer is not complete");
        exit(1);
    }

    GIZMO_LOADED = true;
}

Matrix GizmoUpdate(Camera3D camera, Vector3 position) {
    if (!GIZMO_LOADED) {
        TraceLog(LOG_ERROR, "Gizmo must be loaded before the update");
        exit(1);
    }

    Matrix transform = MatrixIdentity();
    Vector2 mouse_delta = GetMouseDelta();
    Vector2 curr_mouse_position = GetMousePosition();
    Vector2 prev_mouse_position = Vector2Subtract(curr_mouse_position, mouse_delta);

    float radius = GIZMO_SIZE * Vector3Distance(camera.position, position);
    float axis_handle_length = radius * 1.2;
    float axis_handle_tip_length = radius * 0.3;
    float axis_handle_tip_radius = radius * 0.1;
    float handle_plane_offset = radius * 0.4;
    Vector2 handle_plane_size = Vector2Scale(Vector2One(), radius * 0.2);

    Color color_x = RED;
    Color color_y = GREEN;
    Color color_z = BLUE;

    Color rot_handle_mask_x = (Color){1, 0, 0, 0};
    Color rot_handle_mask_y = (Color){2, 0, 0, 0};
    Color rot_handle_mask_z = (Color){3, 0, 0, 0};
    Color axis_handle_mask_x = (Color){4, 0, 0, 0};
    Color axis_handle_mask_y = (Color){5, 0, 0, 0};
    Color axis_handle_mask_z = (Color){6, 0, 0, 0};

    Vector3 axis_handle_tip_start_x = Vector3Add(position, Vector3Scale(X_AXIS, axis_handle_length));
    Vector3 axis_handle_tip_start_y = Vector3Add(position, Vector3Scale(Y_AXIS, axis_handle_length));
    Vector3 axis_handle_tip_start_z = Vector3Add(position, Vector3Scale(Z_AXIS, axis_handle_length));

    Vector3 axis_handle_tip_end_x = Vector3Add(axis_handle_tip_start_x, Vector3Scale(X_AXIS, axis_handle_tip_length));
    Vector3 axis_handle_tip_end_y = Vector3Add(axis_handle_tip_start_y, Vector3Scale(Y_AXIS, axis_handle_tip_length));
    Vector3 axis_handle_tip_end_z = Vector3Add(axis_handle_tip_start_z, Vector3Scale(Z_AXIS, axis_handle_tip_length));


    // -------------------------------------------------------------------
    // Draw gizmo to the mask buffer (for pixel picking)
    rlEnableFramebuffer(MASK_FRAMEBUFFER);
    rlViewport(0, 0, MASK_FRAMEBUFFER_WIDTH, MASK_FRAMEBUFFER_HEIGHT);
    rlClearScreenBuffers();
    rlDisableColorBlend();

    BeginMode3D(camera);
        rlSetLineWidth(8.0);

        BeginShaderMode(SHADER_ROT_HANDLE_COLOR);
            SetShaderValue(SHADER_ROT_HANDLE_COLOR, SHADER_ROT_HANDLE_CAMERA_POS_LOC, &camera.position, SHADER_UNIFORM_VEC3);
            SetShaderValue(SHADER_ROT_HANDLE_COLOR, SHADER_ROT_HANDLE_GIZMO_POS_LOC, &position, SHADER_UNIFORM_VEC3);
            DrawCircle3D(position, radius, Y_AXIS, 90.0, rot_handle_mask_x);
            DrawCircle3D(position, radius, X_AXIS, 90.0, rot_handle_mask_y);
            DrawCircle3D(position, radius, X_AXIS, 0.0, rot_handle_mask_z);
        EndShaderMode();

        DrawLine3D(position, axis_handle_tip_start_x, axis_handle_mask_x);
        DrawLine3D(position, axis_handle_tip_start_y, axis_handle_mask_y);
        DrawLine3D(position, axis_handle_tip_start_z, axis_handle_mask_z);

        DrawCylinderEx(axis_handle_tip_start_x, axis_handle_tip_end_x, axis_handle_tip_radius, 0.0, 16, axis_handle_mask_x);
        DrawCylinderEx(axis_handle_tip_start_y, axis_handle_tip_end_y, axis_handle_tip_radius, 0.0, 16, axis_handle_mask_y);
        DrawCylinderEx(axis_handle_tip_start_z, axis_handle_tip_end_z, axis_handle_tip_radius, 0.0, 16, axis_handle_mask_z);
    EndMode3D();

    unsigned char *pixels = (unsigned char*)rlReadTexturePixels(MASK_TEXTURE, MASK_FRAMEBUFFER_WIDTH, MASK_FRAMEBUFFER_HEIGHT, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    int x = (int)(MASK_FRAMEBUFFER_WIDTH * Clamp(curr_mouse_position.x / (float)GetScreenWidth(), 0.0, 1.0));
    int y = (int)(MASK_FRAMEBUFFER_HEIGHT * Clamp(1.0 - (curr_mouse_position.y / (float)GetScreenHeight()), 0.0, 1.0));
    int idx = 4 * (y * MASK_FRAMEBUFFER_WIDTH + x);
    unsigned char mask_val = pixels[idx];

    free(pixels);

    rlDisableFramebuffer();
    rlEnableColorBlend();
    rlViewport(0, 0, GetScreenWidth(), GetScreenHeight());

    // -------------------------------------------------------------------
    // Update gizmo CPU logic

    if (!IsMouseButtonDown(0)) {
        GIZMO_STATE = GIZMO_COLD;
        GIZMO_CURRENT_AXIS = Vector3Zero();
    }

    if (GIZMO_STATE != GIZMO_ACTIVE_ROT && GIZMO_STATE != GIZMO_ACTIVE_AXIS) {  
        if (mask_val == rot_handle_mask_x.r) {
            GIZMO_CURRENT_AXIS = X_AXIS;
            GIZMO_STATE = GIZMO_HOT_ROT;
        } else if (mask_val == rot_handle_mask_y.r) {
            GIZMO_CURRENT_AXIS = Y_AXIS;
            GIZMO_STATE = GIZMO_HOT_ROT;
        } else if (mask_val == rot_handle_mask_z.r) {
            GIZMO_CURRENT_AXIS = Z_AXIS;
            GIZMO_STATE = GIZMO_HOT_ROT;
        } else if (mask_val == axis_handle_mask_x.r) {
            GIZMO_CURRENT_AXIS = X_AXIS;
            GIZMO_STATE = GIZMO_HOT_AXIS;
        } else if (mask_val == axis_handle_mask_y.r) {
            GIZMO_CURRENT_AXIS = Y_AXIS;
            GIZMO_STATE = GIZMO_HOT_AXIS;
        } else if (mask_val == axis_handle_mask_z.r) {
            GIZMO_CURRENT_AXIS = Z_AXIS;
            GIZMO_STATE = GIZMO_HOT_AXIS;
        }

        if (GIZMO_STATE == GIZMO_HOT_ROT && IsMouseButtonDown(0)) {
            GIZMO_STATE = GIZMO_ACTIVE_ROT;
        } else if (GIZMO_STATE == GIZMO_HOT_AXIS && IsMouseButtonDown(0)) {
            GIZMO_STATE = GIZMO_ACTIVE_AXIS;
        }
    } else if (GIZMO_STATE == GIZMO_ACTIVE_ROT) {
        Vector2 rot_center = GetWorldToScreen(position, camera);
        Vector2 p1 = Vector2Subtract(curr_mouse_position, rot_center);
        Vector2 p0 = Vector2Subtract(p1, mouse_delta);
        float angle = Vector2WrapAngle(p1, p0);

        if (
            Vector3DotProduct(GIZMO_CURRENT_AXIS, position)
            > Vector3DotProduct(GIZMO_CURRENT_AXIS, camera.position)
        ) {
            angle *= -1;
        }

        transform = MatrixMultiply(
                MatrixMultiply(
                    MatrixTranslate(-position.x, -position.y, -position.z),
                    MatrixRotate(GIZMO_CURRENT_AXIS, angle)
                ),
                MatrixTranslate(position.x, position.y, position.z)
        );
    } else if (GIZMO_STATE == GIZMO_ACTIVE_AXIS) {
        Vector3 normal;
        if (GIZMO_CURRENT_AXIS.x == 1.0) {
            normal = Y_AXIS;
        } else if (GIZMO_CURRENT_AXIS.y == 1.0) {
            normal = Z_AXIS;
        } else if (GIZMO_CURRENT_AXIS.z == 1.0) {
            normal = X_AXIS;
        }

        Vector2 p = Vector2Add(GetWorldToScreen(position, camera), mouse_delta);
        Ray r = GetMouseRay(p, camera);
        Vector3 isect_p;
        int is_isect = get_two_vecs_nearest_point(
            &isect_p,
            camera.position,
            Vector3Add(camera.position, r.direction),
            position,
            Vector3Add(position, GIZMO_CURRENT_AXIS)
        );

        Vector3 offset = Vector3Subtract(isect_p, position);
        offset = Vector3Multiply(offset, GIZMO_CURRENT_AXIS);
        transform = MatrixTranslate(offset.x, offset.y, offset.z);
    }

    // -------------------------------------------------------------------
    // Draw gizmo
    Color rot_handle_color_x = color_x;
    Color rot_handle_color_y = color_y;
    Color rot_handle_color_z = color_z;
    if (GIZMO_STATE == GIZMO_ACTIVE_ROT || GIZMO_STATE == GIZMO_HOT_ROT) {
        if (GIZMO_CURRENT_AXIS.x == 1.0) rot_handle_color_x = WHITE;
        if (GIZMO_CURRENT_AXIS.y == 1.0) rot_handle_color_y = WHITE;
        if (GIZMO_CURRENT_AXIS.z == 1.0) rot_handle_color_z = WHITE;
    }

    Color axis_handle_color_x = color_x;
    Color axis_handle_color_y = color_y;
    Color axis_handle_color_z = color_z;
    if (GIZMO_STATE == GIZMO_ACTIVE_AXIS || GIZMO_STATE == GIZMO_HOT_AXIS) {
        if (GIZMO_CURRENT_AXIS.x == 1.0) axis_handle_color_x = WHITE;
        if (GIZMO_CURRENT_AXIS.y == 1.0) axis_handle_color_y = WHITE;
        if (GIZMO_CURRENT_AXIS.z == 1.0) axis_handle_color_z = WHITE;
    }

    BeginMode3D(camera);
        rlSetLineWidth(GIZMO_ROT_CIRCLE_DRAW_THICKNESS);
        rlDisableDepthTest();

        BeginShaderMode(SHADER_ROT_HANDLE_COLOR);
            SetShaderValue(SHADER_ROT_HANDLE_COLOR, SHADER_ROT_HANDLE_CAMERA_POS_LOC, &camera.position, SHADER_UNIFORM_VEC3);
            SetShaderValue(SHADER_ROT_HANDLE_COLOR, SHADER_ROT_HANDLE_GIZMO_POS_LOC, &position, SHADER_UNIFORM_VEC3);
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

        rlDisableBackfaceCulling();

        Vector3 tz = (Vector3){handle_plane_offset, handle_plane_offset, 0.0};
        Vector3 tx = (Vector3){0.0, handle_plane_offset, handle_plane_offset};
        Vector3 ty = (Vector3){handle_plane_offset, 0.0, handle_plane_offset};
        Vector3 pz = Vector3Add(position, tz);
        Vector3 px = Vector3Add(position, tx);
        Vector3 py = Vector3Add(position, ty);
        float dz = Vector3DistanceSqr(pz, camera.position);
        float dx = Vector3DistanceSqr(px, camera.position);
        float dy = Vector3DistanceSqr(py, camera.position);

        if (dz <= MIN(dx, dy)) {
            if (dx <= dy) {
                rlPushMatrix();
                    rlTranslatef(py.x, py.y, py.z);
                    rlRotatef(90.0, 0.0, 1.0, 0.0);
                    DrawPlane(Vector3Zero(), handle_plane_size, GREEN);
                rlPopMatrix();
                rlPushMatrix();
                    rlTranslatef(px.x, px.y, px.z);
                    rlRotatef(90.0, 0.0, 0.0, 1.0);
                    DrawPlane(Vector3Zero(), handle_plane_size, RED);
                rlPopMatrix();
            } else {
                rlPushMatrix();
                    rlTranslatef(px.x, px.y, px.z);
                    rlRotatef(90.0, 0.0, 0.0, 1.0);
                    DrawPlane(Vector3Zero(), handle_plane_size, RED);
                rlPopMatrix();
                rlPushMatrix();
                    rlTranslatef(py.x, py.y, py.z);
                    rlRotatef(90.0, 0.0, 1.0, 0.0);
                    DrawPlane(Vector3Zero(), handle_plane_size, GREEN);
                rlPopMatrix();
            }

            rlPushMatrix();
                rlTranslatef(pz.x, pz.y, pz.z);
                rlRotatef(90.0, 1.0, 0.0, 0.0);
                DrawPlane(Vector3Zero(), handle_plane_size, BLUE);
            rlPopMatrix();
        } else if (dx <= MIN(dz, dy)) {
            if (dz <= dy) {
                rlPushMatrix();
                    rlTranslatef(py.x, py.y, py.z);
                    rlRotatef(90.0, 0.0, 1.0, 0.0);
                    DrawPlane(Vector3Zero(), handle_plane_size, GREEN);
                rlPopMatrix();
                rlPushMatrix();
                    rlTranslatef(pz.x, pz.y, pz.z);
                    rlRotatef(90.0, 1.0, 0.0, 0.0);
                    DrawPlane(Vector3Zero(), handle_plane_size, BLUE);
                rlPopMatrix();
            } else {
                rlPushMatrix();
                    rlTranslatef(pz.x, pz.y, pz.z);
                    rlRotatef(90.0, 1.0, 0.0, 0.0);
                    DrawPlane(Vector3Zero(), handle_plane_size, BLUE);
                rlPopMatrix();
                rlPushMatrix();
                    rlTranslatef(py.x, py.y, py.z);
                    rlRotatef(90.0, 0.0, 1.0, 0.0);
                    DrawPlane(Vector3Zero(), handle_plane_size, GREEN);
                rlPopMatrix();
            }

            rlPushMatrix();
                rlTranslatef(px.x, px.y, px.z);
                rlRotatef(90.0, 0.0, 0.0, 1.0);
                DrawPlane(Vector3Zero(), handle_plane_size, RED);
            rlPopMatrix();
        } else {
            if (dz <= dx) {
                rlPushMatrix();
                    rlTranslatef(px.x, px.y, px.z);
                    rlRotatef(90.0, 0.0, 0.0, 1.0);
                    DrawPlane(Vector3Zero(), handle_plane_size, RED);
                rlPopMatrix();
                rlPushMatrix();
                    rlTranslatef(pz.x, pz.y, pz.z);
                    rlRotatef(90.0, 1.0, 0.0, 0.0);
                    DrawPlane(Vector3Zero(), handle_plane_size, BLUE);
                rlPopMatrix();
            } else {
                rlPushMatrix();
                    rlTranslatef(pz.x, pz.y, pz.z);
                    rlRotatef(90.0, 1.0, 0.0, 0.0);
                    DrawPlane(Vector3Zero(), handle_plane_size, BLUE);
                rlPopMatrix();
                rlPushMatrix();
                    rlTranslatef(px.x, px.y, px.z);
                    rlRotatef(90.0, 0.0, 0.0, 1.0);
                    DrawPlane(Vector3Zero(), handle_plane_size, RED);
                rlPopMatrix();
            }

            rlPushMatrix();
                rlTranslatef(py.x, py.y, py.z);
                rlRotatef(90.0, 0.0, 1.0, 0.0);
                DrawPlane(Vector3Zero(), handle_plane_size, GREEN);
            rlPopMatrix();
        }


    EndMode3D();

    if (GIZMO_STATE == GIZMO_ACTIVE_ROT || GIZMO_STATE == GIZMO_ACTIVE_AXIS) {
        rlSetLineWidth(GIZMO_ROT_HANDLE_DRAW_THICKNESS);

        Vector3 half_axis_line = Vector3Scale(GIZMO_CURRENT_AXIS, 1000.0);
        BeginMode3D(camera);
            DrawLine3D(Vector3Subtract(position, half_axis_line), Vector3Add(position, half_axis_line), WHITE);
        EndMode3D();

        if (GIZMO_STATE == GIZMO_ACTIVE_ROT) {
            DrawLineV(GetWorldToScreen(position, camera), curr_mouse_position, WHITE);
        }
    }

    return transform;
}


static float Vector2WrapAngle(Vector2 v1, Vector2 v2) {
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

static RayCollision GetRayCollisionPlane(Ray ray, Vector3 plane_point, Vector3 plane_normal) {
    RayCollision collision = {0};

    // Calculate the parameter t
    float denominator = ray.direction.x * plane_normal.x +
                        ray.direction.y * plane_normal.y +
                        ray.direction.z * plane_normal.z;

    if (denominator == 0) {
        // Ray is parallel to the plane, no collision
        return collision;
    }

    float t = ((plane_point.x - ray.position.x) * plane_normal.x +
               (plane_point.y - ray.position.y) * plane_normal.y +
               (plane_point.z - ray.position.z) * plane_normal.z) / denominator;

    if (t < 0) {
        // Intersection point is behind the ray's starting point, no collision
        return collision;
    }

    // Calculate the collision point
    collision.point.x = ray.position.x + t * ray.direction.x;
    collision.point.y = ray.position.y + t * ray.direction.y;
    collision.point.z = ray.position.z + t * ray.direction.z;
    collision.hit = true;

    return collision;
}

static int isect_line_plane(
    Vector3 *out_p,
    Vector3 line_p0,
    Vector3 line_p1,
    Vector3 plane_p,
    Vector3 plane_normal
) {
    static float eps = 0.00001;
    Vector3 u = Vector3Subtract(line_p1, line_p0);
    float dot = Vector3DotProduct(plane_normal, u);
    if (fabs(dot) <= eps) {
        return 0;
    }

    Vector3 w = Vector3Subtract(line_p0, plane_p);
    float k = -Vector3DotProduct(plane_normal, w) / dot;
    u = Vector3Scale(u, k);
    *out_p = Vector3Add(line_p0, u);
    return 1;
}


static int get_two_vecs_nearest_point(
    Vector3 *vec0_out_nearest_point,
    Vector3 vec0_p0,
    Vector3 vec0_p1,
    Vector3 vec1_p0,
    Vector3 vec1_p1
) {
    Vector3 vec0 = Vector3Subtract(vec0_p1, vec0_p0);
    Vector3 vec1 = Vector3Subtract(vec1_p1, vec1_p0);
    Vector3 plane_vec = Vector3CrossProduct(vec0, vec1);
    plane_vec = Vector3Normalize(plane_vec);
    Vector3 plane_normal = Vector3CrossProduct(vec0, plane_vec);
    plane_normal = Vector3Normalize(plane_normal);

    int is_isect = isect_line_plane(
        vec0_out_nearest_point, vec1_p0, vec1_p1, vec0_p0, plane_normal
    );

    return is_isect;
}

#endif
