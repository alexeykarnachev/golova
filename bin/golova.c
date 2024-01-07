#include "raylib.h"
#include "raymath.h"
#include "rcamera.h"
#include "rlgl.h"
#include <stdio.h>
#include <stdlib.h>

#define RAYGIZMO_IMPLEMENTATION
#include "raygizmo.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_USE_GLFW
#define CIMGUI_USE_OPENGL3
#include "cimgui.h"
#include "cimgui_impl.h"
#include <GLFW/glfw3.h>

#define MAX_N_ENTITIES 256
#define MAX_N_MATERIALS 256
#define MAX_N_MESHES 256

#define print_vector3(v) printf("%f, %f, %f\n", v.x, v.y, v.z)

typedef struct Golova {
    int id;
} Golova;

typedef struct Ground {
    int id;

    struct {
        int dimension[2];
        float scale;
    } grid;

    struct {
        float scale;
        float elevation;
    } item;
} Ground;

typedef struct CameraShell {
    int id;
    Camera3D* camera;
} CameraShell;

typedef enum Mode {
    EDITOR_MODE,
    GAME_MODE,
} Mode;

Mode MODE;

// int SCREEN_WIDTH = 1024;
// int SCREEN_HEIGHT = 768;
int SCREEN_WIDTH = 2560;
int SCREEN_HEIGHT = 1440;

RenderTexture FULL_SCREEN;
RenderTexture PREVIEW_SCREEN;
RenderTexture PICKING_SCREEN;

Material PICKING_MATERIAL;
Material ITEM_MATERIAL;

Mesh PLANE_MESH;

Camera3D EDITOR_CAMERA;
Camera3D GAME_CAMERA;

int N_MATERIALS = 0;
Material MATERIALS[MAX_N_MATERIALS];

int N_MESHES = 0;
Mesh MESHES[MAX_N_MESHES];

int N_ENTITIES = 0;
Transform TRANSFORMS[MAX_N_ENTITIES];
int MATERIAL_IDS[MAX_N_ENTITIES];
int MESH_IDS[MAX_N_ENTITIES];

Golova GOLOVA;
Ground GROUND;
CameraShell GAME_CAMERA_SHELL;

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

static void load_imgui(void) {
    igCreateContext(NULL);
    GLFWwindow* window = (GLFWwindow*)GetWindowHandle();
    glfwGetWindowUserPointer(window);
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    igStyleColorsDark(NULL);
}

static void load_golova(void) {
    int entity_id = N_ENTITIES++;
    int material_id = N_MATERIALS++;
    int mesh_id = N_MESHES++;

    GOLOVA.id = entity_id;

    TRANSFORMS[entity_id].scale = Vector3One();
    TRANSFORMS[entity_id].rotation.w = 1.0;
    MESH_IDS[entity_id] = mesh_id;
    MATERIAL_IDS[entity_id] = material_id;

    MATERIALS[material_id] = LoadMaterialDefault();
    MATERIALS[material_id].shader = LoadShader(
        0, "resources/shaders/sprite.frag"
    );
    MATERIALS[material_id].maps[0].texture = LoadTexture(
        "resources/textures/golova.png"
    );

    float w = (float)MATERIALS[material_id].maps[0].texture.width;
    float h = (float)MATERIALS[material_id].maps[0].texture.height;
    MESHES[mesh_id] = GenMeshPlane(w / h, 1.0, 2, 2);
}

static void load_ground(void) {
    // Ground
    int entity_id = N_ENTITIES++;
    int material_id = N_MATERIALS++;
    int mesh_id = N_MESHES++;

    GROUND.id = entity_id;

    TRANSFORMS[entity_id].scale = Vector3One();
    TRANSFORMS[entity_id].rotation.w = 1.0;
    MESH_IDS[entity_id] = mesh_id;
    MATERIAL_IDS[entity_id] = material_id;

    MATERIALS[material_id] = LoadMaterialDefault();
    MATERIALS[material_id].shader = LoadShader(
        0, "resources/shaders/ground.frag"
    );
    MESHES[mesh_id] = GenMeshPlane(1.0, 1.0, 2, 2);

    // Grid
    GROUND.grid.dimension[0] = 4;
    GROUND.grid.dimension[1] = 3;
    GROUND.grid.scale = 0.7;

    // Item
    GROUND.item.scale = 0.1;
    GROUND.item.elevation = 0.5;
}

static void load_game_camera(void) {
    // Camera object
    GAME_CAMERA.fovy = 60.0f;
    GAME_CAMERA.up = (Vector3){0.0f, 1.0f, 0.0f};
    GAME_CAMERA.projection = CAMERA_PERSPECTIVE;

    // Camera shell object (mesh for controlling in editor mode)
    int entity_id = N_ENTITIES++;
    int material_id = N_MATERIALS++;
    int mesh_id = N_MESHES++;

    GAME_CAMERA_SHELL.id = entity_id;
    GAME_CAMERA_SHELL.camera = &GAME_CAMERA;

    TRANSFORMS[entity_id].translation = (Vector3){0.0, 2.0, 2.0};
    TRANSFORMS[entity_id].scale = Vector3One();
    TRANSFORMS[entity_id].rotation.w = 1.0;
    MESH_IDS[entity_id] = mesh_id;
    MATERIAL_IDS[entity_id] = material_id;

    MATERIALS[material_id] = LoadMaterialDefault();
    MATERIALS[material_id].maps[0].color = PINK;

    MESHES[mesh_id] = GenMeshSphere(0.2, 16, 16);
}

static void load_editor_camera(void) {
    EDITOR_CAMERA.fovy = 45.0f;
    EDITOR_CAMERA.target = (Vector3){0.0f, 0.0f, 0.0f};
    EDITOR_CAMERA.position = (Vector3){5.0f, 5.0f, 5.0f};
    EDITOR_CAMERA.up = (Vector3){0.0f, 1.0f, 0.0f};
    EDITOR_CAMERA.projection = CAMERA_PERSPECTIVE;
}

static void draw_scene(
    RenderTexture screen,
    Color clear_color,
    Camera3D camera,
    bool is_picking,
    bool is_editor
) {
    static int mesh_ids[MAX_N_MESHES] = {-1};
    size_t n_meshes = 0;
    mesh_ids[n_meshes++] = GOLOVA.id;
    mesh_ids[n_meshes++] = GROUND.id;
    if (is_editor) mesh_ids[n_meshes++] = GAME_CAMERA_SHELL.id;

    BeginTextureMode(screen);
    {
        ClearBackground(clear_color);
        BeginMode3D(camera);
        {
            // Draw meshes
            for (size_t i = 0; i < n_meshes; ++i) {
                int id = mesh_ids[i];

                Mesh mesh = MESHES[MESH_IDS[id]];
                Transform transform = TRANSFORMS[id];

                Material material;
                if (is_picking) {
                    material = PICKING_MATERIAL;
                    material.maps[0].color = (Color){id, 0, 0, 255};
                } else {
                    material = MATERIALS[MATERIAL_IDS[id]];
                }

                rlPushMatrix();
                {
                    rl_transform(transform);
                    DrawMesh(mesh, material, MatrixIdentity());
                }
                rlPopMatrix();

                // Draw game camera shell view ray
                if (id == GAME_CAMERA_SHELL.id) {
                    rlSetLineWidth(4.0);
                    Vector3 start = GAME_CAMERA_SHELL.camera->position;
                    Vector3 target = GAME_CAMERA_SHELL.camera->target;
                    Vector3 dir = Vector3Normalize(
                        Vector3Subtract(target, start)
                    );
                    Vector3 end = Vector3Add(start, Vector3Scale(dir, 1.0));
                    DrawLine3D(start, end, PINK);
                }
            }
        }
        EndMode3D();

        // Draw editor grid
        if (is_editor) {
            BeginMode3D(camera);
            {
                rlSetLineWidth(1.0);
                DrawGrid(10.0, 5.0);
                float d = 25.0f;

                rlSetLineWidth(2.0);
                DrawLine3D(
                    (Vector3){-d, 0.0f, 0.0f}, (Vector3){d, 0.0f, 0.0f}, RED
                );
                DrawLine3D(
                    (Vector3){0.0f, -d, 0.0f}, (Vector3){0.0f, d, 0.0f}, GREEN
                );
                DrawLine3D(
                    (Vector3){0.0f, 0.0f, -d},
                    (Vector3){0.0f, 0.0f, d},
                    DARKBLUE
                );
            }
            EndMode3D();
        }
    }
    EndTextureMode();
}

static void draw_items(RenderTexture screen, Camera3D camera, bool is_picking) {
    Transform transform = TRANSFORMS[GROUND.id];
    transform.scale = Vector3Scale(Vector3One(), transform.scale.x);

    BeginTextureMode(screen);
    BeginMode3D(camera);

    int item_idx = 0;
    for (size_t i = 0; i < GROUND.grid.dimension[0]; ++i) {
        float z = (float)i / (GROUND.grid.dimension[0] - 1) - 0.5;

        for (size_t j = 0; j < GROUND.grid.dimension[1]; ++j, ++item_idx) {
            float x = (float)j / (GROUND.grid.dimension[1] - 1) - 0.5;

            rlPushMatrix();

            rl_transform(transform);

            rlScalef(GROUND.grid.scale, GROUND.grid.scale, GROUND.grid.scale);
            rlTranslatef(x, 0.0, z);
            rlScalef(GROUND.item.scale, GROUND.item.scale, GROUND.item.scale);
            rlTranslatef(0.0, GROUND.item.elevation, 0.0);

            rlRotatef(90.0, 1.0, 0.0, 0.0);

            int atlas_grid_size[2] = {8, 8};
            SetShaderValue(
                ITEM_MATERIAL.shader,
                GetShaderLocation(ITEM_MATERIAL.shader, "atlas_grid_size"),
                atlas_grid_size,
                SHADER_UNIFORM_IVEC2
            );
            SetShaderValue(
                ITEM_MATERIAL.shader,
                GetShaderLocation(ITEM_MATERIAL.shader, "item_idx"),
                &item_idx,
                SHADER_UNIFORM_INT
            );

            DrawMesh(PLANE_MESH, ITEM_MATERIAL, MatrixIdentity());

            rlPopMatrix();
        }
    }

    EndMode3D();
    EndTextureMode();
}

static void draw_imgui(void) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    igNewFrame();

    ImGuiIO* io = igGetIO();
    IS_IMGUI_INTERACTED = io->WantCaptureMouse || io->WantCaptureKeyboard;

    // Place next window on top left
    ImVec2 position = {0.0, 0.0};
    igSetNextWindowPos(position, ImGuiCond_Always, (ImVec2){0.0, 0.0});
    igSetNextWindowSize((ImVec2){0.0, 0.0}, ImGuiCond_Always);

    // Draw inspector
    if (igBegin("Inspector", NULL, 0)) {
        int tree_node_open = ImGuiTreeNodeFlags_DefaultOpen;
        // Draw camera inspector
        if (igCollapsingHeader_TreeNodeFlags("Camera", tree_node_open)) {
            igPushItemWidth(150.0);
            igDragFloat("FOV", &GAME_CAMERA.fovy, 1.0, 10.0, 170.0, "%.1f", 0);
            igPopItemWidth();
        }

        if (igCollapsingHeader_TreeNodeFlags("Ground", tree_node_open)) {
            igPushItemWidth(150.0);

            igSeparatorText("Grid");
            igDragInt2(
                "Dimension##grid", GROUND.grid.dimension, 1, 1, 5, "%d", 0
            );
            igDragFloat(
                "Scale##grid", &GROUND.grid.scale, 0.01, 0.01, 1.0, "%.3f", 0
            );

            igSeparatorText("Item");
            igDragFloat(
                "Elevation##item",
                &GROUND.item.elevation,
                0.01,
                0.01,
                2.0,
                "%.3f",
                0
            );
            igDragFloat(
                "Scale##item", &GROUND.item.scale, 0.01, 0.01, 1.0, "%.3f", 0
            );
            igPopItemWidth();
        }

        // Draw picked model transform inspector
        if (igCollapsingHeader_TreeNodeFlags("Transform", tree_node_open)
            && PICKED_ID != -1) {
            Transform* t = &TRANSFORMS[PICKED_ID];
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

    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
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
    Vector2 mouse_position = GetMousePosition();
    unsigned char* pixels = (unsigned char*)rlReadTexturePixels(
        PICKING_SCREEN.texture.id,
        PICKING_SCREEN.texture.width,
        PICKING_SCREEN.texture.height,
        RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    );

    float x_fract = Clamp(mouse_position.x / SCREEN_WIDTH, 0.0, 1.0);
    float y_fract = Clamp(1.0 - (mouse_position.y / SCREEN_HEIGHT), 0.0, 1.0);
    int x = (int)(PICKING_SCREEN.texture.width * x_fract);
    int y = (int)(PICKING_SCREEN.texture.height * y_fract);
    int idx = 4 * (y * PICKING_SCREEN.texture.width + x);
    int id = pixels[idx];
    id = pixels[idx + 3] == 255 ? id : -1;

    free(pixels);
    return id;
}

void update_camera_shell(CameraShell* camera_shell) {
    Transform t = TRANSFORMS[camera_shell->id];
    camera_shell->camera->position = t.translation;
    Vector3 dir = Vector3RotateByQuaternion(
        (Vector3){0.0, 0.0, -1.0}, t.rotation
    );
    camera_shell->camera->target = Vector3Add(
        camera_shell->camera->position, dir
    );
}

void update_orbital_camera(Camera3D* camera) {
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

int main(void) {
    // -------------------------------------------------------------------
    // Load window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Golova");
    SetTargetFPS(60);

    // -------------------------------------------------------------------
    // Load the Scene
    load_imgui();

    // Common
    MODE = EDITOR_MODE;
    GIZMO = rgizmo_create();
    PICKING_MATERIAL = LoadMaterialDefault();
    ITEM_MATERIAL = LoadMaterialDefault();
    ITEM_MATERIAL.shader = LoadShader(0, "resources/shaders/item.frag");
    ITEM_MATERIAL.maps[0].texture = LoadTexture("resources/textures/items_0.png"
    );
    PLANE_MESH = GenMeshPlane(1.0, 1.0, 2, 2);
    FULL_SCREEN = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    PREVIEW_SCREEN = LoadRenderTexture(SCREEN_WIDTH / 3, SCREEN_HEIGHT / 3);
    PICKING_SCREEN = LoadRenderTexture(SCREEN_WIDTH / 3, SCREEN_HEIGHT / 3);

    // Scene entities
    load_golova();
    load_ground();
    load_game_camera();
    load_editor_camera();

    // -------------------------------------------------------------------
    // Main loop
    while (!WindowShouldClose()) {
        bool is_enter_pressed = IsKeyPressed(KEY_ENTER) && !IS_IMGUI_INTERACTED;
        bool is_lmb_released = IsMouseButtonReleased(0) && !IS_IMGUI_INTERACTED;

        if (is_enter_pressed) {
            MODE = MODE == EDITOR_MODE ? GAME_MODE : EDITOR_MODE;
        }

        if (MODE == EDITOR_MODE) {
            rlDisableBackfaceCulling();

            update_camera_shell(&GAME_CAMERA_SHELL);
            update_orbital_camera(&EDITOR_CAMERA);

            // Draw scene
            draw_scene(FULL_SCREEN, DARKGRAY, EDITOR_CAMERA, false, true);
            draw_scene(PREVIEW_SCREEN, BLACK, GAME_CAMERA, false, false);
            draw_items(FULL_SCREEN, EDITOR_CAMERA, false);
            draw_items(PREVIEW_SCREEN, GAME_CAMERA, false);

            // Update picked id
            int picked_id = PICKED_ID;
            if (is_lmb_released && GIZMO.state == RGIZMO_STATE_COLD) {
                draw_scene(PICKING_SCREEN, BLANK, EDITOR_CAMERA, true, true);
                picked_id = get_entity_id_under_cursor();
            }

            if (picked_id == -1 && GIZMO.state == RGIZMO_STATE_COLD) {
                PICKED_ID = -1;
            } else if (picked_id != -1) {
                PICKED_ID = picked_id;
            }

            // Update, apply and draw gizmo
            if (PICKED_ID != -1) {
                Transform* t = &TRANSFORMS[PICKED_ID];
                rgizmo_update(&GIZMO, EDITOR_CAMERA, t->translation);

                t->translation = Vector3Add(
                    t->translation, GIZMO.update.translation
                );
                Quaternion q = QuaternionFromAxisAngle(
                    GIZMO.update.axis, GIZMO.update.angle
                );
                t->rotation = QuaternionMultiply(q, t->rotation);

                BeginTextureMode(FULL_SCREEN);
                rgizmo_draw(GIZMO, EDITOR_CAMERA, t->translation);
                EndTextureMode();
            }

            BeginTextureMode(FULL_SCREEN);
            draw_imgui();
            EndTextureMode();

            // Blit screens
            BeginDrawing();
            {
                blit_screen(FULL_SCREEN, Vector2Zero());

                Vector2 p = {
                    FULL_SCREEN.texture.width - PREVIEW_SCREEN.texture.width,
                    0.0};
                blit_screen(PREVIEW_SCREEN, p);
            }
            EndDrawing();
        } else if (MODE == GAME_MODE) {
            rlEnableBackfaceCulling();

            // Draw scene
            draw_scene(FULL_SCREEN, BLACK, GAME_CAMERA, false, false);
            draw_items(FULL_SCREEN, GAME_CAMERA, false);
            // draw_scene(PICKING_SCREEN, BLANK, GAME_CAMERA, true);

            // Blit screens
            BeginDrawing();
            blit_screen(FULL_SCREEN, Vector2Zero());
            EndDrawing();
        }
    }

    // -------------------------------------------------------------------
    // Unload the Scene
    UnloadMaterial(PICKING_MATERIAL);
    UnloadMaterial(ITEM_MATERIAL);
    UnloadMesh(PLANE_MESH);
    rgizmo_unload();

    UnloadRenderTexture(PICKING_SCREEN);
    UnloadRenderTexture(PREVIEW_SCREEN);
    UnloadRenderTexture(FULL_SCREEN);

    for (int i = 0; i < N_MATERIALS; ++i)
        UnloadMaterial(MATERIALS[i]);
    for (int i = 0; i < N_MESHES; ++i)
        UnloadMesh(MESHES[i]);

    return 0;
}
