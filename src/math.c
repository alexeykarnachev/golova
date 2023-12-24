#include <raylib.h>


RayCollision get_ray_collision_plane(Ray ray, Vector3 plane_point, Vector3 plane_normal) {
    RayCollision collision = {0};
    collision.hit = false;

    float denominator = ray.direction.x * plane_normal.x +
                        ray.direction.y * plane_normal.y +
                        ray.direction.z * plane_normal.z;

    if (denominator == 0) {
        // Ray is parallel to the plane, no collision
        return collision;
    }

    float t = ((plane_point.x - ray.position.x) * plane_normal.x +
               (plane_point.y - ray.position.y) * plane_normal.y +
               (plane_point.z - ray.position.z) * plane_normal.z) / denominator;

    if (t < 0) {
        // Intersection point is behind the ray's starting point, no collision
        return collision;
    }

    // Calculate the collision point
    collision.point.x = ray.position.x + t * ray.direction.x;
    collision.point.y = ray.position.y + t * ray.direction.y;
    collision.point.z = ray.position.z + t * ray.direction.z;

    collision.hit = true;
    return collision;
}
