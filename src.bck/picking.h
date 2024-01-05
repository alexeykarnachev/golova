#pragma once

#include "raylib.h"
#include <stddef.h>

int pick_mesh_3d(
    Camera3D camera, Mesh* meshes, size_t n_meshes, Vector2 mouse_position
);
void load_picking(void);
void unload_picking(void);
