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

void load_scene(void) {
    // Camera shell
    Vector3 camera_position = {0.0, 2.0, 2.0};
    SCENE.camera_shell.entity = create_entity();
    SCENE.camera_shell.entity->transform.translation = camera_position;
    SCENE.camera_shell.entity->mesh = load_sphere_mesh(0.2);
    SCENE.camera_shell.entity->material->maps[0].color = RAYWHITE;
    SCENE.camera_shell.camera.fovy = 60.0;
    SCENE.camera_shell.camera.projection = CAMERA_PERSPECTIVE;
    SCENE.camera_shell.camera.up = (Vector3){0.0, 1.0, 0.0};
    SCENE.camera_shell.camera.position = camera_position;

    // Board
    SCENE.board.entity = create_shader_sprite_entity("resources/shaders/board.frag");
    SCENE.board.board_scale = 0.7;
    SCENE.board.item_scale = 0.2;
    SCENE.board.item_elevation = 0.5;
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
    // Update camera by camera shell
    Entity* shell = SCENE.camera_shell.entity;
    Camera3D* camera = &SCENE.camera_shell.camera;
    camera->position = shell->transform.translation;
    Vector3 dir = Vector3RotateByQuaternion(
        (Vector3){0.0, 0.0, -1.0}, shell->transform.rotation
    );
    camera->target = Vector3Add(camera->position, dir);
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

    // Draw camera ray
    rlSetLineWidth(5.0);
    Vector3 start = SCENE.camera_shell.camera.position;
    Vector3 target = SCENE.camera_shell.camera.target;
    Vector3 dir = Vector3Normalize(Vector3Subtract(target, start));
    Vector3 end = Vector3Add(start, Vector3Scale(dir, 1.0));
    DrawLine3D(start, end, PINK);
}

void unload_scene(void) {
    SCENE = (Scene){0};
}
