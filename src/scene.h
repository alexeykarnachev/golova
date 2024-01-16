#pragma once

#include "raylib.h"
#include <stddef.h>

#define MAX_N_BOARD_ITEMS 64
#define MAX_ITEM_NAME_LENGTH 128
#define MAX_RULE_LENGTH 128

typedef struct Golova {
    Transform transform;
    Material material;
    Mesh mesh;
} Golova;

typedef struct Item {
    Matrix matrix;
    Material material;
    Mesh mesh;

    bool is_correct;
    bool is_alive;

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

    int n_items;
    Item items[MAX_N_BOARD_ITEMS];
} Board;

typedef struct Scene {
    Golova golova;
    Board board;
    Camera3D camera;
} Scene;

extern Scene SCENE;

void load_scene(const char* file_path);
void save_scene(const char* file_path);
void unload_scene(void);
void update_scene(void);
void draw_scene(void);
