#include <raylib.h>
#include <stdlib.h>

void GizmoLoad();
Matrix GizmoUpdate(Camera3D camera, Vector3 position);


#define RAYGIZMO_IMPLEMENTATION
#ifdef RAYGIZMO_IMPLEMENTATION

#include "raymath.h"
#include <math.h>
#include <rlgl.h>
#include <stdio.h>
#include <string.h>

#define swap(x,y) \
    do { \
        unsigned char swap_temp[sizeof(x) == sizeof(y) ? (signed)sizeof(x) : -1]; \
        memcpy(swap_temp,&y,sizeof(x)); \
        memcpy(&y,&x,       sizeof(x)); \
        memcpy(&x,swap_temp,sizeof(x)); \
    } while(0)

#define id_to_red_color(id) (Color){id, 0, 0, 0}

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

static Vector3 GIZMO_CURRENT_AXIS;

static unsigned int MASK_FRAMEBUFFER;
static unsigned int MASK_TEXTURE;

typedef enum HandleId {
    HANDLE_X,

    ROT_HANDLE_X,
    AXIS_HANDLE_X,
    PLANE_HANDLE_X,

    HANDLE_Y,

    ROT_HANDLE_Y,
    AXIS_HANDLE_Y,
    PLANE_HANDLE_Y,

    HANDLE_Z,

    ROT_HANDLE_Z,
    AXIS_HANDLE_Z,
    PLANE_HANDLE_Z
} HandleId;

typedef enum GizmoState {
    GIZMO_COLD,

    GIZMO_HOT,

    GIZMO_HOT_ROT,
    GIZMO_HOT_AXIS,
    GIZMO_HOT_PLANE,

    GIZMO_ACTIVE,

    GIZMO_ACTIVE_ROT,
    GIZMO_ACTIVE_AXIS,
    GIZMO_ACTIVE_PLANE,
} GizmoState;

typedef struct Handle {
    Vector3 position;
    Vector3 axis;
    Color color;
    float dist_to_cam;
} Handle;

typedef struct Handles {
    Handle arr[3];
} Handles;

static GizmoState GIZMO_STATE;

static Handles get_sorted_handles(Handle h0, Handle h1, Handle h2) {
    if (h0.dist_to_cam < h1.dist_to_cam) swap(h0, h1);
    if (h1.dist_to_cam < h2.dist_to_cam) swap(h1, h2);
    if (h0.dist_to_cam < h1.dist_to_cam) swap(h0, h1);

    Handles handles = {.arr={h0, h1, h2}};
    return handles;
}

static void draw_axis_handles(
    Camera3D camera,
    Vector3 position,
    float gizmo_radius,
    Color color_x,
    Color color_y,
    Color color_z
) {
    float length = gizmo_radius * 1.2;
    float tip_length = gizmo_radius * 0.3;
    float tip_radius = gizmo_radius * 0.1;

    Vector3 px = Vector3Add(position, Vector3Scale(X_AXIS, length));
    Vector3 py = Vector3Add(position, Vector3Scale(Y_AXIS, length));
    Vector3 pz = Vector3Add(position, Vector3Scale(Z_AXIS, length));

    Handle hx = {px, X_AXIS, color_x, Vector3DistanceSqr(px, camera.position)};
    Handle hy = {py, Y_AXIS, color_y, Vector3DistanceSqr(py, camera.position)};
    Handle hz = {pz, Z_AXIS, color_z, Vector3DistanceSqr(pz, camera.position)};

    Handles handles = get_sorted_handles(hx, hy, hz);

    for (int i = 0; i < 3; ++i) {
        Handle *h = &handles.arr[i];
        Vector3 tip_end = Vector3Add(h->position, Vector3Scale(h->axis, tip_length));
        DrawLine3D(position, h->position, h->color);
        DrawCylinderEx(h->position, tip_end, tip_radius, 0.0, 16, h->color);
    }
}

static void draw_plane_handles(
    Camera3D camera,
    Vector3 position,
    float gizmo_radius,
    Color color_x,
    Color color_y,
    Color color_z
) {
    float offset = gizmo_radius * 0.4;
    float size = gizmo_radius * 0.2;

    Vector3 px = Vector3Add(position, (Vector3){0.0, offset, offset});
    Vector3 py = Vector3Add(position, (Vector3){offset, 0.0, offset});
    Vector3 pz = Vector3Add(position, (Vector3){offset, offset, 0.0});

    Handle hx = { px, Z_AXIS, color_x, Vector3DistanceSqr(px, camera.position) };
    Handle hy = { py, Y_AXIS, color_y, Vector3DistanceSqr(py, camera.position) };
    Handle hz = { pz, X_AXIS, color_z, Vector3DistanceSqr(pz, camera.position) };

    Handles handles = get_sorted_handles(hx, hy, hz);

    rlDisableBackfaceCulling();
    for (int i = 0; i < 3; ++i) {
        Handle *h = &handles.arr[i];
        rlPushMatrix();
            rlTranslatef(h->position.x, h->position.y, h->position.z);
            rlRotatef(90.0, h->axis.x, h->axis.y, h->axis.z);
            DrawPlane(Vector3Zero(), Vector2Scale(Vector2One(), size), h->color);
        rlPopMatrix();
    }
}

static void draw_rot_handles(
    Camera3D camera,
    Vector3 position,
    float gizmo_radius,
    Color color_x,
    Color color_y,
    Color color_z
) {
    BeginShaderMode(SHADER_ROT_HANDLE_COLOR);
        SetShaderValue(
            SHADER_ROT_HANDLE_COLOR,
            SHADER_ROT_HANDLE_CAMERA_POS_LOC,
            &camera.position,
            SHADER_UNIFORM_VEC3
        );
        SetShaderValue(
            SHADER_ROT_HANDLE_COLOR,
            SHADER_ROT_HANDLE_GIZMO_POS_LOC,
            &position,
            SHADER_UNIFORM_VEC3
        );
        DrawCircle3D(position, gizmo_radius, Y_AXIS, 90.0, color_x);
        DrawCircle3D(position, gizmo_radius, X_AXIS, 90.0, color_y);
        DrawCircle3D(position, gizmo_radius, X_AXIS, 0.0, color_z);
    EndShaderMode();
}

static float vector2_get_angle(Vector2 v1, Vector2 v2) {
    Vector2 v1_norm = Vector2Normalize(v1);
    Vector2 v2_norm = Vector2Normalize(v2);
    float dot = Vector2DotProduct(v1_norm, v2_norm);
    if (1.0 - fabs(dot) < EPSILON) {
        return 0.0;
    }
    float angle = acos(dot);
    float z = v1.x * v2.y - v1.y * v2.x;
    if (fabs(z) < EPSILON) {
        return 0.0;
    } else if (z > 0) {
        return angle;
    } else {
        return -angle;
    }
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

static RayCollision get_ray_plane_collision(
    Ray ray,
    Vector3 plane_point,
    Vector3 plane_normal
) {
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


static void draw_gizmo(
    Camera3D camera,
    Vector3 position,
    Color rot_handle_color_x,
    Color rot_handle_color_y,
    Color rot_handle_color_z,
    Color axis_handle_color_x,
    Color axis_handle_color_y,
    Color axis_handle_color_z,
    Color plane_handle_color_x,
    Color plane_handle_color_y,
    Color plane_handle_color_z
) {
    float radius = GIZMO_SIZE * Vector3Distance(camera.position, position);

    BeginMode3D(camera);
        rlSetLineWidth(GIZMO_ROT_CIRCLE_DRAW_THICKNESS);
        rlDisableDepthTest();

        draw_plane_handles(
            camera,
            position,
            radius,
            plane_handle_color_x,
            plane_handle_color_y,
            plane_handle_color_z
        );

        draw_rot_handles(
            camera,
            position,
            radius,
            rot_handle_color_x,
            rot_handle_color_y,
            rot_handle_color_z
        );

        draw_axis_handles(
            camera,
            position,
            radius,
            axis_handle_color_x,
            axis_handle_color_y,
            axis_handle_color_z
        );
    EndMode3D();
}

static unsigned char get_gizmo_mask_pixel(Camera3D camera, Vector3 position) {
    // -------------------------------------------------------------------
    // Draw gizmo into a separate framebuffer for the mouse pixel-picking
    rlEnableFramebuffer(MASK_FRAMEBUFFER);
    rlViewport(0, 0, MASK_FRAMEBUFFER_WIDTH, MASK_FRAMEBUFFER_HEIGHT);
    rlClearScreenBuffers();
    rlDisableColorBlend();

    draw_gizmo(
        camera,
        position,
        id_to_red_color(ROT_HANDLE_X),
        id_to_red_color(ROT_HANDLE_Y),
        id_to_red_color(ROT_HANDLE_Z),
        id_to_red_color(AXIS_HANDLE_X),
        id_to_red_color(AXIS_HANDLE_Y),
        id_to_red_color(AXIS_HANDLE_Z),
        id_to_red_color(PLANE_HANDLE_X),
        id_to_red_color(PLANE_HANDLE_Y),
        id_to_red_color(PLANE_HANDLE_Z)
    );

    rlDisableFramebuffer();
    rlEnableColorBlend();
    rlViewport(0, 0, GetScreenWidth(), GetScreenHeight());

    // -------------------------------------------------------------------
    // Pick the pixel under the mouse cursor
    Vector2 mouse_position = GetMousePosition();
    unsigned char *pixels = (unsigned char*)rlReadTexturePixels(
        MASK_TEXTURE,
        MASK_FRAMEBUFFER_WIDTH,
        MASK_FRAMEBUFFER_HEIGHT,
        RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    );
    int x = (int)(
        MASK_FRAMEBUFFER_WIDTH
        * Clamp(mouse_position.x / (float)GetScreenWidth(), 0.0, 1.0)
    );
    int y = (int)(
        MASK_FRAMEBUFFER_HEIGHT
        * Clamp(1.0 - (mouse_position.y / (float)GetScreenHeight()), 0.0, 1.0)
    );
    int idx = 4 * (y * MASK_FRAMEBUFFER_WIDTH + x);
    unsigned char mask_val = pixels[idx];
    free(pixels);

    return mask_val;
}

bool check_if_mouse_moved() {
    Vector2 mouse_delta = GetMouseDelta();
    return (fabs(mouse_delta.x) + fabs(mouse_delta.y)) > EPSILON;
}

Matrix update_gizmo_rot(Camera3D camera, Vector3 position) {
    if (!check_if_mouse_moved()) return MatrixIdentity();
    Vector2 rot_center = GetWorldToScreen(position, camera);
    Vector2 p1 = Vector2Subtract(GetMousePosition(), rot_center);
    Vector2 p0 = Vector2Subtract(p1, GetMouseDelta());
    float angle = vector2_get_angle(p1, p0);

    if (
        Vector3DotProduct(GIZMO_CURRENT_AXIS, position)
        > Vector3DotProduct(GIZMO_CURRENT_AXIS, camera.position)
    ) {
        angle *= -1;
    }

    Matrix transform = MatrixMultiply(
            MatrixMultiply(
                MatrixTranslate(-position.x, -position.y, -position.z),
                MatrixRotate(GIZMO_CURRENT_AXIS, angle)
            ),
            MatrixTranslate(position.x, position.y, position.z)
    );

    return transform;
}

Matrix update_gizmo_axis(Camera3D camera, Vector3 position) {
    if (!check_if_mouse_moved()) return MatrixIdentity();
    Vector2 p = Vector2Add(GetWorldToScreen(position, camera), GetMouseDelta());
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
    Matrix transform = MatrixTranslate(offset.x, offset.y, offset.z);

    return transform;
}

Matrix update_gizmo_plane(Camera3D camera, Vector3 position) {
    if (!check_if_mouse_moved()) return MatrixIdentity();
    Vector2 p = Vector2Add(GetWorldToScreen(position, camera), GetMouseDelta());
    Ray r = GetMouseRay(p, camera);
    RayCollision c = get_ray_plane_collision(r, position, GIZMO_CURRENT_AXIS);
    Vector3 offset = Vector3Subtract(c.point, position);
    Matrix transform = MatrixTranslate(offset.x, offset.y, offset.z);

    return transform;
}

Matrix update_gizmo(Camera3D camera, Vector3 position, unsigned char mask_val) {
    Matrix transform = MatrixIdentity();
    Vector2 curr_mouse_position = GetMousePosition();
    bool is_lmb_down = IsMouseButtonDown(0);

    if (!is_lmb_down) GIZMO_STATE = GIZMO_COLD;

    if (GIZMO_STATE < GIZMO_ACTIVE) {  
        if      (mask_val < HANDLE_Y) GIZMO_CURRENT_AXIS = X_AXIS;
        else if (mask_val < HANDLE_Z) GIZMO_CURRENT_AXIS = Y_AXIS;
        else                          GIZMO_CURRENT_AXIS = Z_AXIS;

        if      (mask_val % 4 == 1)   GIZMO_STATE = is_lmb_down ? GIZMO_ACTIVE_ROT : GIZMO_HOT_ROT;
        else if (mask_val % 4 == 2)   GIZMO_STATE = is_lmb_down ? GIZMO_ACTIVE_AXIS : GIZMO_HOT_AXIS;
        else if (mask_val % 4 == 3)   GIZMO_STATE = is_lmb_down ? GIZMO_ACTIVE_PLANE : GIZMO_HOT_PLANE;
    }

    switch (GIZMO_STATE) {
        case GIZMO_ACTIVE_ROT: return update_gizmo_rot(camera, position);
        case GIZMO_ACTIVE_AXIS: return update_gizmo_axis(camera, position);
        case GIZMO_ACTIVE_PLANE: return update_gizmo_plane(camera, position);
        default: return MatrixIdentity();
    }
}

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

    unsigned char mask_val = get_gizmo_mask_pixel(camera, position);
    Matrix transform = update_gizmo(camera, position, mask_val);

    // -------------------------------------------------------------------
    // Draw gizmo
    Color rot_handle_color_x = RED;
    Color rot_handle_color_y = GREEN;
    Color rot_handle_color_z = BLUE;
    if (GIZMO_STATE == GIZMO_ACTIVE_ROT || GIZMO_STATE == GIZMO_HOT_ROT) {
        if (GIZMO_CURRENT_AXIS.x == 1.0) rot_handle_color_x = WHITE;
        if (GIZMO_CURRENT_AXIS.y == 1.0) rot_handle_color_y = WHITE;
        if (GIZMO_CURRENT_AXIS.z == 1.0) rot_handle_color_z = WHITE;
    }

    Color axis_handle_color_x = RED;
    Color axis_handle_color_y = GREEN;
    Color axis_handle_color_z = BLUE;
    if (GIZMO_STATE == GIZMO_ACTIVE_AXIS || GIZMO_STATE == GIZMO_HOT_AXIS) {
        if (GIZMO_CURRENT_AXIS.x == 1.0) axis_handle_color_x = WHITE;
        if (GIZMO_CURRENT_AXIS.y == 1.0) axis_handle_color_y = WHITE;
        if (GIZMO_CURRENT_AXIS.z == 1.0) axis_handle_color_z = WHITE;
    }

    Color plane_handle_color_x = RED;
    Color plane_handle_color_y = GREEN;
    Color plane_handle_color_z = BLUE;
    if (GIZMO_STATE == GIZMO_ACTIVE_PLANE || GIZMO_STATE == GIZMO_HOT_PLANE) {
        if (GIZMO_CURRENT_AXIS.x == 1.0) plane_handle_color_x = WHITE;
        if (GIZMO_CURRENT_AXIS.y == 1.0) plane_handle_color_y = WHITE;
        if (GIZMO_CURRENT_AXIS.z == 1.0) plane_handle_color_z = WHITE;
    }

    draw_gizmo(
        camera,
        position,
        rot_handle_color_x,
        rot_handle_color_y,
        rot_handle_color_z,
        axis_handle_color_x,
        axis_handle_color_y,
        axis_handle_color_z,
        plane_handle_color_x,
        plane_handle_color_y,
        plane_handle_color_z
    );

    if (GIZMO_STATE == GIZMO_ACTIVE_ROT || GIZMO_STATE == GIZMO_ACTIVE_AXIS) {
        rlSetLineWidth(GIZMO_ROT_HANDLE_DRAW_THICKNESS);

        Vector3 half_axis_line = Vector3Scale(GIZMO_CURRENT_AXIS, 1000.0);
        BeginMode3D(camera);
            DrawLine3D(Vector3Subtract(position, half_axis_line), Vector3Add(position, half_axis_line), WHITE);
        EndMode3D();

        if (GIZMO_STATE == GIZMO_ACTIVE_ROT) {
            DrawLineV(GetWorldToScreen(position, camera), GetMousePosition(), WHITE);
        }
    }

    return transform;
}

#endif
