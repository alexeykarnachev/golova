#include "../src/cimgui_utils.h"
#include "../src/drawing.h"
#include "../src/math.h"
#include "../src/nfd_utils.h"
#include "../src/scene.h"
#include "../src/utils.h"
#include "raylib.h"
#include "raymath.h"
#include "rcamera.h"
#include "rlgl.h"
#include <string.h>

#define RAYGIZMO_IMPLEMENTATION
#include "raygizmo.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

// #define SCREEN_WIDTH 1024
// #define SCREEN_HEIGHT 768
#define SCREEN_WIDTH 2560
#define SCREEN_HEIGHT 1440
#define MAX_N_COLLISION_INFOS 256

typedef enum EntityType {
    NULL_TYPE = 0,
    CAMERA_SHELL_TYPE,
    GOLOVA_TYPE,
    BOARD_TYPE,
    ITEM_TYPE,
    TREE_TYPE,
} EntityType;

typedef struct CollisionInfo {
    RayCollision collision;
    Matrix matrix;
    Mesh mesh;
    Transform *transform;
    EntityType entity_type;
    void *entity;
} CollisionInfo;

typedef struct CameraShell {
    Transform transform;
    Material material;
    Mesh mesh;
    Camera3D *camera;
} CameraShell;

static nfdfilteritem_t NFD_TEXTURE_FILTER[1] = {{"Texture", "png"}};
static nfdfilteritem_t NFD_FOREST_FILTER[1] = {{"Forest", "fst"}};

static RenderTexture2D FULL_SCREEN;
static RenderTexture2D PREVIEW_SCREEN;
static RenderTexture2D PREVIEW_SCREEN_POSTFX;
static RGizmo GIZMO;
static Camera3D CAMERA;
static CameraShell CAMERA_SHELL;
static CameraShell LIGHT_CAMERA_SHELL;

static char SCENE_FILE_PATH[2048];

static size_t N_COLLISION_INFOS;
static CollisionInfo *PICKED_COLLISION_INFO;
static CollisionInfo COLLISION_INFOS[MAX_N_COLLISION_INFOS];

static bool IS_MMB_DOWN;
static bool IS_LMB_PRESSED;
static bool IS_SHIFT_DOWN;
static bool IS_DELETE_PRESSED;
static float MOUSE_WHEEL_MOVE;
static Vector2 MOUSE_POSITION;
static Vector2 MOUSE_DELTA;

static int IG_ID;
static bool IS_IG_INTERACTED;

static bool WITH_SHADOWS = true;
static bool WITH_BLUR = false;
static int CLEAR_COLOR[3];

static Transform *get_picked_transform(void);
static void *get_picked_entity(void);
static EntityType get_picked_entity_type(void);

static void reset_camera_shells();
static void update_editor(void);
static void set_board_values(int n_items, int n_hits_required, int n_misses_allowed);
static void draw_editor_grid(void);
static void draw_camera_shells(void);
static void draw_item_boxes(void);
static void draw_imgui(void);

static bool load_sprite(
    const char *search_path,
    char *dst_name,
    Transform *dst_transform,
    Texture2D *dst_texture,
    Mesh *dst_mesh
);

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Editor");
    SetTargetFPS(60);

    init_core(SCREEN_WIDTH, SCREEN_HEIGHT);
    load_scene(NULL);
    load_imgui();

    FULL_SCREEN = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    PREVIEW_SCREEN = LoadRenderTexture(SCREEN_WIDTH / 3, SCREEN_HEIGHT / 3);
    PREVIEW_SCREEN_POSTFX = LoadRenderTexture(SCREEN_WIDTH / 3, SCREEN_HEIGHT / 3);
    GIZMO = rgizmo_create();

    CAMERA_SHELL.mesh = GenMeshSphere(0.15, 16, 16);
    CAMERA_SHELL.material = LoadMaterialDefault();
    CAMERA_SHELL.material.maps[0].color = RAYWHITE;
    CAMERA_SHELL.camera = &SCENE.camera;

    LIGHT_CAMERA_SHELL.mesh = GenMeshSphere(0.15, 16, 16);
    LIGHT_CAMERA_SHELL.material = LoadMaterialDefault();
    LIGHT_CAMERA_SHELL.material.maps[0].color = YELLOW;
    LIGHT_CAMERA_SHELL.camera = &SCENE.light_camera;

    reset_camera_shells();

    CAMERA.fovy = 60.0;
    CAMERA.position = (Vector3){5.0, 5.0, 5.0};
    CAMERA.projection = CAMERA_PERSPECTIVE;
    CAMERA.up = (Vector3){0.0, 1.0, 0.0};

    while (!WindowShouldClose()) {
        update_editor();

        bool sort_trees = get_picked_entity_type() != TREE_TYPE;

        // Draw main editor screen
        draw_scene(FULL_SCREEN, DARKGRAY, CAMERA, WITH_SHADOWS, false, true, sort_trees);

        BeginTextureMode(FULL_SCREEN);
        rlDisableBackfaceCulling();

        BeginMode3D(CAMERA);
        rlSetLineWidth(2.0);
        draw_editor_grid();
        EndMode3D();

        BeginMode3D(CAMERA);
        rlSetLineWidth(3.0);
        draw_camera_shells();
        draw_item_boxes();
        EndMode3D();

        BeginMode3D(CAMERA);
        if (get_picked_transform()) {
            rgizmo_draw(GIZMO, CAMERA, get_picked_transform()->translation);
        }
        EndMode3D();

        draw_imgui();

        EndTextureMode();

        // Draw scene preview screen
        rlEnableBackfaceCulling();
        Color clear_color = {CLEAR_COLOR[0], CLEAR_COLOR[1], CLEAR_COLOR[2], 255};
        draw_scene(
            PREVIEW_SCREEN,
            clear_color,
            SCENE.camera,
            WITH_SHADOWS,
            false,
            true,
            sort_trees
        );

        BeginTextureMode(PREVIEW_SCREEN_POSTFX);
        draw_postfx(PREVIEW_SCREEN.texture, WITH_BLUR);
        EndTextureMode();

        // Blit screens
        BeginDrawing();
        ClearBackground(BLANK);
        draw_screen(FULL_SCREEN);
        draw_screen_top_right(PREVIEW_SCREEN_POSTFX);
        EndDrawing();
    }

    return 0;
}

static Transform *get_picked_transform(void) {
    if (!PICKED_COLLISION_INFO) return NULL;
    return PICKED_COLLISION_INFO->transform;
}

static void *get_picked_entity(void) {
    if (!PICKED_COLLISION_INFO) return NULL;
    return PICKED_COLLISION_INFO->entity;
}

static EntityType get_picked_entity_type(void) {
    if (!PICKED_COLLISION_INFO) return NULL_TYPE;
    return PICKED_COLLISION_INFO->entity_type;
}

static void reset_camera_shells(void) {
    CameraShell *shells[2] = {&CAMERA_SHELL, &LIGHT_CAMERA_SHELL};
    for (size_t i = 0; i < 2; ++i) {
        CameraShell *shell = shells[i];
        shell->transform = get_default_transform();
        shell->transform.translation = shell->camera->position;
        Vector3 dir = Vector3Subtract(shell->camera->target, shell->camera->position);
        shell->transform.rotation = QuaternionFromVector3ToVector3(
            (Vector3){0.0, 0.0, -1.0}, Vector3Normalize(dir)
        );
    }
}

static void delete_tree(size_t idx) {
    Tree *tree = &SCENE.forest.trees[idx];
    UnloadTexture(tree->texture);
    UnloadMesh(tree->mesh);
    SCENE.forest.n_trees -= 1;
    size_t n_move = SCENE.forest.n_trees - idx;
    if (n_move > 0) {
        size_t size_move = n_move * sizeof(SCENE.forest.trees[0]);
        memmove(&SCENE.forest.trees[idx], &SCENE.forest.trees[idx + 1], size_move);
    }
}

static void unpick(void) {
    PICKED_COLLISION_INFO = NULL;
    GIZMO.state = RGIZMO_STATE_COLD;
}

static void update_editor(void) {
    IS_MMB_DOWN = IsMouseButtonDown(2) && !IS_IG_INTERACTED;
    IS_LMB_PRESSED = IsMouseButtonPressed(0) && !IS_IG_INTERACTED;
    IS_SHIFT_DOWN = IsKeyDown(KEY_LEFT_SHIFT) && !IS_IG_INTERACTED;
    IS_DELETE_PRESSED = IsKeyPressed(KEY_DELETE);
    MOUSE_POSITION = GetMousePosition();
    MOUSE_DELTA = GetMouseDelta();
    MOUSE_WHEEL_MOVE = GetMouseWheelMove() * (int)(!IS_IG_INTERACTED);

    // ------------------------------------------------------------------
    // Scene save/load
    bool is_save_pressed = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S);
    bool is_load_pressed = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_O);
    static nfdfilteritem_t filter[1] = {{"Scene", "scn"}};

    if (is_save_pressed) {
        if (SCENE_FILE_PATH[0] == '\0' || IsKeyDown(KEY_LEFT_SHIFT)) {
            char *fp = save_nfd("resources/scenes", filter, 1);
            if (fp != NULL) {
                strcpy(SCENE_FILE_PATH, fp);
                NFD_FreePathN(fp);
            }
        }

        if (SCENE_FILE_PATH[0] != '\0') save_scene(SCENE_FILE_PATH);
    } else if (is_load_pressed) {
        char *fp = open_nfd("resources/scenes", filter, 1);
        if (fp != NULL) {
            load_scene(fp);
            reset_camera_shells();
            strcpy(SCENE_FILE_PATH, fp);
        }
    }

    // ------------------------------------------------------------------
    // Collision infos
    N_COLLISION_INFOS = 0;

    COLLISION_INFOS[N_COLLISION_INFOS].entity_type = GOLOVA_TYPE;
    COLLISION_INFOS[N_COLLISION_INFOS].transform = &SCENE.golova.transform;
    COLLISION_INFOS[N_COLLISION_INFOS++].mesh = SCENE.golova.idle.mesh;

    COLLISION_INFOS[N_COLLISION_INFOS].entity_type = BOARD_TYPE;
    COLLISION_INFOS[N_COLLISION_INFOS].transform = &SCENE.board.transform;
    COLLISION_INFOS[N_COLLISION_INFOS++].mesh = SCENE.board.mesh;

    COLLISION_INFOS[N_COLLISION_INFOS].entity_type = CAMERA_SHELL_TYPE;
    COLLISION_INFOS[N_COLLISION_INFOS].transform = &CAMERA_SHELL.transform;
    COLLISION_INFOS[N_COLLISION_INFOS++].mesh = CAMERA_SHELL.mesh;

    COLLISION_INFOS[N_COLLISION_INFOS].entity_type = CAMERA_SHELL_TYPE;
    COLLISION_INFOS[N_COLLISION_INFOS].transform = &LIGHT_CAMERA_SHELL.transform;
    COLLISION_INFOS[N_COLLISION_INFOS++].mesh = LIGHT_CAMERA_SHELL.mesh;

    for (size_t i = 0; i < SCENE.board.n_items; ++i) {
        Item *item = &SCENE.board.items[i];

        COLLISION_INFOS[N_COLLISION_INFOS].entity_type = ITEM_TYPE;
        COLLISION_INFOS[N_COLLISION_INFOS].transform = NULL;
        COLLISION_INFOS[N_COLLISION_INFOS].matrix = item->matrix;
        COLLISION_INFOS[N_COLLISION_INFOS].entity = item;
        COLLISION_INFOS[N_COLLISION_INFOS++].mesh = SCENE.board.item_mesh;
    }

    for (size_t i = 0; i < SCENE.forest.n_trees; ++i) {
        Tree *tree = &SCENE.forest.trees[i];

        COLLISION_INFOS[N_COLLISION_INFOS].entity_type = TREE_TYPE;
        COLLISION_INFOS[N_COLLISION_INFOS].transform = &tree->transform;
        COLLISION_INFOS[N_COLLISION_INFOS].entity = tree;
        COLLISION_INFOS[N_COLLISION_INFOS++].mesh = tree->mesh;
    }

    // -------------------------------------------------------------------
    // Board items
    Board *b = &SCENE.board;
    Transform t = b->transform;
    t.scale = Vector3Scale(Vector3One(), t.scale.x);

    size_t n_items = b->n_items;

    int n_rows = sqrt(n_items);
    int n_cols = ceil((float)n_items / n_rows);
    for (size_t i = 0; i < n_items; ++i) {
        Item *item = &b->items[i];

        int i_row = i / n_cols;
        int i_col = i % n_cols;

        float z = n_rows > 1 ? (float)i_row / (n_rows - 1) - 0.5 : 0.0;
        float x = n_cols > 1 ? (float)i_col / (n_cols - 1) - 0.5 : 0.0;

        rlPushMatrix();
        {
            rlMultMatrixf(MatrixToFloat(get_transform_matrix(t)));
            rlScalef(b->board_scale, b->board_scale, b->board_scale);
            rlTranslatef(x, 0.0, z);
            rlScalef(b->item_scale, b->item_scale, b->item_scale);
            rlTranslatef(0.0, b->item_elevation, 0.0);
            rlRotatef(90.0, 1.0, 0.0, 0.0);
            item->matrix = rlGetMatrixTransform();
        }
        rlPopMatrix();
    }

    // -------------------------------------------------------------------
    // Editor camera
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

    // -------------------------------------------------------------------
    // Camera shells
    CameraShell *shells[2] = {&CAMERA_SHELL, &LIGHT_CAMERA_SHELL};
    for (size_t i = 0; i < 2; ++i) {
        CameraShell *shell = shells[i];
        shell->camera->position = shell->transform.translation;
        Vector3 dir = Vector3RotateByQuaternion(
            (Vector3){0.0, 0.0, -1.0}, shell->transform.rotation
        );
        shell->camera->target = Vector3Add(shell->camera->position, dir);
    }

    // -------------------------------------------------------------------
    // Picking
    if (IS_LMB_PRESSED && GIZMO.state == RGIZMO_STATE_COLD) {
        unpick();
        Ray ray = GetMouseRay(MOUSE_POSITION, CAMERA);

        for (size_t id = 0; id < N_COLLISION_INFOS; ++id) {
            CollisionInfo *info = &COLLISION_INFOS[id];

            Matrix matrix;
            matrix = info->transform ? get_transform_matrix(*info->transform)
                                     : info->matrix;
            info->collision = GetRayCollisionMesh(ray, info->mesh, matrix);

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

    // -------------------------------------------------------------------
    // Forest
    if (IS_DELETE_PRESSED && PICKED_COLLISION_INFO->entity) {
        for (size_t i = 0; i < SCENE.forest.n_trees; ++i) {
            if (&SCENE.forest.trees[i] == PICKED_COLLISION_INFO->entity) {
                delete_tree(i);
                unpick();
                break;
            }
        }
    }

    // -------------------------------------------------------------------
    // Gizmo
    Transform *picked_transform = get_picked_transform();
    if (picked_transform) {
        rgizmo_update(&GIZMO, CAMERA, picked_transform->translation);
        picked_transform->translation = Vector3Add(
            picked_transform->translation, GIZMO.update.translation
        );
        picked_transform->rotation = QuaternionMultiply(
            QuaternionFromAxisAngle(GIZMO.update.axis, GIZMO.update.angle),
            picked_transform->rotation
        );
    }
}

static void set_board_values(int n_items, int n_hits_required, int n_misses_allowed) {
    Board *b = &SCENE.board;

    if (n_items < 0 || n_items > MAX_N_BOARD_ITEMS) return;
    while (n_items < b->n_items) {
        b->n_items -= 1;
        Item *item = &b->items[b->n_items];
        UnloadTexture(item->texture);
        *item = (Item){0};
    }
    while (n_items > b->n_items) {
        Item *item = &b->items[b->n_items];
        b->n_items += 1;
    }

    n_hits_required = CLAMP(n_hits_required, 0, b->n_items);
    n_misses_allowed = CLAMP(n_misses_allowed, 0, b->n_items);
    if (n_hits_required + n_misses_allowed > b->n_items) return;
    b->n_hits_required = n_hits_required;
    b->n_misses_allowed = n_misses_allowed;
}

static void draw_editor_grid(void) {
    DrawGrid(10.0, 5.0);
    float d = 25.0f;
    DrawLine3D((Vector3){-d, 0.0f, 0.0f}, (Vector3){d, 0.0f, 0.0f}, RED);
    DrawLine3D((Vector3){0.0f, -d, 0.0f}, (Vector3){0.0f, d, 0.0f}, GREEN);
    DrawLine3D((Vector3){0.0f, 0.0f, -d}, (Vector3){0.0f, 0.0f, d}, DARKBLUE);
    EndMode3D();
}

static void draw_camera_shells(void) {
    CameraShell *shells[2] = {&CAMERA_SHELL, &LIGHT_CAMERA_SHELL};
    for (size_t i = 0; i < 2; ++i) {
        CameraShell *shell = shells[i];
        draw_mesh_t(shell->transform, shell->material, shell->mesh);

        Vector3 start = shell->camera->position;
        Vector3 target = shell->camera->target;
        Vector3 dir = Vector3Normalize(Vector3Subtract(target, start));
        Vector3 end = Vector3Add(start, Vector3Scale(dir, 0.8));
        DrawLine3D(start, end, shell->material.maps[0].color);
    }
}

static void draw_item_boxes(void) {
    BoundingBox box = GetMeshBoundingBox(SCENE.board.item_mesh);
    for (size_t i = 0; i < SCENE.board.n_items; ++i) {
        Item *item = &SCENE.board.items[i];
        Color color = item->is_correct ? GREEN : RED;
        rlPushMatrix();
        rlMultMatrixf(MatrixToFloat(item->matrix));
        DrawBoundingBox(box, color);
        rlPopMatrix();
    }
}

static void draw_imgui(void) {
    int screen_height = GetScreenHeight();
    begin_imgui();

    IG_ID = 1;
    ImGuiIO *io = igGetIO();
    IS_IG_INTERACTED = io->WantCaptureMouse || io->WantCaptureKeyboard;

    ig_fix_window_top_left();
    igSetNextWindowSize((ImVec2){0, screen_height - 30}, 0);
    if (igBegin("Inspector", NULL, 0)) {
        if (ig_collapsing_header("Debug", true)) {
            static char name[MAX_NAME_LENGTH];
            get_file_name(name, SCENE_FILE_PATH, true);
            igText("SCENE: %s", name);
            igCheckbox("WITH_SHADOWS", &WITH_SHADOWS);
            igCheckbox("WITH_BLUR", &WITH_BLUR);
            igDragInt3("CLEAR_COLOR", CLEAR_COLOR, 1, 0, 255, "%d", 0);
        }

        if (ig_collapsing_header("Camera", true)) {
            igDragFloat("FOV##camera", &SCENE.camera.fovy, 1.0, 10.0, 170.0, "%.1f", 0);
        }

        if (ig_collapsing_header("Light camera", true)) {
            igDragFloat(
                "FOV##light_camera", &SCENE.light_camera.fovy, 1.0, 1.0, 170.0, "%.1f", 0
            );
            if (SCENE.light_camera.fovy == 1.0) {
                SCENE.light_camera.projection = CAMERA_ORTHOGRAPHIC;
            } else {
                SCENE.light_camera.projection = CAMERA_PERSPECTIVE;
            }
        }

        if (ig_collapsing_header("Golova", true)) {
            Golova *g = &SCENE.golova;
            igSeparatorText("Eyes");
            igDragFloat(
                "Uplift##eyes", &g->eyes_idle_uplift, 0.001, -0.02, 0.08, "%.3f", 0
            );
            igDragFloat("Scale##eyes", &g->eyes_idle_scale, 0.001, 0.001, 0.2, "%.3f", 0);
            igDragFloat("Shift##eyes", &g->eyes_idle_shift, 0.001, -0.2, 0.2, "%.3f", 0);
            igDragFloat("Spread##eyes", &g->eyes_idle_spread, 0.001, 0.1, 0.5, "%.3f", 0);
        }

        if (ig_collapsing_header("Board", true)) {
            Board *b = &SCENE.board;
            igInputText("Rule", b->rule, MAX_RULE_LENGTH, 0, 0, NULL);
            igInputInt("N hint items", &SCENE.board.n_hint_items, 1, 1, 0);
            for (int i = 0; i < SCENE.board.n_hint_items; ++i) {
                if (i > 0) igSameLine(0, 5);
                Item *item = &SCENE.board.hint_items[i];
                Texture2D texture = item->texture;
                int texture_id = texture.id;

                igBeginGroup();
                igText(item->name);
                igPushID_Int(IG_ID++);
                bool is_clicked = igImageButton(
                    "",
                    (ImTextureID)(long)texture_id,
                    (ImVec2){64.0, 64.0},
                    (ImVec2){0.0, 0.0},
                    (ImVec2){1.0, 1.0},
                    (ImVec4){0.0, 0.0, 0.0, 1.0},
                    (ImVec4){1.0, 1.0, 1.0, 1.0}
                );
                igPopID();
                igEndGroup();

                if (is_clicked) {
                    load_sprite(
                        "resources/items/sprites", item->name, 0, &item->texture, 0
                    );
                }
            }

            igDragFloat("Board scale", &b->board_scale, 0.01, 0.01, 1.0, "%.3f", 0);
            igDragFloat("Item Scale", &b->item_scale, 0.01, 0.01, 1.0, "%.3f", 0);
            igDragFloat("Item elevation", &b->item_elevation, 0.01, 0.01, 1.0, "%.3f", 0);

            int n_items = b->n_items;
            int n_hits_required = b->n_hits_required;
            int n_misses_allowed = b->n_misses_allowed;
            if (igInputInt("N items", &n_items, 1, 1, 0)) {
                unpick();
            }
            igInputInt("N hits required", &n_hits_required, 1, 1, 0);
            igInputInt("N misses allowed", &n_misses_allowed, 1, 1, 0);
            set_board_values(n_items, n_hits_required, n_misses_allowed);
        }

        if (ig_collapsing_header("Item", true) && get_picked_entity_type() == ITEM_TYPE) {
            Item *item = get_picked_entity();
            Texture2D texture = item->texture;
            int texture_id = texture.id;

            bool is_clicked = igImageButton(
                "##item_texture",
                (ImTextureID)(long)texture_id,
                (ImVec2){128.0, 128.0},
                (ImVec2){0.0, 0.0},
                (ImVec2){1.0, 1.0},
                (ImVec4){0.0, 0.0, 0.0, 1.0},
                (ImVec4){1.0, 1.0, 1.0, 1.0}
            );

            if (is_clicked) {
                load_sprite("resources/items/sprites", item->name, 0, &item->texture, 0);
            }

            igSameLine(0.0, 5.0);
            igBeginGroup();
            igText(item->name[0] == '\0' ? "???" : item->name);
            igCheckbox("Is correct", &item->is_correct);
            igEndGroup();
        }

        if (ig_collapsing_header("Transform", true) && get_picked_transform()) {
            Transform *t = get_picked_transform();
            igDragFloat3("Scale", (float *)&t->scale, 0.1, 0.1, 100.0, "%.1f", 0);
            igDragFloat3(
                "Translation", (float *)&t->translation, 0.1, -100.0, 100.0, "%.2f", 0
            );

            Vector3 e = Vector3Scale(QuaternionToEuler(t->rotation), RAD2DEG);
            igDragFloat3("Rotation", (float *)&e, 5.0, -180.0, 180.0, "%.2f", 0);
            e = Vector3Scale(e, DEG2RAD);
            t->rotation = QuaternionFromEuler(e.x, e.y, e.z);
        }

        if (ig_collapsing_header("Forest", true)) {
            Forest *f = &SCENE.forest;
            igText("%s", f->name);
            igText("N trees: %d", f->n_trees);

            if (igButton("Save##forest", (ImVec2){0.0, 0.0})) {
                char *fp = save_nfd("resources/forests", NFD_FOREST_FILTER, 1);
                if (fp != NULL) {
                    get_file_name(f->name, fp, true);
                    save_forest(f, fp);
                    NFD_FreePathN(fp);
                }
            }

            igSameLine(0, 5);
            if (igButton("Load##forest", (ImVec2){0.0, 0.0})) {
                char *fp = open_nfd("resources/forests", NFD_FOREST_FILTER, 1);
                if (fp != NULL) {
                    load_forest(f, fp);
                    NFD_FreePathN(fp);
                }
            }

            igSeparatorText("Trees");
            if (f->n_trees < MAX_N_FOREST_TREES
                && igButton("New tree", (ImVec2){0.0, 0.0})) {
                Tree *tree = &f->trees[f->n_trees];
                f->n_trees += load_sprite(
                    "resources/trees/sprites",
                    tree->name,
                    &tree->transform,
                    &tree->texture,
                    &tree->mesh
                );
                tree->transform.rotation = QuaternionMultiply(
                    tree->transform.rotation,
                    QuaternionFromEuler(DEG2RAD * 90.0, 0.0, 0.0)
                );
                tree->matrix = MatrixIdentity();
            }
            for (size_t i = 0; i < f->n_trees; ++i) {
                igPushID_Int(IG_ID++);

                if (i % 3 != 0) igSameLine(0, 5);
                igBeginGroup();

                Tree *tree = &f->trees[i];
                igText(tree->name);

                Texture2D texture = tree->texture;
                int texture_id = texture.id;

                bool is_clicked = igImageButton(
                    "Texture",
                    (ImTextureID)(long)texture_id,
                    (ImVec2){64.0, 64.0},
                    (ImVec2){0.0, 0.0},
                    (ImVec2){1.0, 1.0},
                    (ImVec4){1.0, 1.0, 1.0, 1.0},
                    (ImVec4){1.0, 1.0, 1.0, 1.0}
                );

                if (is_clicked) {
                    load_sprite(
                        "resources/trees/sprites",
                        tree->name,
                        &tree->transform,
                        &tree->texture,
                        &tree->mesh
                    );
                }
                igSameLine(0, 3);

                igBeginGroup();
                if (igButton("Pick", (ImVec2){0.0, 0.0})) {
                    for (size_t i = 0; i < N_COLLISION_INFOS; ++i) {
                        CollisionInfo *info = &COLLISION_INFOS[i];
                        if (info->entity == tree) PICKED_COLLISION_INFO = info;
                    }
                }

                if (igButton("Remove", (ImVec2){0.0, 0.0})) {
                    size_t pop_idx;
                    for (size_t i = 0; i < SCENE.forest.n_trees; ++i) {
                        if (&SCENE.forest.trees[i] == tree) {
                            pop_idx = i;
                            if (PICKED_COLLISION_INFO
                                && PICKED_COLLISION_INFO->entity == tree) {
                                unpick();
                            }
                            break;
                        };
                    }

                    delete_tree(pop_idx);
                }
                igEndGroup();
                igEndGroup();

                igPopID();
            }
        }
    }
    igEnd();
    end_imgui();
}

static bool load_sprite(
    const char *search_path,
    char *dst_name,
    Transform *dst_transform,
    Texture2D *dst_texture,
    Mesh *dst_mesh
) {
    char *fp = open_nfd(search_path, NFD_TEXTURE_FILTER, 1);
    if (fp != NULL) {
        get_file_name(dst_name, fp, true);
        *dst_texture = LoadTexture(fp);
        NFD_FreePathN(fp);

        if (dst_mesh) {
            float aspect = (float)dst_texture->width / dst_texture->height;
            *dst_mesh = GenMeshPlane(aspect, 1.0, 2, 2);
        }

        if (dst_transform) {
            *dst_transform = get_default_transform();
        }
        return true;
    }

    return false;
}
