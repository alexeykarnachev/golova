#pragma once
#include "raylib.h"
#include <stdint.h>

#define MAX_N_ENTITIES 256
#define MAX_N_MATERIALS 256
#define MAX_N_MESHES 256
#define MAX_N_SCREENS 4
#define MAX_N_CAMERAS 4

typedef struct Scene {
    struct {
        int n_screens;
        int n_materials;
        int n_meshes;

        RenderTexture screen[MAX_N_SCREENS];
        Material material[MAX_N_MATERIALS];
        Mesh mesh[MAX_N_MESHES];
    } resource;

    struct {
        int n_entities;
        int material[MAX_N_ENTITIES];
        int mesh[MAX_N_ENTITIES];
        uint64_t component[MAX_N_ENTITIES];
        Transform transform[MAX_N_ENTITIES];
    } entity;

    int n_cameras;
    Camera3D camera[MAX_N_CAMERAS];
} Scene;

typedef enum Component {
    MESH_COMPONENT = 1 << 0,
    MATERIAL_COMPONENT = 1 << 1,
    NO_DRAW_COMPONENT = 1 << 3,
} Component;

extern Scene SCENE;

void create_scene(void);

int create_screen(RenderTexture screen);
int create_material(Material material);
int create_mesh(Mesh mesh);
int create_entity(void);
int create_camera(void);

RenderTexture* get_screen(int screen_id);
Camera3D* get_camera(int camera_id);
Mesh* get_mesh(int mesh_id);
Material* get_material(int material_id);

Transform* get_entity_transform(int entity_id);
Mesh* get_entity_mesh(int entity_id);
Material* get_entity_material(int entity_id);

void attach_entity_mesh(int entity_id, int mesh_id);
void attach_entity_material(int entity_id, int material_id);

void set_entity_component(int entity_id, Component component);
void set_entity_scale(int entity_id, Vector3 scale);
void set_entity_scalef(int entity_id, float scale);

bool check_if_entity_has_component(int entity_id, Component component);

void unload_scene(void);