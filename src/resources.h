#pragma once
#include "raylib.h"

#define MAX_N_MATERILAS 256
#define MAX_N_MESHES 256

extern Material MATERIALS[MAX_N_MATERILAS];
extern Mesh MESHES[MAX_N_MESHES];

extern Material* DEFAULT_MATERIAL;
extern Mesh* DEFAULT_PLANE_MESH;
extern Mesh* DEFAULT_SPHERE_MESH;
extern Mesh* DEFAULT_CUBE_MESH;

void load_resources(void);
void unload_resources(void);
