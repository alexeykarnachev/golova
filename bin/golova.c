#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_N_ENTITIES 256
#define MAX_N_MATERIALS 256
#define MAX_N_MESHES 256

int SCREEN_WIDTH = 1024;
int SCREEN_HEIGHT = 768;

RenderTexture FULL_SCREEN;
RenderTexture PREVIEW_SCREEN;
RenderTexture PICKING_SCREEN;

Material PICKING_MATERIAL;

Camera3D GAME_CAMERA;
Camera3D EDITOR_CAMERA;

int N_MATERIALS = 0;
Material MATERIALS[MAX_N_MATERIALS];

int N_MESHES = 0;
Mesh MESHES[MAX_N_MESHES];

int N_ENTITIES = 0;
Transform TRANSFORMS[MAX_N_ENTITIES];
int MATERIAL_IDS[MAX_N_ENTITIES];
int MESH_IDS[MAX_N_ENTITIES];

int GOLOVA;
int GROUND;

void draw_scene(
    RenderTexture screen, Color clear_color, Camera3D camera, bool is_picking
) {
    int mesh_ids[2] = {GOLOVA, GROUND};

    BeginTextureMode(screen);
    {
        ClearBackground(clear_color);
        BeginMode3D(camera);
        {
            for (size_t i = 0; i < 2; ++i) {
                int id = mesh_ids[i];
                Mesh mesh = MESHES[MESH_IDS[id]];
                Transform transform = TRANSFORMS[id];

                Material material;
                if (is_picking) {
                    rlDisableBackfaceCulling();
                    material = PICKING_MATERIAL;
                    material.maps[0].color = (Color){id, 0, 0, 255};
                } else {
                    rlEnableBackfaceCulling();
                    material = MATERIALS[MATERIAL_IDS[id]];
                }

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
                    rlRotatef(angle, axis.x, axis.y, axis.z);

                    DrawMesh(mesh, material, MatrixIdentity());
                }
                rlPopMatrix();
            }
        }
        EndMode3D();
    }
    EndTextureMode();
}

void blit_screen(RenderTexture screen) {
    int w, h;
    Rectangle r;

    w = screen.texture.width;
    h = screen.texture.height;
    r = (Rectangle){0, 0, w, -h};
    DrawTextureRec(FULL_SCREEN.texture, r, Vector2Zero(), WHITE);
}

int get_entity_id_under_cursor(void) {
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
    PICKING_MATERIAL = LoadMaterialDefault();
    FULL_SCREEN = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    PREVIEW_SCREEN = LoadRenderTexture(SCREEN_WIDTH / 3, SCREEN_HEIGHT / 3);
    PICKING_SCREEN = LoadRenderTexture(SCREEN_WIDTH / 3, SCREEN_HEIGHT / 3);

    // Entities
    GOLOVA = N_ENTITIES++;
    GROUND = N_ENTITIES++;

    // Transforms
    TRANSFORMS[GROUND].scale = Vector3One();
    TRANSFORMS[GOLOVA].scale = Vector3One();

    // Golova material and mesh
    Material material;
    Mesh mesh;
    int id;

    material = LoadMaterialDefault();
    material.shader = LoadShader(0, "resources/shaders/sprite.frag");
    material.maps[0].texture = LoadTexture("resources/textures/golova.png");
    float w = (float)material.maps[0].texture.width;
    float h = (float)material.maps[0].texture.height;
    mesh = GenMeshPlane(w / h, 1.0, 2, 2);

    id = N_MATERIALS++;
    MATERIALS[id] = material;
    MATERIAL_IDS[GOLOVA] = id;

    id = N_MESHES++;
    MESHES[id] = mesh;
    MESH_IDS[GOLOVA] = id;

    // Ground material and mesh
    material = LoadMaterialDefault();
    material.shader = LoadShader(0, "resources/shaders/ground.frag");
    material.maps[0].texture = LoadTexture("resources/textures/ground.png");
    mesh = GenMeshPlane(1.0, 1.0, 2, 2);

    id = N_MATERIALS++;
    MATERIALS[id] = material;
    MATERIAL_IDS[GROUND] = id;

    id = N_MESHES++;
    MESHES[id] = mesh;
    MESH_IDS[GROUND] = id;

    // Editor camera
    EDITOR_CAMERA.fovy = 45.0f;
    EDITOR_CAMERA.target = (Vector3){0.0f, 0.0f, 0.0f};
    EDITOR_CAMERA.position = (Vector3){5.0f, 5.0f, 5.0f};
    EDITOR_CAMERA.up = (Vector3){0.0f, 1.0f, 0.0f};
    EDITOR_CAMERA.projection = CAMERA_PERSPECTIVE;

    // Game camera
    GAME_CAMERA.fovy = 45.0f;
    GAME_CAMERA.target = (Vector3){0.0f, 0.0f, -1.0f};
    GAME_CAMERA.position = (Vector3){0.0f, 2.0f, 2.0f};
    GAME_CAMERA.up = (Vector3){0.0f, 1.0f, 0.0f};
    GAME_CAMERA.projection = CAMERA_PERSPECTIVE;

    // -------------------------------------------------------------------
    // Main loop
    while (!WindowShouldClose()) {
        // Draw scene
        draw_scene(PICKING_SCREEN, BLANK, EDITOR_CAMERA, true);
        draw_scene(FULL_SCREEN, DARKGRAY, EDITOR_CAMERA, false);
        draw_scene(PREVIEW_SCREEN, BLACK, GAME_CAMERA, false);

        int picked_entity_id = get_entity_id_under_cursor();
        printf("%d\n", picked_entity_id);

        // Blit Editor and Game screens
        BeginDrawing();
        {
            int w, h;
            Rectangle r;

            // Blit Editor screen
            w = FULL_SCREEN.texture.width;
            h = FULL_SCREEN.texture.height;
            r = (Rectangle){0, 0, w, -h};
            DrawTextureRec(FULL_SCREEN.texture, r, Vector2Zero(), WHITE);

            // Blit Game screen
            // w = PREVIEW_SCREEN.texture.width;
            // h = PREVIEW_SCREEN.texture.height;
            // r = (Rectangle){0, 0, w, h};
            // DrawTextureRec(PREVIEW_SCREEN.texture, r, Vector2Zero(), WHITE);
        }
        EndDrawing();
    }

    // -------------------------------------------------------------------
    // Unload the Scene
    UnloadMaterial(PICKING_MATERIAL);

    UnloadRenderTexture(PICKING_SCREEN);
    UnloadRenderTexture(PREVIEW_SCREEN);
    UnloadRenderTexture(FULL_SCREEN);

    for (int i = 0; i < N_MATERIALS; ++i)
        UnloadMaterial(MATERIALS[i]);
    for (int i = 0; i < N_MESHES; ++i)
        UnloadMesh(MESHES[i]);

    return 0;
}
