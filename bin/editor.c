#include "../src/scene.h"
#include "../src/drawing.h"
#include "../src/math.h"
#include "../src/cimgui_utils.h"
#include "rcamera.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#define RAYGIZMO_IMPLEMENTATION
#include "raygizmo.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
#define MAX_N_COLLISION_INFOS 64

typedef enum EntityType {
    CAMERA_SHELL_TYPE,
    GOLOVA_TYPE,
    BOARD_TYPE,
    ITEM_TYPE,
} EntityType;

typedef struct CollisionInfo {
    RayCollision collision;
    Transform *transform;
    BoundingBox box;
    EntityType entity_type;
} CollisionInfo;

typedef struct CameraShell {
    Transform transform;
    Material material;
    Mesh mesh;
} CameraShell;

static RenderTexture2D FULL_SCREEN;
static RenderTexture2D PREVIEW_SCREEN;
static RGizmo GIZMO;
static Camera3D CAMERA;
static CameraShell CAMERA_SHELL;

static size_t N_COLLISION_INFOS;
static CollisionInfo *PICKED_COLLISION_INFO;
static CollisionInfo COLLISION_INFOS[MAX_N_COLLISION_INFOS];

static bool IS_MMB_DOWN;
static bool IS_LMB_PRESSED;
static bool IS_SHIFT_DOWN;
static float MOUSE_WHEEL_MOVE;
static Vector2 MOUSE_POSITION;
static Vector2 MOUSE_DELTA;

static int IG_ID;
static bool IS_IG_INTERACTED;

static void update_collision_infos(void);
static void update_input(void);
static void update_mouse_picking(void);
static void update_camera_shell(void);
static void update_camera(void);
static void update_gizmo(void);
static void set_board_n_items(int n_items);
static void draw_editor_grid(void);
static void draw_camera_shell(void);
static void draw_imgui(void);

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Editor");
    SetTargetFPS(60);
    load_scene();
    load_imgui();

    FULL_SCREEN = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    PREVIEW_SCREEN = LoadRenderTexture(SCREEN_WIDTH / 3, SCREEN_HEIGHT / 3);
    GIZMO = rgizmo_create();

    CAMERA_SHELL.transform = get_default_transform();
    CAMERA_SHELL.transform.translation = SCENE.camera.position;
    CAMERA_SHELL.mesh = GenMeshSphere(0.2, 16, 16);
    CAMERA_SHELL.material = LoadMaterialDefault();
    CAMERA_SHELL.material.maps[0].color = RAYWHITE;

    CAMERA.fovy = 60.0;
    CAMERA.position = (Vector3){5.0, 5.0, 5.0};
    CAMERA.projection = CAMERA_PERSPECTIVE;
    CAMERA.up = (Vector3){0.0, 1.0, 0.0};


    while (!WindowShouldClose()) {
        update_collision_infos();
        update_input();
        update_scene();
        update_gizmo();
        update_camera();
        update_camera_shell();
        update_mouse_picking();
  
        // Draw main editor screen
        BeginTextureMode(FULL_SCREEN);
        ClearBackground(DARKGRAY);
        rlDisableBackfaceCulling();

        BeginMode3D(CAMERA);
        draw_editor_grid();
        EndMode3D();

        BeginMode3D(CAMERA);
        draw_scene();
        EndMode3D();

        BeginMode3D(CAMERA);
        draw_camera_shell();
        EndMode3D();

        BeginMode3D(CAMERA);
        if (PICKED_COLLISION_INFO && PICKED_COLLISION_INFO->transform) {
            rgizmo_draw(GIZMO, CAMERA, PICKED_COLLISION_INFO->transform->translation);
        }
        EndMode3D();

        begin_imgui();
        draw_imgui();
        end_imgui();

        EndTextureMode();

        // Draw scene preview screen
        BeginTextureMode(PREVIEW_SCREEN);
        ClearBackground(BLACK);
        rlEnableBackfaceCulling();
        BeginMode3D(SCENE.camera);
        draw_scene();
        EndMode3D();
        EndTextureMode();

        // Blit screens
        BeginDrawing();
        ClearBackground(BLANK);
        draw_screen(FULL_SCREEN);
        draw_screen_top_right(PREVIEW_SCREEN);
        EndDrawing();
    }

    return 0;
}

static void update_collision_infos(void) {
    N_COLLISION_INFOS = 0;

    COLLISION_INFOS[N_COLLISION_INFOS].entity_type = GOLOVA_TYPE;
    COLLISION_INFOS[N_COLLISION_INFOS].transform = &SCENE.golova.transform;
    COLLISION_INFOS[N_COLLISION_INFOS++].box = GetMeshBoundingBox(SCENE.golova.mesh);

    COLLISION_INFOS[N_COLLISION_INFOS].entity_type = BOARD_TYPE;
    COLLISION_INFOS[N_COLLISION_INFOS].transform = &SCENE.board.transform;
    COLLISION_INFOS[N_COLLISION_INFOS++].box = GetMeshBoundingBox(SCENE.board.mesh);

    COLLISION_INFOS[N_COLLISION_INFOS].entity_type = CAMERA_SHELL_TYPE;
    COLLISION_INFOS[N_COLLISION_INFOS].transform = &CAMERA_SHELL.transform;
    COLLISION_INFOS[N_COLLISION_INFOS++].box = GetMeshBoundingBox(CAMERA_SHELL.mesh);

    for (size_t i = 0; i < SCENE.board.n_items; ++i) {
        Item *item = &SCENE.board.items[i];
        BoundingBox box = GetMeshBoundingBox(item->mesh);
        box.max = Vector3Transform(box.max, item->matrix);
        box.min = Vector3Transform(box.min, item->matrix);

        COLLISION_INFOS[N_COLLISION_INFOS].entity_type = ITEM_TYPE;
        COLLISION_INFOS[N_COLLISION_INFOS].transform = NULL;
        COLLISION_INFOS[N_COLLISION_INFOS++].box = box;
    }
}

static void update_input(void) {
    IS_MMB_DOWN = IsMouseButtonDown(2);
    IS_LMB_PRESSED = IsMouseButtonPressed(0);
    IS_SHIFT_DOWN = IsKeyDown(KEY_LEFT_SHIFT);
    MOUSE_POSITION = GetMousePosition();
    MOUSE_DELTA = GetMouseDelta();
    MOUSE_WHEEL_MOVE = GetMouseWheelMove();
}

static void update_gizmo(void) {
    if (!PICKED_COLLISION_INFO || !PICKED_COLLISION_INFO->transform) return;

    Transform* t = PICKED_COLLISION_INFO->transform;
    rgizmo_update(&GIZMO, CAMERA, t->translation);
    t->translation = Vector3Add(t->translation, GIZMO.update.translation);
    t->rotation = QuaternionMultiply(
        QuaternionFromAxisAngle(GIZMO.update.axis, GIZMO.update.angle), t->rotation
    );
}

static void update_camera(void) {
    static float rot_speed = 0.003f;
    static float move_speed = 0.01f;
    static float zoom_speed = 1.0f;

    if (IS_MMB_DOWN && IS_SHIFT_DOWN) {
        // Shift + MMB + mouse move -> change the camera position in the
        // right-direction plane
        CameraMoveRight(&CAMERA, -move_speed * MOUSE_DELTA.x, true);

        Vector3 right = GetCameraRight(&CAMERA);
        Vector3 up = Vector3CrossProduct(
            Vector3Subtract(CAMERA.position, CAMERA.target), right
        );
        up = Vector3Scale(Vector3Normalize(up), move_speed * MOUSE_DELTA.y);
        CAMERA.position = Vector3Add(CAMERA.position, up);
        CAMERA.target = Vector3Add(CAMERA.target, up);
    } else if (IS_MMB_DOWN) {
        // Rotate the camera around the look-at point
        CameraYaw(&CAMERA, -rot_speed * MOUSE_DELTA.x, true);
        CameraPitch(&CAMERA, rot_speed * MOUSE_DELTA.y, true, true, false);
    }

    // Bring camera closer (or move away), to the look-at point
    CameraMoveToTarget(&CAMERA, -MOUSE_WHEEL_MOVE * zoom_speed);
}

static void update_mouse_picking(void) {
    if (!IS_LMB_PRESSED || GIZMO.state != RGIZMO_STATE_COLD) return;

    PICKED_COLLISION_INFO = NULL;
    Ray ray = GetMouseRay(MOUSE_POSITION, CAMERA);

    for (size_t id = 0; id < N_COLLISION_INFOS; ++id) {
        CollisionInfo *info = &COLLISION_INFOS[id];
        BoundingBox box = info->box;
        if (info->transform) {
            Matrix mat = get_transform_matrix(*info->transform);
            box.max = Vector3Transform(box.max, mat);
            box.min = Vector3Transform(box.min, mat);
        }

        info->collision = GetRayCollisionBox(ray, box);
        if (!info->collision.hit) continue;

        if (!PICKED_COLLISION_INFO) {
            PICKED_COLLISION_INFO = info;
            continue;
        }

        if (info->collision.distance < PICKED_COLLISION_INFO->collision.distance) {
            PICKED_COLLISION_INFO = info;
        }
    }
}

static void update_camera_shell(void) {
    SCENE.camera.position = CAMERA_SHELL.transform.translation;
    Vector3 dir = Vector3RotateByQuaternion(
        (Vector3){0.0, 0.0, -1.0}, CAMERA_SHELL.transform.rotation
    );
    SCENE.camera.target = Vector3Add(SCENE.camera.position, dir);
}

static void set_board_n_items(int n_items) {
    Board *b = &SCENE.board;

    if (n_items < 0 || n_items > MAX_N_BOARD_ITEMS || n_items == b->n_items) return;

    while (n_items < b->n_items) {
        b->n_items -= 1;
        Item *item = &b->items[b->n_items];
        UnloadMaterial(item->material);
        UnloadMesh(item->mesh);
        *item = (Item){0};
    }

    while (n_items > b->n_items) {
        Item *item = &b->items[b->n_items];
        item->material = LoadMaterialDefault();
        item->mesh = GenMeshPlane(1.0, 1.0, 2, 2);
        b->n_items += 1;
    }
}

static void draw_editor_grid(void) {
    rlSetLineWidth(2.0);
    DrawGrid(10.0, 5.0);
    float d = 25.0f;
    DrawLine3D((Vector3){-d, 0.0f, 0.0f}, (Vector3){d, 0.0f, 0.0f}, RED);
    DrawLine3D((Vector3){0.0f, -d, 0.0f}, (Vector3){0.0f, d, 0.0f}, GREEN);
    DrawLine3D((Vector3){0.0f, 0.0f, -d}, (Vector3){0.0f, 0.0f, d}, DARKBLUE);
    EndMode3D();
}

static void draw_camera_shell(void) {
    draw_mesh_t(CAMERA_SHELL.transform, CAMERA_SHELL.material, CAMERA_SHELL.mesh);

    rlSetLineWidth(5.0);
    Vector3 start = SCENE.camera.position;
    Vector3 target = SCENE.camera.target;
    Vector3 dir = Vector3Normalize(Vector3Subtract(target, start));
    Vector3 end = Vector3Add(start, Vector3Scale(dir, 1.0));
    DrawLine3D(start, end, RAYWHITE);
}

static void draw_imgui(void) {
    IG_ID = 1;
    ImGuiIO* io = igGetIO();
    IS_IG_INTERACTED = io->WantCaptureMouse || io->WantCaptureKeyboard;

    ig_fix_window_top_left();
    if (igBegin("Inspector", NULL, 0)) {
        if (ig_collapsing_header("Camera", true)) {
            igDragFloat("FOV", &SCENE.camera.fovy, 1.0, 10.0, 170.0, "%.1f", 0);
        }

        if (ig_collapsing_header("Board", true)) {
            Board* b = &SCENE.board;
            igInputText("Rule", b->rule, MAX_RULE_LENGTH, 0, 0, NULL);
            igDragFloat("Board scale", &b->board_scale, 0.01, 0.01, 1.0, "%.3f", 0);
            igDragFloat("Item Scale", &b->item_scale, 0.01, 0.01, 1.0, "%.3f", 0);
            igDragFloat("Item elevation", &b->item_elevation, 0.01, 0.01, 1.0, "%.3f", 0);

            int n_items = b->n_items;
            igInputInt("N items", &n_items, 1, 1, 0);
            set_board_n_items(n_items);
        }
    }
    igEnd();
}
