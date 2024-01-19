#include "scene.h"

#include "drawing.h"
#include "math.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "utils.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SHADOWMAP_WIDTH 1024
#define SHADOWMAP_HEIGHT 768

Scene SCENE;

RenderTexture2D SHADOWMAP;
RenderTexture2D SCREEN;
Material MATERIAL_DEFAULT;
Shader POSTFX_SHADER;

static char* load_shader_src(const char* file_path);
static Shader load_shader(const char* vs_file_path, const char* fs_file_path);

void init_core(int screen_width, int screen_height) {
    InitWindow(screen_width, screen_height, "Golova");
    SetTargetFPS(60);

    MATERIAL_DEFAULT = LoadMaterialDefault();
    SHADOWMAP = LoadRenderTexture(SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT);
    SetTextureWrap(SHADOWMAP.texture, TEXTURE_WRAP_CLAMP);
    SCREEN = LoadRenderTexture(screen_width, screen_height);
    POSTFX_SHADER = load_shader(0, "resources/shaders/postfx.frag");

    // -------------------------------------------------------------------
    // Load resources
    // Golova idle
    Texture2D texture = LoadTexture("resources/golova/sprites/golova_idle.png");
    SCENE.golova.idle.material = LoadMaterialDefault();
    SCENE.golova.idle.material.shader = load_shader(0, "resources/shaders/sprite.frag");
    SCENE.golova.idle.material.maps[0].texture = texture;
    SCENE.golova.idle.mesh = GenMeshPlane(
        (float)texture.width / texture.height, 1.0, 2, 2
    );

    // Golova eat
    texture = LoadTexture("resources/golova/sprites/golova_eat.png");
    SCENE.golova.eat.material = LoadMaterialDefault();
    SCENE.golova.eat.material.shader = load_shader(0, "resources/shaders/sprite.frag");
    SCENE.golova.eat.material.maps[0].texture = texture;
    SCENE.golova.eat.mesh = GenMeshPlane(
        (float)texture.width / texture.height, 1.0, 2, 2
    );

    // Golova eyes
    SCENE.golova.eyes_material = LoadMaterialDefault();
    SCENE.golova.eyes_material.shader = load_shader(0, "resources/shaders/sprite.frag");

    texture = LoadTexture("resources/golova/sprites/eye_left.png");
    SCENE.golova.eye_left.texture = texture;
    SCENE.golova.eye_left.mesh = GenMeshPlane(
        (float)texture.width / texture.height, 1.0, 2, 2
    );

    texture = LoadTexture("resources/golova/sprites/eye_right.png");
    SCENE.golova.eye_right.texture = texture;
    SCENE.golova.eye_right.mesh = GenMeshPlane(
        (float)texture.width / texture.height, 1.0, 2, 2
    );

    // Board
    SCENE.board.material = LoadMaterialDefault();
    SCENE.board.material.shader = load_shader(
        "resources/shaders/board.vert", "resources/shaders/board.frag"
    );
    SCENE.board.mesh = GenMeshPlane(1.0, 1.0, 2, 2);
    SCENE.board.item_material = LoadMaterialDefault();
    SCENE.board.item_material.shader = load_shader(0, "resources/shaders/item.frag");
}

void load_scene(const char* file_path) {
    // Golova
    SCENE.golova.transform = get_default_transform();
    SCENE.golova.eyes_idle_scale = 0.056;
    SCENE.golova.eyes_idle_uplift = 0.027;
    SCENE.golova.eyes_idle_shift = 0.014;
    SCENE.golova.eyes_idle_spread = 0.252;

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

    // Light camera
    SCENE.light_camera.fovy = 1.0;
    SCENE.light_camera.projection = CAMERA_ORTHOGRAPHIC;
    SCENE.light_camera.up = (Vector3){0.0, 1.0, 0.0};
    SCENE.light_camera.position = (Vector3){0.0, 1.0, -1.0};
    SCENE.light_camera.target = Vector3Zero();

    // -------------------------------------------------------------------
    // Update entities from the scene save file
    if (file_path) {
        FILE* f = fopen(file_path, "rb");

        // Camera
        fread(&SCENE.camera, sizeof(Camera3D), 1, f);

        // Golova
        fread(&SCENE.golova.transform, sizeof(Transform), 1, f);
        fread(&SCENE.golova.eyes_idle_scale, sizeof(float), 1, f);
        fread(&SCENE.golova.eyes_idle_uplift, sizeof(float), 1, f);
        fread(&SCENE.golova.eyes_idle_shift, sizeof(float), 1, f);
        fread(&SCENE.golova.eyes_idle_spread, sizeof(float), 1, f);

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
                if (IsTextureReady(item->texture)) UnloadTexture(item->texture);
                item->texture = LoadTexture(fp);
            }
        }

        fclose(f);
    }

    SCENE.golova.eyes_curr_shift = SCENE.golova.eyes_idle_shift;
    SCENE.golova.eyes_curr_uplift = SCENE.golova.eyes_idle_uplift;
}

void save_scene(const char* file_path) {
    FILE* f = fopen(file_path, "wb");

    // Camera
    fwrite(&SCENE.camera, sizeof(Camera3D), 1, f);

    // Golova
    fwrite(&SCENE.golova.transform, sizeof(Transform), 1, f);
    fwrite(&SCENE.golova.eyes_idle_scale, sizeof(float), 1, f);
    fwrite(&SCENE.golova.eyes_idle_uplift, sizeof(float), 1, f);
    fwrite(&SCENE.golova.eyes_idle_shift, sizeof(float), 1, f);
    fwrite(&SCENE.golova.eyes_idle_spread, sizeof(float), 1, f);

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

static void draw_items(void) {
    Shader item_shader = SCENE.board.item_material.shader;
    for (size_t i = 0; i < SCENE.board.n_items; ++i) {
        Item* item = &SCENE.board.items[i];
        if (item->state == ITEM_DEAD) continue;

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

void draw_scene(bool with_shadows) {
    draw_scene_ex(SCREEN, BLACK, SCENE.camera, with_shadows);
}

void draw_scene_ex(
    RenderTexture2D screen, Color clear_color, Camera3D camera, bool with_shadows
) {
    Matrix light_vp;
    if (with_shadows) {
        BeginTextureMode(SHADOWMAP);
        rlDisableBackfaceCulling();

        ClearBackground(BLANK);
        BeginMode3D(SCENE.light_camera);
        Matrix light_view = rlGetMatrixModelview();
        Matrix light_proj = rlGetMatrixProjection();
        light_vp = MatrixMultiply(light_view, light_proj);
        draw_items();
        EndMode3D();

        EndTextureMode();
    }

    // -------------------------------------------------------------------
    // Draw scene
    BeginTextureMode(screen);
    ClearBackground(clear_color);
    BeginMode3D(camera);

    // Board
    Shader shader = SCENE.board.material.shader;
    SCENE.board.material.maps[0].texture = SHADOWMAP.texture;
    SetShaderValueMatrix(shader, GetShaderLocation(shader, "u_light_vp"), light_vp);
    int u_with_shadows = (int)with_shadows;
    SetShaderValue(
        shader,
        GetShaderLocation(shader, "u_with_shadows"),
        &u_with_shadows,
        SHADER_UNIFORM_INT
    );
    draw_mesh_t(SCENE.board.transform, SCENE.board.material, SCENE.board.mesh);

    // Golova
    Transform golova_transform = SCENE.golova.transform;
    Matrix golova_mat = get_transform_matrix(golova_transform);
    Mesh golova_mesh;
    Material golova_material;
    if (SCENE.golova.state == GOLOVA_IDLE) {
        golova_mesh = SCENE.golova.idle.mesh;
        golova_material = SCENE.golova.idle.material;
    } else if (SCENE.golova.state == GOLOVA_EAT) {
        golova_mesh = SCENE.golova.eat.mesh;
        golova_material = SCENE.golova.eat.material;
    }
    draw_mesh_t(golova_transform, golova_material, golova_mesh);

    // Golova Eyes
    float eyes_uplift = SCENE.golova.eyes_curr_uplift;
    float eyes_shift = SCENE.golova.eyes_curr_shift;
    float eyes_scale = SCENE.golova.eyes_idle_scale;
    float eyes_spread = SCENE.golova.eyes_idle_spread;

    Matrix s = MatrixScale(eyes_scale, eyes_scale, eyes_scale);
    Matrix left_t = MatrixTranslate(-eyes_spread / 2.0 + eyes_shift, -0.01, -eyes_uplift);
    Matrix right_t = MatrixTranslate(eyes_spread / 2.0 + eyes_shift, -0.01, -eyes_uplift);

    Matrix left_mat = MatrixMultiply(s, MatrixMultiply(left_t, golova_mat));
    Matrix right_mat = MatrixMultiply(s, MatrixMultiply(right_t, golova_mat));

    Material material = SCENE.golova.eyes_material;

    material.maps[0].texture = SCENE.golova.eye_left.texture;
    draw_mesh_m(left_mat, material, SCENE.golova.eye_left.mesh);

    material.maps[0].texture = SCENE.golova.eye_right.texture;
    draw_mesh_m(right_mat, material, SCENE.golova.eye_right.mesh);

    // Eyes background
    s = MatrixScale(0.75, 1.0, 0.15);
    Matrix t = MatrixTranslate(0.0, -0.02, -eyes_uplift);
    Matrix eyes_background_mat = MatrixMultiply(s, MatrixMultiply(t, golova_mat));

    MATERIAL_DEFAULT.maps[0].color = LIGHTGRAY;
    draw_mesh_m(eyes_background_mat, MATERIAL_DEFAULT, golova_mesh);

    // Items
    draw_items();

    EndMode3D();
    EndTextureMode();
}

void draw_postfx_ex(Texture2D texture, bool with_blur) {
    BeginShaderMode(POSTFX_SHADER);
    int u_with_blur = (int)with_blur;
    SetShaderValue(
        POSTFX_SHADER,
        GetShaderLocation(POSTFX_SHADER, "u_with_blur"),
        &u_with_blur,
        SHADER_UNIFORM_INT
    );
    DrawTextureRec(
        texture,
        (Rectangle){0, 0, (float)texture.width, (float)-texture.height},
        (Vector2){0, 0},
        WHITE
    );
    EndShaderMode();
}

void draw_postfx(bool with_blur) {
    draw_postfx_ex(SCREEN.texture, with_blur);
}

static char* load_shader_src(const char* file_path) {
    long size;

    static char* version = "#version 330";
    char* common = read_cstr_file("resources/shaders/common.glsl", "r", &size);

    char* text = read_cstr_file(file_path, "r", &size);
    char* src = malloc(strlen(version) + strlen(common) + strlen(text) + 6);

    size_t p = 0;
    strcpy(&src[p], version);
    p += strlen(version);
    src[p] = '\n';
    p += 1;
    strcpy(&src[p], common);
    p += strlen(common);
    src[p] = '\n';
    p += 1;
    strcpy(&src[p], text);

    free(common);
    free(text);

    return src;
}

static Shader load_shader(const char* vs_file_path, const char* fs_file_path) {
    char* vs = NULL;
    char* fs = NULL;

    if (vs_file_path) vs = load_shader_src(vs_file_path);
    if (fs_file_path) fs = load_shader_src(fs_file_path);
    Shader shader = LoadShaderFromMemory(vs, fs);

    if (vs) free(vs);
    if (fs) free(fs);
    return shader;
}
