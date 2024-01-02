#pragma once

#include "raylib.h"
#include <stddef.h>

#define SCENE_MAX_N_MODELS 256

typedef struct Golova {
    size_t model_idx;
} Golova;

typedef struct Ground {
    size_t model_idx;
} Ground;

typedef struct Scene {
    Model models[SCENE_MAX_N_MODELS];
    size_t n_models;

    Golova golova;
    Ground ground;
    Camera3D camera;
} Scene;

extern Scene SCENE;

void load_scene(void);
