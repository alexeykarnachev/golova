#include "editor.h"

#include "camera.h"
#include "collisions.h"
#include "raylib.h"
#include "raymath.h"
#include <stdbool.h>

#define RAYGIZMO_IMPLEMENTATION
#include "raygizmo.h"

static Camera3D CAMERA;
static RGizmo GIZMO;
static Entity* PICKED_ENTITY;

static void draw_grid(void);

void load_editor(void) {
    CAMERA.fovy = 60.0;
    CAMERA.projection = CAMERA_PERSPECTIVE;
    CAMERA.up = (Vector3){0.0, 1.0, 0.0};
    CAMERA.position = (Vector3){5.0, 5.0, 5.0};

    GIZMO = rgizmo_create();
}

void update_editor(void) {
    update_orbital_camera(&CAMERA);

    bool is_lmb_pressed = IsMouseButtonPressed(0);

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
            QuaternionFromAxisAngle(GIZMO.update.axis, GIZMO.update.angle),
            t->rotation
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
