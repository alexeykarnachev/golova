#include "entities.h"

#include "raylib.h"
#include "raymath.h"
#include "resources.h"
#include <stddef.h>

size_t N_ENTITIES;
Entity ENTITIES[MAX_N_ENTITIES];

Entity* create_entity(void) {
    Entity* entity = &ENTITIES[N_ENTITIES++];
    entity->transform.rotation.w = 1.0;
    entity->transform.scale = Vector3One();
    entity->mesh = DEFAULT_CUBE_MESH;
    entity->material = DEFAULT_MATERIAL;
    return entity;
}

Entity* create_sprite_entity(const char* texture_file_path) {
    Entity* entity = create_entity();
    entity->material = load_sprite_material(texture_file_path);

    Texture2D* texture = &entity->material->maps[0].texture;
    float aspect = (float)texture->width / texture->height;
    entity->mesh = load_plane_mesh(aspect);

    return entity;
}
