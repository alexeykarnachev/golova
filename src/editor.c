#include "editor.h"

#include "camera.h"
#include "cimgui_utils.h"
#include "collisions.h"
#include "raylib.h"
#include "raymath.h"
#include <stdbool.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_USE_GLFW
#define CIMGUI_USE_OPENGL3
#include "cimgui.h"
#include "cimgui_impl.h"
#include <GLFW/glfw3.h>

#define RAYGIZMO_IMPLEMENTATION
#include "raygizmo.h"

static Camera3D CAMERA;
static RGizmo GIZMO;
static Entity* PICKED_ENTITY;

static int IG_ID;
static bool IS_IG_INTERACTED;

static void draw_grid(void);
static void draw_imgui(void);

void load_editor(void) {
    CAMERA.fovy = 60.0;
    CAMERA.projection = CAMERA_PERSPECTIVE;
    CAMERA.up = (Vector3){0.0, 1.0, 0.0};
    CAMERA.position = (Vector3){5.0, 5.0, 5.0};

    GIZMO = rgizmo_create();
    load_imgui();
}

void update_editor(void) {
    bool is_lmb_pressed = IsMouseButtonPressed(0) && !IS_IG_INTERACTED;
    bool is_mmb_down = IsMouseButtonDown(2) && !IS_IG_INTERACTED;
    bool is_shift_down = IsKeyDown(KEY_LEFT_SHIFT) && !IS_IG_INTERACTED;
    Vector2 mouse_delta = GetMouseDelta();

    update_orbital_camera(&CAMERA, is_mmb_down, is_shift_down, mouse_delta);

    if (is_lmb_pressed && GIZMO.state == RGIZMO_STATE_COLD) {
        // Pick the entity
        CollisionInfo info = cast_ray(GetMouseRay(GetMousePosition(), CAMERA));
        PICKED_ENTITY = info.entity;
    }

    if (PICKED_ENTITY) {
        Transform* t = &PICKED_ENTITY->transform;

        // Update gizmo on the picked entity
        rgizmo_update(&GIZMO, CAMERA, t->translation);
        t->translation = Vector3Add(t->translation, GIZMO.update.translation);
        t->rotation = QuaternionMultiply(
            QuaternionFromAxisAngle(GIZMO.update.axis, GIZMO.update.angle), t->rotation
        );
    }
}

void draw_editor(void) {
    rlDisableBackfaceCulling();

    // Draw grid
    BeginMode3D(CAMERA);
    draw_grid();
    EndMode3D();

    // Draw scene
    BeginMode3D(CAMERA);
    {
        draw_scene();

        // Draw gizmo on the picked entity
        if (PICKED_ENTITY) {
            rgizmo_draw(GIZMO, CAMERA, PICKED_ENTITY->transform.translation);
        }
    }
    EndMode3D();

    begin_imgui();
    draw_imgui();
    end_imgui();
}

void unload_editor(void) {
    rgizmo_unload();
}

static void draw_grid(void) {
    rlSetLineWidth(2.0);
    DrawGrid(10.0, 5.0);

    float d = 25.0f;
    DrawLine3D((Vector3){-d, 0.0f, 0.0f}, (Vector3){d, 0.0f, 0.0f}, RED);
    DrawLine3D((Vector3){0.0f, -d, 0.0f}, (Vector3){0.0f, d, 0.0f}, GREEN);
    DrawLine3D((Vector3){0.0f, 0.0f, -d}, (Vector3){0.0f, 0.0f, d}, DARKBLUE);
}

static void draw_imgui(void) {
    IG_ID = 1;
    ImGuiIO* io = igGetIO();
    IS_IG_INTERACTED = io->WantCaptureMouse || io->WantCaptureKeyboard;

    Camera3D* camera = &SCENE.camera_shell.camera;

    ig_fix_window_top_left();
    if (igBegin("Inspector", NULL, 0)) {
        if (ig_collapsing_header("Camera", true)) {
            igDragFloat("FOV", &camera->fovy, 1.0, 10.0, 170.0, "%.1f", 0);
        }

        if (ig_collapsing_header("Transform", true) && PICKED_ENTITY) {
            Transform* t = &PICKED_ENTITY->transform;
            igDragFloat3("Scale", (float*)&t->scale, 0.1, 0.1, 100.0, "%.1f", 0);
            igDragFloat3(
                "Translation", (float*)&t->translation, 0.1, -100.0, 100.0, "%.2f", 0
            );

            Vector3 e = Vector3Scale(QuaternionToEuler(t->rotation), RAD2DEG);
            igDragFloat3("Rotation", (float*)&e, 5.0, -180.0, 180.0, "%.2f", 0);
            e = Vector3Scale(e, DEG2RAD);
            t->rotation = QuaternionFromEuler(e.x, e.y, e.z);
        }
    }
    igEnd();  // Inspector
}
