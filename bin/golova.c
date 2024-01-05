#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#define MAX_N_ENTITIES 256
#define MAX_N_MATERIALS 256
#define MAX_N_MESHES 256

int SCREEN_WIDTH = 1024;
int SCREEN_HEIGHT = 768;

RenderTexture EDITOR_SCREEN;
RenderTexture GAME_SCREEN;

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


void draw_entity(int id) {
    Vector3 axis;
    float angle;

    Mesh mesh = MESHES[MESH_IDS[id]];
    Material material = MATERIALS[MATERIAL_IDS[id]];

    rlPushMatrix();
    {
        Vector3 *t = &TRANSFORMS[id].translation;
        Vector3 *s = &TRANSFORMS[id].scale;
        Quaternion *q = &TRANSFORMS[id].rotation;
        QuaternionToAxisAngle(*q, &axis, &angle);

        rlScalef(s->x, s->y, s->z);
        rlTranslatef(t->x, t->y, t->z);
        rlRotatef(angle, axis.x, axis.y, axis.z);

        DrawMesh(mesh, material, MatrixIdentity());
    }
    rlPopMatrix();
}


int main(void) {
    // -------------------------------------------------------------------
    // Load window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Golova");
    SetTargetFPS(60);

    // -------------------------------------------------------------------
    // Load the Scene

    // Entities
    GOLOVA = N_ENTITIES++;
    GROUND = N_ENTITIES++;

    // Transforms
    TRANSFORMS[GROUND].scale = Vector3One();
    TRANSFORMS[GOLOVA].scale = Vector3One();

    // Meshes and materials
    Material material;
    Mesh mesh;
    int id;

    // Golova material and mesh
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

    // Screens
    EDITOR_SCREEN = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    GAME_SCREEN = LoadRenderTexture(SCREEN_WIDTH / 3, SCREEN_HEIGHT / 3);

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
        // Draw editor
        rlDisableBackfaceCulling();
        BeginTextureMode(EDITOR_SCREEN);
        {
            ClearBackground(DARKGRAY);
            BeginMode3D(EDITOR_CAMERA);
            {
                draw_entity(GOLOVA);
                draw_entity(GROUND);
            }
            EndMode3D();
        }
        EndTextureMode();

        // Blit Editor and Game screens
        BeginDrawing();
        {
            int w, h;
            Rectangle r;

            // Blit Editor screen
            w = EDITOR_SCREEN.texture.width;
            h = EDITOR_SCREEN.texture.height;
            r = (Rectangle){0, 0, w, -h};
            DrawTextureRec(EDITOR_SCREEN.texture, r, Vector2Zero(), WHITE);

            // Blit Game screen
            // w = GAME_SCREEN.texture.width;
            // h = GAME_SCREEN.texture.height;
            // r = (Rectangle){0, 0, w, h};
            // DrawTextureRec(GAME_SCREEN.texture, r, Vector2Zero(), WHITE);
        }
        EndDrawing();
    }

    // -------------------------------------------------------------------
    // Unload the Scene
    for (int i = 0; i < N_MATERIALS; ++i) UnloadMaterial(MATERIALS[i]);
    for (int i = 0; i < N_MESHES; ++i) UnloadMesh(MESHES[i]);

    return 0;
}
