#pragma once

#include "raylib.h"
#include <stddef.h>

#define SCENE_MAX_N_MODELS 256

typedef struct Golova {
    size_t model_id;
} Golova;

typedef struct Ground {
    size_t model_id;
} Ground;

typedef struct GameCamera {
    Camera3D c3d;
    size_t model_id;
} GameCamera;

typedef struct Scene {
    Model models[SCENE_MAX_N_MODELS];
    size_t n_models;

    Golova golova;
    Ground ground;
    GameCamera camera;
} Scene;

extern Scene SCENE;

void load_scene(void);
