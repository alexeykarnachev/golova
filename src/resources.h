#pragma once
#include "raylib.h"

#define MAX_N_MATERILAS 256
#define MAX_N_MESHES 256

extern Material MATERIALS[MAX_N_MATERILAS];
extern Mesh MESHES[MAX_N_MESHES];

extern Material* DEFAULT_MATERIAL;
extern Material* SPRITE_MATERIAL;

extern Mesh* DEFAULT_PLANE_MESH;
extern Mesh* DEFAULT_SPHERE_MESH;
extern Mesh* DEFAULT_CUBE_MESH;

void load_resources(void);
Material* load_material(void);
Material* load_sprite_material(const char* texture_file_path);
Material* load_shader_material(const char* vs_file_path, const char* fs_file_path);
Mesh* load_sphere_mesh(float radius);
Mesh* load_plane_mesh(float aspect);
void unload_resources(void);
