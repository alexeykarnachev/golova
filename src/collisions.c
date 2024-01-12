#include "collisions.h"

#include "math.h"

CollisionInfo cast_ray(Ray ray) {
    RayCollision nearest_collision = {0};
    Entity* nearest_entity = NULL;
    for (size_t i = 0; i < SCENE.n_entities; ++i) {
        Entity* entity = &SCENE.entities[i];
        Mesh* mesh = entity->mesh;
        if (!mesh) continue;

        BoundingBox box = get_mesh_bounding_box(*mesh, entity->transform);
        RayCollision collision = GetRayCollisionBox(ray, box);
        if (!collision.hit) continue;

        bool is_closer = collision.distance < nearest_collision.distance;
        if (!nearest_collision.hit || is_closer) {
            nearest_collision = collision;
            nearest_entity = entity;
        }
    }

    CollisionInfo info = {
        .collision = nearest_collision, .entity = nearest_entity};
    return info;
}
