#pragma once

#include "raylib.h"
#include <stddef.h>

#define SCENE_MAX_N_ENTITIES 256

typedef struct GameCamera {
    int id;

    Camera3D c3d;
} GameCamera;

typedef struct Golova {
    int id;
} Golova;

typedef struct Ground {
    int id;

    size_t grid_size;
    Model item_model;

    Transform transform;
} Ground;

typedef struct Scene {
    Model models[SCENE_MAX_N_ENTITIES];
    Transform transforms[SCENE_MAX_N_ENTITIES];
    size_t n_entities;

    Golova golova;
    Ground ground;
    GameCamera camera;
} Scene;

// typedef struct SceneSaveData {
//     Matrix golova_transform;
//     Matrix ground_transform;
//     Camera3D c3d;
// } SceneSaveData;

extern Scene SCENE;

void load_scene(const char* file_path);
// void save_scene(const char* file_path);
