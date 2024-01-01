#pragma once

#include "raylib.h"

typedef struct Golova {
    Model model;
} Golova;

typedef struct Scene {
    Golova golova;
} Scene;

extern Scene SCENE;

void load_scene(void);
