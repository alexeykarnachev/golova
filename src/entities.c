#include "entities.h"

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
