#pragma once

#include "raylib.h"
#include <stddef.h>

#define MAX_N_ENTITIES 1024

typedef enum EntityTag {
    CAMERA_SHELL_ENTITY = 1 << 0,
} EntityTag;

typedef struct Entity {
    EntityTag tag;
    Transform transform;
    Mesh* mesh;
    Material* material;
} Entity;

typedef struct Scene {
    size_t n_entities;
    Entity entities[MAX_N_ENTITIES];

    Camera3D camera;
} Scene;

extern Scene SCENE;

void load_scene(void);

bool entity_has_tag(Entity* e, EntityTag tag);

Entity* create_entity(void);
Entity* create_texture_sprite_entity(const char* texture_file_path);
Entity* create_shader_sprite_entity(const char* fs_file_path);

void update_scene(void);
void draw_scene(void);

void unload_scene(void);
