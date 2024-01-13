#pragma once

#include "raylib.h"
#include <stddef.h>

#define MAX_N_ENTITIES 1024
#define MAX_ITEM_NAME_LENGTH 256
#define MAX_N_BOARD_ITEMS 64
#define MAX_RULE_LENGTH 1024

typedef struct Entity {
    Transform transform;
    Mesh* mesh;
    Material* material;
} Entity;

typedef struct CameraShell {
    Entity* entity;
    Camera3D camera;
} CameraShell;

typedef struct Item {
    char name[MAX_ITEM_NAME_LENGTH];
    bool is_correct;
    bool is_alive;

    Texture2D texture;
} Item;

typedef struct Board {
    Entity* entity;

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
    size_t n_entities;
    Entity entities[MAX_N_ENTITIES];

    CameraShell camera_shell;
    Board board;
} Scene;

extern Scene SCENE;

void load_scene(void);

Entity* create_entity(void);
Entity* create_texture_sprite_entity(const char* texture_file_path);
Entity* create_shader_sprite_entity(const char* fs_file_path);

void update_scene(void);
void draw_scene(void);

void unload_scene(void);
