#pragma once

#include "raylib.h"

typedef struct Golova {
    Model model;
} Golova;

typedef struct Ground {
    Model model;
} Ground;

typedef struct Scene {
    Golova golova;
    Ground ground;
} Scene;

extern Scene SCENE;

void load_scene(void);
