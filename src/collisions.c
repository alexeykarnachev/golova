#include "collisions.h"

#include "math.h"
#include "resources.h"

CollisionInfo cast_ray(Ray ray) {
    RayCollision nearest_collision = {0};
    Entity* nearest_entity = NULL;
    Item* nearest_item = NULL;
    for (size_t i = 0; i < SCENE.n_entities; ++i) {
        Entity* entity = &SCENE.entities[i];
        Mesh* mesh = entity->mesh;
        if (!mesh) continue;

        BoundingBox box = get_mesh_bounding_box(
            *mesh, get_transform_matrix(entity->transform)
        );
        RayCollision collision = GetRayCollisionBox(ray, box);
        if (!collision.hit) continue;

        bool is_closer = collision.distance < nearest_collision.distance;
        if (!nearest_collision.hit || is_closer) {
            nearest_collision = collision;
            nearest_entity = entity;
        }
    }

    for (size_t i = 0; i < SCENE.board.n_items; ++i) {
        Item* item = &SCENE.board.items[i];

        BoundingBox box = get_mesh_bounding_box(*DEFAULT_PLANE_MESH, item->mat);
        RayCollision collision = GetRayCollisionBox(ray, box);
        if (!collision.hit) continue;

        bool is_closer = collision.distance < nearest_collision.distance;
        if (!nearest_collision.hit || is_closer) {
            nearest_collision = collision;
            nearest_item = item;
            nearest_entity = NULL;
        }
    }

    CollisionInfo info = {
        .collision = nearest_collision, .entity = nearest_entity, .item = nearest_item};
    return info;
}
