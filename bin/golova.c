#include "raylib.h"
#include "raymath.h"
#include "rcamera.h"
#include "rlgl.h"
#include <stdio.h>
#include <stdlib.h>

#define RAYGIZMO_IMPLEMENTATION
#include "raygizmo.h"

#define MAX_N_ENTITIES 256
#define MAX_N_MATERIALS 256
#define MAX_N_MESHES 256

typedef struct Golova {
    int id;
} Golova;

typedef struct Ground {
    int id;
} Ground;

typedef struct GameCamera {
    int id;
    Camera3D c;
} GameCamera;

typedef enum Mode { EDITOR_MODE, GAME_MODE } Mode;

Mode MODE;

int SCREEN_WIDTH = 1024;
int SCREEN_HEIGHT = 768;

RenderTexture FULL_SCREEN;
RenderTexture PREVIEW_SCREEN;
RenderTexture PICKING_SCREEN;

Material PICKING_MATERIAL;

Camera3D EDITOR_CAMERA;

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
GameCamera GAME_CAMERA;

RGizmo GIZMO;

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
    MATERIALS[material_id].shader = LoadShader(0, "resources/shaders/sprite.frag");
    MATERIALS[material_id].maps[0].texture = LoadTexture("resources/textures/golova.png");

    float w = (float)MATERIALS[material_id].maps[0].texture.width;
    float h = (float)MATERIALS[material_id].maps[0].texture.height;
    MESHES[mesh_id] = GenMeshPlane(w / h, 1.0, 2, 2);
}

static void load_ground(void) {
    int entity_id = N_ENTITIES++;
    int material_id = N_MATERIALS++;
    int mesh_id = N_MESHES++;

    GROUND.id = entity_id;

    TRANSFORMS[entity_id].scale = Vector3One();
    TRANSFORMS[entity_id].rotation.w = 1.0;
    MESH_IDS[entity_id] = mesh_id;
    MATERIAL_IDS[entity_id] = material_id;

    MATERIALS[material_id] = LoadMaterialDefault();
    MATERIALS[material_id].shader = LoadShader(0, "resources/shaders/ground.frag");
    MESHES[mesh_id] = GenMeshPlane(1.0, 1.0, 2, 2);
}

static void load_game_camera(void) {
    int entity_id = N_ENTITIES++;
    int material_id = N_MATERIALS++;
    int mesh_id = N_MESHES++;

    GAME_CAMERA.id = entity_id;
    GAME_CAMERA.c.fovy = 45.0f;
    GAME_CAMERA.c.target = (Vector3){0.0f, 0.0f, -1.0f};
    GAME_CAMERA.c.up = (Vector3){0.0f, 1.0f, 0.0f};
    GAME_CAMERA.c.projection = CAMERA_PERSPECTIVE;

    TRANSFORMS[GAME_CAMERA.id].translation = (Vector3){0.0, 2.0, 2.0};
    TRANSFORMS[GAME_CAMERA.id].scale = Vector3One();
    TRANSFORMS[GAME_CAMERA.id].rotation.w = 1.0;
    MESH_IDS[entity_id] = mesh_id;
    MATERIAL_IDS[entity_id] = material_id;

    MATERIALS[material_id] = LoadMaterialDefault();
    MATERIALS[material_id].maps[0].color = PINK;

    MESHES[mesh_id] = GenMeshSphere(0.1, 16, 16);
}

static void draw_scene(
    RenderTexture screen, Color clear_color, Camera3D camera, bool is_picking
) {
    rlDisableBackfaceCulling();

    int mesh_ids[3] = {GOLOVA.id, GROUND.id, GAME_CAMERA.id};
    size_t n_meshes = sizeof(mesh_ids) / sizeof(int);

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

                if (id == GAME_CAMERA.id) rlEnableBackfaceCulling();

                Vector3 axis;
                float angle;
                rlPushMatrix();
                {
                    Vector3* t = &transform.translation;
                    Vector3* s = &transform.scale;
                    Quaternion* q = &transform.rotation;
                    QuaternionToAxisAngle(*q, &axis, &angle);

                    rlScalef(s->x, s->y, s->z);
                    rlTranslatef(t->x, t->y, t->z);
                    rlRotatef(angle * RAD2DEG, axis.x, axis.y, axis.z);

                    DrawMesh(mesh, material, MatrixIdentity());
                }
                rlPopMatrix();
            }

            // Draw game camera sphere
            // rlEnableBackfaceCulling();
        }
        EndMode3D();
    }
    EndTextureMode();
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

int main(void) {
    // -------------------------------------------------------------------
    // Load window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Golova");
    SetTargetFPS(60);

    // -------------------------------------------------------------------
    // Load the Scene

    // Common
    MODE = EDITOR_MODE;
    GIZMO = rgizmo_create();
    PICKING_MATERIAL = LoadMaterialDefault();
    FULL_SCREEN = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    PREVIEW_SCREEN = LoadRenderTexture(SCREEN_WIDTH / 3, SCREEN_HEIGHT / 3);
    PICKING_SCREEN = LoadRenderTexture(SCREEN_WIDTH / 3, SCREEN_HEIGHT / 3);

    // Scene entities
    load_golova();
    load_ground();
    load_game_camera();

    // Editor camera
    EDITOR_CAMERA.fovy = 45.0f;
    EDITOR_CAMERA.target = (Vector3){0.0f, 0.0f, 0.0f};
    EDITOR_CAMERA.position = (Vector3){5.0f, 5.0f, 5.0f};
    EDITOR_CAMERA.up = (Vector3){0.0f, 1.0f, 0.0f};
    EDITOR_CAMERA.projection = CAMERA_PERSPECTIVE;

    // -------------------------------------------------------------------
    // Main loop
    while (!WindowShouldClose()) {
        // Update game camera
        Transform t = TRANSFORMS[GAME_CAMERA.id];
        Vector3 offset = Vector3Subtract(t.translation, GAME_CAMERA.c.position);
        Vector3 target = (Vector3){0.0, 0.0, -1.0};
        target = Vector3Add(target, offset);
        target = Vector3RotateByQuaternion(target, t.rotation);
        GAME_CAMERA.c.position = t.translation;
        GAME_CAMERA.c.target = Vector3Add(GAME_CAMERA.c.position, target);

        if (MODE == EDITOR_MODE) {
            // Draw scene
            draw_scene(FULL_SCREEN, DARKGRAY, EDITOR_CAMERA, false);
            draw_scene(PREVIEW_SCREEN, BLACK, GAME_CAMERA.c, false);

            // Update picked id
            static int PICKED_ID = -1;
            int picked_id = PICKED_ID;
            if (IsMouseButtonReleased(0) && GIZMO.state == RGIZMO_STATE_COLD) {
                draw_scene(PICKING_SCREEN, BLANK, EDITOR_CAMERA, true);
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

            // Blit screens
            BeginDrawing();
            {
                blit_screen(FULL_SCREEN, Vector2Zero());

                Vector2 p = {FULL_SCREEN.texture.width - PREVIEW_SCREEN.texture.width, 0.0};
                blit_screen(PREVIEW_SCREEN, p);
            }
            EndDrawing();
        } else if (MODE == GAME_MODE) {
            // Draw scene
            draw_scene(FULL_SCREEN, BLACK, GAME_CAMERA.c, false);
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
