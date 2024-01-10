#include "../src/cimgui_utils.h"
#include "../src/scene.h"
#include "raylib.h"
#include "raymath.h"
#include "rcamera.h"
#include "rlgl.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAYGIZMO_IMPLEMENTATION
#include "raygizmo.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

typedef enum Mode {
    EDITOR_MODE,
    GAME_MODE,
} Mode;

Mode MODE;

// int SCREEN_WIDTH = 1024;
// int SCREEN_HEIGHT = 768;
int SCREEN_WIDTH = 2560;
int SCREEN_HEIGHT = 1440;

int PICKING_MATERIAL;
int PLANE_MESH;
int SPHERE_MESH;
int FULL_SCREEN;
int PREVIEW_SCREEN;
int PICKING_SCREEN;

int EDITOR_CAMERA;
int GAME_CAMERA;
int GAME_CAMERA_SHELL;

int GOLOVA;
int GROUND;
int ITEM;

RGizmo GIZMO;

bool IS_IMGUI_INTERACTED;
int PICKED_ID = -1;

static void rl_transform(Transform transform) {
    Vector3 axis;
    float angle;
    Vector3 t = transform.translation;
    Vector3 s = transform.scale;
    Quaternion q = transform.rotation;
    QuaternionToAxisAngle(q, &axis, &angle);

    rlTranslatef(t.x, t.y, t.z);
    rlRotatef(angle * RAD2DEG, axis.x, axis.y, axis.z);
    rlScalef(s.x, s.y, s.z);
}

static void draw_scene(bool is_picking) {
    for (size_t id = 0; id < SCENE.entity.n_entities; ++id) {
        bool is_mesh = check_if_entity_has_component(id, MESH_COMPONENT);
        bool is_no_draw = check_if_entity_has_component(id, NO_DRAW_COMPONENT);
        bool skip = is_no_draw || (!is_mesh);
        if (skip) continue;

        Mesh mesh = *get_entity_mesh(id);
        Transform transform = *get_entity_transform(id);

        Material material;
        if (is_picking) {
            material = *get_material(PICKING_MATERIAL);
            material.maps[0].color = (Color){id, 0, 0, 255};
        } else {
            material = *get_entity_material(id);
        }

        rlPushMatrix();
        {
            rl_transform(transform);
            DrawMesh(mesh, material, MatrixIdentity());
        }
        rlPopMatrix();
    }
}

static void draw_game_camera_ray(void) {
    Camera3D* camera = get_camera(GAME_CAMERA);
    Vector3 start = camera->position;
    Vector3 target = camera->target;
    Vector3 dir = Vector3Normalize(Vector3Subtract(target, start));
    Vector3 end = Vector3Add(start, Vector3Scale(dir, 1.0));
    DrawLine3D(start, end, PINK);
}

static void draw_editor_grid(void) {
    DrawGrid(10.0, 5.0);
    float d = 25.0f;

    DrawLine3D((Vector3){-d, 0.0f, 0.0f}, (Vector3){d, 0.0f, 0.0f}, RED);
    DrawLine3D((Vector3){0.0f, -d, 0.0f}, (Vector3){0.0f, d, 0.0f}, GREEN);
    DrawLine3D((Vector3){0.0f, 0.0f, -d}, (Vector3){0.0f, 0.0f, d}, DARKBLUE);
}

static void draw_items(bool is_picking) {
    Mesh mesh = *get_entity_mesh(ITEM);
    Transform transform = *get_entity_transform(GROUND);
    transform.scale = Vector3Scale(Vector3One(), transform.scale.x);

    Board* board = get_loaded_board();
    int n_rows = sqrt(board->n_items);
    int n_cols = n_rows;

    int id = 0;
    for (size_t i = 0; i < n_rows; ++i) {
        float z = (float)i / (n_rows - 1) - 0.5;

        for (size_t j = 0; j < n_cols; ++j, ++id) {
            Material material;
            if (is_picking) {
                material = *get_material(PICKING_MATERIAL);
                material.maps[0].color = (Color){id, 0, 0, 255};
            } else {
                material = *get_entity_material(ITEM);
                material.maps[0].texture = board->items[id].texture;
            }

            float x = (float)j / (n_cols - 1) - 0.5;
            rlPushMatrix();
            {
                rl_transform(transform);
                rlScalef(
                    board->board_scale, board->board_scale, board->board_scale
                );
                rlTranslatef(x, 0.0, z);
                rlScalef(
                    board->item_scale, board->item_scale, board->item_scale
                );
                rlTranslatef(0.0, board->item_elevation, 0.0);
                rlRotatef(90.0, 1.0, 0.0, 0.0);

                DrawMesh(mesh, material, MatrixIdentity());
            }
            rlPopMatrix();
        }
    }
}

static void draw_imgui(void) {
    ImGuiIO* io = igGetIO();
    IS_IMGUI_INTERACTED = io->WantCaptureMouse || io->WantCaptureKeyboard;

    ig_fix_window_top_left();
    // Draw inspector
    if (igBegin("Inspector", NULL, 0)) {
        // Draw camera inspector
        if (ig_collapsing_header("Camera", true)) {
            igDragFloat(
                "FOV",
                &get_camera(GAME_CAMERA)->fovy,
                1.0,
                10.0,
                170.0,
                "%.1f",
                0
            );
        }

        if (ig_collapsing_header("Ground", true)) {
            igSeparatorText("Board");
            if (igBeginListBox("##", (ImVec2){200.0f, 100.0f})) {

                for (size_t i = 0; i < SCENE.n_boards; ++i) {
                    const bool is_selected = (i == SCENE.loaded_board_id);
                    bool is_clicked = igSelectable_Bool(
                        SCENE.board[i].rule,
                        is_selected,
                        0,
                        (ImVec2){0.0f, 0.0f}
                    );
                    if (is_clicked) load_board(i);
                    if (is_selected) igSetItemDefaultFocus();
                }

                igEndListBox();
            }
            igDragFloat(
                "Scale##board",
                &get_loaded_board()->board_scale,
                0.01,
                0.01,
                1.0,
                "%.3f",
                0
            );

            igSeparatorText("Item");
            igDragFloat(
                "Scale##item",
                &get_loaded_board()->item_scale,
                0.01,
                0.01,
                1.0,
                "%.3f",
                0
            );
            igDragFloat(
                "Elevation##item",
                &get_loaded_board()->item_elevation,
                0.01,
                0.01,
                1.0,
                "%.3f",
                0
            );
        }

        // Draw picked model transform inspector
        if (ig_collapsing_header("Transform", true)) {
            Transform* t = get_entity_transform(PICKED_ID);
            igDragFloat3(
                "Scale", (float*)&t->scale, 0.1, 0.1, 100.0, "%.1f", 0
            );
            igDragFloat3(
                "Translation",
                (float*)&t->translation,
                0.1,
                -100.0,
                100.0,
                "%.2f",
                0
            );

            Vector3 e = Vector3Scale(QuaternionToEuler(t->rotation), RAD2DEG);
            igDragFloat3("Rotation", (float*)&e, 5.0, -180.0, 180.0, "%.2f", 0);
            e = Vector3Scale(e, DEG2RAD);
            t->rotation = QuaternionFromEuler(e.x, e.y, e.z);
        }
    }
    igEnd();
}

static void blit_screen(RenderTexture screen, Vector2 position) {
    int w, h;
    Rectangle r;

    w = screen.texture.width;
    h = screen.texture.height;
    r = (Rectangle){0, 0, w, -h};
    DrawTextureRec(screen.texture, r, position, WHITE);
}

static int get_entity_id_under_cursor(void) {
    RenderTexture screen = *get_screen(PICKING_SCREEN);

    Vector2 mouse_position = GetMousePosition();
    unsigned char* pixels = (unsigned char*)rlReadTexturePixels(
        screen.texture.id,
        screen.texture.width,
        screen.texture.height,
        RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    );

    float x_fract = Clamp(mouse_position.x / SCREEN_WIDTH, 0.0, 1.0);
    float y_fract = Clamp(1.0 - (mouse_position.y / SCREEN_HEIGHT), 0.0, 1.0);
    int x = (int)(screen.texture.width * x_fract);
    int y = (int)(screen.texture.height * y_fract);
    int idx = 4 * (y * screen.texture.width + x);
    int id = pixels[idx];
    id = pixels[idx + 3] == 255 ? id : -1;

    free(pixels);
    return id;
}

void update_camera_shell(int shell, int camera_id) {
    Camera3D* camera = get_camera(camera_id);
    Transform* t = get_entity_transform(shell);
    camera->position = t->translation;
    Vector3 dir = Vector3RotateByQuaternion(
        (Vector3){0.0, 0.0, -1.0}, t->rotation
    );
    camera->target = Vector3Add(camera->position, dir);
}

void update_orbital_camera(int camera_id) {
    Camera3D* camera = get_camera(camera_id);
    static float rot_speed = 0.003f;
    static float move_speed = 0.01f;
    static float zoom_speed = 1.0f;

    bool is_mmb_down = IsMouseButtonDown(2);
    bool is_shift_down = IsKeyDown(KEY_LEFT_SHIFT);
    Vector2 mouse_delta = GetMouseDelta();

    // Shift + MMB + mouse move -> change the camera position in the
    // right-direction plane
    if (is_mmb_down && is_shift_down) {
        CameraMoveRight(camera, -move_speed * mouse_delta.x, true);

        Vector3 right = GetCameraRight(camera);
        Vector3 up = Vector3CrossProduct(
            Vector3Subtract(camera->position, camera->target), right
        );
        up = Vector3Scale(Vector3Normalize(up), move_speed * mouse_delta.y);
        camera->position = Vector3Add(camera->position, up);
        camera->target = Vector3Add(camera->target, up);
        // Rotate the camera around the look-at point
    } else if (is_mmb_down) {
        CameraYaw(camera, -rot_speed * mouse_delta.x, true);
        CameraPitch(camera, rot_speed * mouse_delta.y, true, true, false);
    }

    // Bring camera closer (or move away), to the look-at point
    CameraMoveToTarget(camera, -GetMouseWheelMove() * zoom_speed);
}

typedef struct SceneSaveData {
    struct {
        Transform transform[MAX_N_ENTITIES];
    } entity;

    Camera3D camera[MAX_N_CAMERAS];
} SceneSaveData;

void save_scene(const char* file_path) {
    SceneSaveData data;
    memcpy(
        data.entity.transform,
        SCENE.entity.transform,
        sizeof(SCENE.entity.transform)
    );
    memcpy(data.camera, SCENE.camera, sizeof(SCENE.camera));

    SaveFileData(file_path, &data, sizeof(data));
    TraceLog(LOG_INFO, "Scene data saved: %s", file_path);
}

void load_scene(const char* file_path) {
    int data_size;
    SceneSaveData data = *(SceneSaveData*)LoadFileData(file_path, &data_size);
    memcpy(
        SCENE.entity.transform,
        data.entity.transform,
        sizeof(SCENE.entity.transform)
    );
    memcpy(SCENE.camera, data.camera, sizeof(SCENE.camera));

    TraceLog(LOG_INFO, "Scene data loaded: %s", file_path);
}

int main(void) {
    // -------------------------------------------------------------------
    // Load window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Golova");
    SetTargetFPS(60);

    // -------------------------------------------------------------------
    // Load common resources
    create_scene();
    load_imgui();

    MODE = EDITOR_MODE;
    GIZMO = rgizmo_create();

    FULL_SCREEN = create_screen(LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT));
    PREVIEW_SCREEN = create_screen(
        LoadRenderTexture(SCREEN_WIDTH / 3, SCREEN_HEIGHT / 3)
    );
    PICKING_SCREEN = create_screen(
        LoadRenderTexture(SCREEN_WIDTH / 3, SCREEN_HEIGHT / 3)
    );

    PLANE_MESH = create_mesh(GenMeshPlane(1.0, 1.0, 2, 2));
    SPHERE_MESH = create_mesh(GenMeshSphere(1.0, 16, 16));
    PICKING_MATERIAL = create_material(LoadMaterialDefault());

    // Golova
    Material material = LoadMaterialDefault();
    material.shader = LoadShader(0, "resources/shaders/sprite.frag");
    material.maps[0].texture = LoadTexture("resources/textures/golova.png");
    SetTextureFilter(material.maps[0].texture, TEXTURE_FILTER_BILINEAR);

    float aspect = (float)material.maps[0].texture.width
                   / material.maps[0].texture.height;
    Mesh mesh = GenMeshPlane(aspect, 1.0, 2, 2);

    GOLOVA = create_entity();
    attach_entity_material(GOLOVA, create_material(material));
    attach_entity_mesh(GOLOVA, create_mesh(mesh));

    // Ground
    material = LoadMaterialDefault();
    material.shader = LoadShader(0, "resources/shaders/ground.frag");

    GROUND = create_entity();
    attach_entity_material(GROUND, create_material(material));
    attach_entity_mesh(GROUND, PLANE_MESH);

    // Item
    material = LoadMaterialDefault();
    material.shader = LoadShader(0, "resources/shaders/item.frag");
    SetTextureFilter(material.maps[0].texture, TEXTURE_FILTER_BILINEAR);

    ITEM = create_entity();
    set_entity_component(ITEM, NO_DRAW_COMPONENT);
    attach_entity_material(ITEM, create_material(material));
    attach_entity_mesh(ITEM, PLANE_MESH);

    // Cameras
    EDITOR_CAMERA = create_camera();
    get_camera(EDITOR_CAMERA)->position = (Vector3){5.0, 5.0, 5.0};
    GAME_CAMERA = create_camera();

    // Game camera shell
    material = LoadMaterialDefault();
    material.maps[0].color = PINK;

    GAME_CAMERA_SHELL = create_entity();
    attach_entity_mesh(GAME_CAMERA_SHELL, SPHERE_MESH);
    attach_entity_material(GAME_CAMERA_SHELL, create_material(material));
    set_entity_scalef(GAME_CAMERA_SHELL, 0.2);

    // Load scene data from file
    load_scene("./scene.gsc");

    // -------------------------------------------------------------------
    // Main loop
    RenderTexture full_screen = *get_screen(FULL_SCREEN);
    RenderTexture preview_screen = *get_screen(PREVIEW_SCREEN);
    RenderTexture picking_screen = *get_screen(PICKING_SCREEN);

    Camera3D* editor_camera = get_camera(EDITOR_CAMERA);
    Camera3D* game_camera = get_camera(GAME_CAMERA);

    while (!WindowShouldClose()) {
        bool is_enter_pressed = IsKeyPressed(KEY_ENTER) && !IS_IMGUI_INTERACTED;
        bool is_lmb_released = IsMouseButtonReleased(0) && !IS_IMGUI_INTERACTED;

        if (is_enter_pressed) {
            MODE = MODE == EDITOR_MODE ? GAME_MODE : EDITOR_MODE;
        }

        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) {
            save_scene("./scene.gsc");
        }

        if (MODE == EDITOR_MODE) {
            update_camera_shell(GAME_CAMERA_SHELL, GAME_CAMERA);
            update_orbital_camera(EDITOR_CAMERA);

            // Draw editor full screen
            rlDisableBackfaceCulling();
            BeginTextureMode(full_screen);
            ClearBackground(DARKGRAY);
            {

                BeginMode3D(*editor_camera);
                {
                    draw_scene(false);
                    draw_items(false);
                }
                EndMode3D();

                rlSetLineWidth(2.0);
                BeginMode3D(*editor_camera);
                {
                    draw_editor_grid();
                    //
                }
                EndMode3D();

                rlSetLineWidth(4.0);
                BeginMode3D(*editor_camera);
                {
                    draw_game_camera_ray();
                    //
                }
                EndMode3D();
            }
            EndTextureMode();

            // Draw game preview screen
            rlEnableBackfaceCulling();
            BeginTextureMode(preview_screen);
            ClearBackground(BLACK);
            BeginMode3D(*game_camera);
            {
                draw_scene(false);
                draw_items(false);
            }
            EndMode3D();
            EndTextureMode();

            // Update picked id
            int picked_id = PICKED_ID;
            if (is_lmb_released && GIZMO.state == RGIZMO_STATE_COLD) {
                rlDisableBackfaceCulling();
                BeginTextureMode(picking_screen);
                ClearBackground(BLANK);
                BeginMode3D(*editor_camera);
                {
                    draw_scene(true);
                    //
                }
                EndMode3D();
                EndTextureMode();
                picked_id = get_entity_id_under_cursor();
            }

            if (picked_id == -1 && GIZMO.state == RGIZMO_STATE_COLD) {
                PICKED_ID = -1;
            } else if (picked_id != -1) {
                PICKED_ID = picked_id;
            }

            // Update, apply and draw gizmo
            if (PICKED_ID != -1) {
                Transform* t = get_entity_transform(PICKED_ID);
                rgizmo_update(
                    &GIZMO, SCENE.camera[EDITOR_CAMERA], t->translation
                );

                t->translation = Vector3Add(
                    t->translation, GIZMO.update.translation
                );
                Quaternion q = QuaternionFromAxisAngle(
                    GIZMO.update.axis, GIZMO.update.angle
                );
                t->rotation = QuaternionMultiply(q, t->rotation);

                BeginTextureMode(full_screen);
                rgizmo_draw(GIZMO, *get_camera(EDITOR_CAMERA), t->translation);
                EndTextureMode();
            }

            BeginTextureMode(full_screen);
            begin_imgui();
            draw_imgui();
            end_imgui();
            EndTextureMode();

            // Blit screens
            BeginDrawing();
            {
                blit_screen(full_screen, Vector2Zero());

                Vector2 p = {
                    full_screen.texture.width - preview_screen.texture.width,
                    0.0};
                blit_screen(preview_screen, p);
            }
            EndDrawing();
        } else if (MODE == GAME_MODE) {
            // Draw item ids on the picking screen
            rlEnableBackfaceCulling();
            BeginTextureMode(picking_screen);
            ClearBackground(BLANK);
            BeginMode3D(*game_camera);
            {
                draw_items(true);
                //
            }
            EndMode3D();
            EndTextureMode();

            int picked_item_id = get_entity_id_under_cursor();
            printf("%d\n", picked_item_id);

            // Draw final game scene
            BeginTextureMode(full_screen);
            ClearBackground(BLACK);
            BeginMode3D(*game_camera);
            {
                draw_scene(false);
                draw_items(false);
            }
            EndMode3D();
            EndTextureMode();

            // Blit screens
            BeginDrawing();
            blit_screen(full_screen, Vector2Zero());
            EndDrawing();
        }
    }

    // -------------------------------------------------------------------
    // Unload the Scene
    rgizmo_unload();
    unload_scene();

    return 0;
}
