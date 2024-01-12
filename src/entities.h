#pragma once

#include "raylib.h"
#include <stddef.h>

#define MAX_N_ENTITIES 1024

typedef enum EntityTag {
    DEFAULT = 1 << 0,
} EntityTag;

typedef struct Entity {
    EntityTag tag;
    Transform transform;
    Mesh* mesh;
    Material* material;
} Entity;

extern size_t N_ENTITIES;
extern Entity ENTITIES[MAX_N_ENTITIES];

Entity* create_entity(void);
Entity* create_sprite_entity(const char* texture_file_path);
