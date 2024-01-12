#include "scene.h"

#include "math.h"
#include "raylib.h"
#include "raymath.h"
#include "resources.h"
#include "rlgl.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

Scene SCENE;

static void draw_camera_ray(void);

void load_scene(void) {
    // Camera
    SCENE.camera.fovy = 60.0;
    SCENE.camera.projection = CAMERA_PERSPECTIVE;
    SCENE.camera.up = (Vector3){0.0, 1.0, 0.0};
    SCENE.camera.position = (Vector3){0.0, 2.0, 2.0};

    // Camera shell
    Entity* camera_shell = create_entity();
    camera_shell->transform.translation = SCENE.camera.position;
    camera_shell->mesh = load_sphere_mesh(0.2);
    camera_shell->material->maps[0].color = RAYWHITE;
    camera_shell->tag |= CAMERA_SHELL_ENTITY;
}

bool entity_has_tag(Entity* e, EntityTag tag) {
    return (e->tag & tag) == tag;
}

Entity* create_entity(void) {
    Entity* entity = &SCENE.entities[SCENE.n_entities++];
    entity->transform.rotation.w = 1.0;
    entity->transform.scale = Vector3One();
    entity->mesh = DEFAULT_CUBE_MESH;
    entity->material = DEFAULT_MATERIAL;
    return entity;
}

Entity* create_texture_sprite_entity(const char* texture_file_path) {
    Entity* entity = create_entity();
    entity->material = load_sprite_material(texture_file_path);

    Texture2D* texture = &entity->material->maps[0].texture;
    float aspect = (float)texture->width / texture->height;
    entity->mesh = load_plane_mesh(aspect);

    return entity;
}

Entity* create_shader_sprite_entity(const char* fs_file_path) {
    Entity* entity = create_entity();
    entity->material = load_shader_material(0, fs_file_path);
    entity->mesh = DEFAULT_PLANE_MESH;

    return entity;
}

void update_scene(void) {
    for (size_t i = 0; i < SCENE.n_entities; ++i) {
        Entity* e = &SCENE.entities[i];
        Transform* t = &e->transform;

        // Update camera by camera shell
        if (entity_has_tag(e, CAMERA_SHELL_ENTITY)) {
            SCENE.camera.position = t->translation;
            Vector3 dir = Vector3RotateByQuaternion(
                (Vector3){0.0, 0.0, -1.0}, t->rotation
            );
            SCENE.camera.target = Vector3Add(SCENE.camera.position, dir);
        }
    }
}

void draw_scene(void) {
    // Draw entities
    for (size_t i = 0; i < SCENE.n_entities; ++i) {
        Entity* e = &SCENE.entities[i];
        if (!e->mesh) continue;

        rlPushMatrix();
        {
            Matrix m = get_transform_matrix(e->transform);
            rlMultMatrixf(MatrixToFloat(m));
            DrawMesh(*e->mesh, *e->material, MatrixIdentity());
        }
        rlPopMatrix();
    }

    draw_camera_ray();
}

void unload_scene(void) {
    SCENE = (Scene){0};
}

static void draw_camera_ray(void) {
    rlSetLineWidth(5.0);
    Vector3 start = SCENE.camera.position;
    Vector3 target = SCENE.camera.target;
    Vector3 dir = Vector3Normalize(Vector3Subtract(target, start));
    Vector3 end = Vector3Add(start, Vector3Scale(dir, 1.0));
    DrawLine3D(start, end, PINK);
}
