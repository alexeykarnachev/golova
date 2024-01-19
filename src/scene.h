#pragma once

#include "raylib.h"
#include <stddef.h>

#define MAX_N_BOARD_ITEMS 64
#define MAX_ITEM_NAME_LENGTH 128
#define MAX_RULE_LENGTH 128

typedef enum GolovaState {
    GOLOVA_IDLE = 0,
    GOLOVA_EAT,
} GolovaState;

typedef struct Golova {
    Transform transform;
    GolovaState state;

    struct {
        Material material;
        Mesh mesh;
    } idle;

    struct {
        Material material;
        Mesh mesh;
    } eat;

    // Eyes
    float eyes_idle_scale;
    float eyes_idle_uplift;
    float eyes_idle_shift;
    float eyes_idle_spread;

    float eyes_curr_shift;
    float eyes_curr_uplift;

    Material eyes_material;

    struct {
        Texture2D texture;
        Mesh mesh;
    } eye_left;

    struct {
        Texture2D texture;
        Mesh mesh;
    } eye_right;
} Golova;

typedef enum ItemState {
    ITEM_COLD = 0,
    ITEM_HOT,
    ITEM_ACTIVE,
    ITEM_DYING,
    ITEM_DEAD,
} ItemState;

#define ITEM_STATE_TO_NAME(state) \
    ((state == ITEM_COLD)     ? "ITEM_COLD" \
     : (state == ITEM_HOT)    ? "ITEM_HOT" \
     : (state == ITEM_ACTIVE) ? "ITEM_ACTIVE" \
     : (state == ITEM_DYING)  ? "ITEM_DYING" \
                              : "UNKNOWN")

typedef struct Item {
    Texture2D texture;
    Matrix matrix;

    bool is_correct;
    ItemState state;

    char name[MAX_ITEM_NAME_LENGTH];
} Item;

typedef struct Board {
    Transform transform;
    Material material;
    Mesh mesh;

    char rule[MAX_RULE_LENGTH];

    int n_misses_allowed;
    int n_hits_required;
    float board_scale;
    float item_scale;
    float item_elevation;

    Material item_material;
    Mesh item_mesh;
    int n_items;
    Item items[MAX_N_BOARD_ITEMS];
} Board;

typedef struct Scene {
    Golova golova;
    Board board;
    Camera3D camera;
} Scene;

extern Scene SCENE;

void init_core(int screen_width, int screen_height);
void load_scene(const char* file_path);
void save_scene(const char* file_path);
void draw_scene(bool with_shadows);
void draw_scene_ex(
    RenderTexture2D screen, Color clear_color, Camera3D camera, bool with_shadows
);
void draw_postfx(bool is_blured);
void draw_postfx_ex(Texture2D texture, bool is_blured);
