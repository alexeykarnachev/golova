#pragma once

#include "raylib.h"
#include "scene.h"

typedef struct CollisionInfo {
    RayCollision collision;
    Entity* entity;
} CollisionInfo;

CollisionInfo cast_ray(Ray ray);
