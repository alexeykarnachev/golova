#include <raylib.h>
#include <raymath.h>


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

float vec2_angle(Vector2 v1, Vector2 v2) {
    static float eps = 0.00001;
    Vector2 v1_norm = Vector2Normalize(v1);
    Vector2 v2_norm = Vector2Normalize(v2);
    float dot = Vector2DotProduct(v1_norm, v2_norm);
    if (1.0 - fabs(dot) < eps) {
        return 0.0;
    }
    float angle = acos(dot);
    float z = v1.x * v2.y - v1.y * v2.x;
    if (fabs(z) < eps) {
        return 0.0;
    } else if (z > 0) {
        return angle;
    } else {
        return -angle;
    }
}
