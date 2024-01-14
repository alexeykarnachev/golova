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
    char name[MAX_ITEM_NAME_LENGTH];
    bool is_correct;
    bool is_alive;

    Matrix matrix;
    Material material;
    Mesh mesh;
} Item;

typedef struct Board {
    Transform transform;
    Material material;
    Mesh mesh;

    int n_misses_allowed;
    int n_hits_required;

    int n_items;
    Item items[MAX_N_BOARD_ITEMS];

    char rule[MAX_RULE_LENGTH];

    // Drawing
    float board_scale;
    float item_scale;
    float item_elevation;
} Board;

typedef struct Scene {
    Golova golova;
    Board board;
    Camera3D camera;
} Scene;

extern Scene SCENE;

void load_scene(void);
void update_scene(void);
void draw_scene(void);
