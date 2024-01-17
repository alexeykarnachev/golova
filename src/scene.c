#include "scene.h"

#include "drawing.h"
#include "math.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <stdio.h>

Scene SCENE;

static void update_board_items(void);

void load_scene(const char* file_path) {
    // -------------------------------------------------------------------
    // Initialize entities by default
    // Golova
    SCENE.golova.transform = get_default_transform();

    // Board
    SCENE.board.transform = get_default_transform();
    SCENE.board.item_elevation = 0.5;
    SCENE.board.board_scale = 0.7;
    SCENE.board.item_scale = 0.2;
    SCENE.board.item_material = LoadMaterialDefault();
    SCENE.board.item_mesh = GenMeshPlane(1.0, 1.0, 2, 2);

    // Camera
    SCENE.camera.fovy = 60.0;
    SCENE.camera.projection = CAMERA_PERSPECTIVE;
    SCENE.camera.position = (Vector3){0.0, 2.0, 2.0};
    SCENE.camera.up = (Vector3){0.0, 1.0, 0.0};

    // -------------------------------------------------------------------
    // Update entities from the scene save file
    if (file_path) {
        FILE* f = fopen(file_path, "rb");

        // Camera
        fread(&SCENE.camera, sizeof(Camera3D), 1, f);

        // Golova
        fread(&SCENE.golova.transform, sizeof(Transform), 1, f);

        // Board
        fread(&SCENE.board.transform, sizeof(Transform), 1, f);
        fread(&SCENE.board.rule, sizeof(SCENE.board.rule), 1, f);
        fread(&SCENE.board.n_misses_allowed, sizeof(int), 1, f);
        fread(&SCENE.board.n_hits_required, sizeof(int), 1, f);
        fread(&SCENE.board.board_scale, sizeof(float), 1, f);
        fread(&SCENE.board.item_scale, sizeof(float), 1, f);
        fread(&SCENE.board.item_elevation, sizeof(float), 1, f);
        fread(&SCENE.board.n_items, sizeof(int), 1, f);

        for (size_t i = 0; i < SCENE.board.n_items; ++i) {
            Item* item = &SCENE.board.items[i];
            fread(&item->matrix, sizeof(Matrix), 1, f);
            fread(&item->is_correct, sizeof(bool), 1, f);
            fread(&item->name, sizeof(item->name), 1, f);

            if (item->name[0] != '\0') {
                static char fp[2048];
                sprintf(fp, "resources/items/sprites/%s.png", item->name);
                item->texture = LoadTexture(fp);
            }
        }

        fclose(f);
    }

    // -------------------------------------------------------------------
    // Load resources
    // Golova
    Texture2D texture = LoadTexture("resources/golova/sprites/golova.png");
    SCENE.golova.material = LoadMaterialDefault();
    SCENE.golova.material.shader = LoadShader(0, "resources/shaders/sprite.frag");
    SCENE.golova.material.maps[0].texture = texture;
    SCENE.golova.mesh = GenMeshPlane((float)texture.width / texture.height, 1.0, 2, 2);

    // Board
    SCENE.board.material = LoadMaterialDefault();
    SCENE.board.material.shader = LoadShader(0, "resources/shaders/board.frag");
    SCENE.board.mesh = GenMeshPlane(1.0, 1.0, 2, 2);
    SCENE.board.item_material = LoadMaterialDefault();
    SCENE.board.item_material.shader = LoadShader(0, "resources/shaders/item.frag");
}

void save_scene(const char* file_path) {
    FILE* f = fopen(file_path, "wb");

    // Camera
    fwrite(&SCENE.camera, sizeof(Camera3D), 1, f);

    // Golova
    fwrite(&SCENE.golova.transform, sizeof(Transform), 1, f);

    // Board
    fwrite(&SCENE.board.transform, sizeof(Transform), 1, f);
    fwrite(&SCENE.board.rule, sizeof(SCENE.board.rule), 1, f);
    fwrite(&SCENE.board.n_misses_allowed, sizeof(int), 1, f);
    fwrite(&SCENE.board.n_hits_required, sizeof(int), 1, f);
    fwrite(&SCENE.board.board_scale, sizeof(float), 1, f);
    fwrite(&SCENE.board.item_scale, sizeof(float), 1, f);
    fwrite(&SCENE.board.item_elevation, sizeof(float), 1, f);
    fwrite(&SCENE.board.n_items, sizeof(int), 1, f);

    // Items
    for (size_t i = 0; i < SCENE.board.n_items; ++i) {
        Item* item = &SCENE.board.items[i];
        fwrite(&item->matrix, sizeof(Matrix), 1, f);
        fwrite(&item->is_correct, sizeof(bool), 1, f);
        fwrite(&item->name, sizeof(item->name), 1, f);
    }

    TraceLog(LOG_INFO, "Scene saved: %s", file_path);

    fclose(f);
}

void unload_scene(void) {
    UnloadMesh(SCENE.golova.mesh);
    UnloadMesh(SCENE.board.mesh);
    UnloadMesh(SCENE.board.item_mesh);
    UnloadMaterial(SCENE.golova.material);
    UnloadMaterial(SCENE.board.material);
    UnloadMaterial(SCENE.board.item_material);
    for (size_t i = 0; i < SCENE.board.n_items; ++i) {
        Item* item = &SCENE.board.items[i];
        UnloadTexture(item->texture);
    }
    SCENE = (Scene){0};
}

void update_scene(void) {
    update_board_items();
}

void draw_scene(void) {
    draw_mesh_t(SCENE.golova.transform, SCENE.golova.material, SCENE.golova.mesh);
    draw_mesh_t(SCENE.board.transform, SCENE.board.material, SCENE.board.mesh);

    Shader item_shader = SCENE.board.item_material.shader;
    for (size_t i = 0; i < SCENE.board.n_items; ++i) {
        Item* item = &SCENE.board.items[i];
        if (item->is_dead) continue;

        SCENE.board.item_material.maps[0].texture = item->texture;
        int u_state = item->state;
        SetShaderValue(
            item_shader,
            GetShaderLocation(item_shader, "u_state"),
            &u_state,
            SHADER_UNIFORM_INT
        );
        draw_mesh_m(item->matrix, SCENE.board.item_material, SCENE.board.item_mesh);
    }
}

static void update_board_items(void) {
    Board* b = &SCENE.board;
    Transform t = b->transform;
    t.scale = Vector3Scale(Vector3One(), t.scale.x);

    size_t n_items = b->n_items;
    if (n_items == 0) return;

    int n_rows = sqrt(n_items);
    int n_cols = ceil((float)n_items / n_rows);
    for (size_t i = 0; i < n_items; ++i) {
        Item* item = &b->items[i];

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
}
