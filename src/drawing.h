#pragma once

#include "raylib.h"

void draw_screen_ex(RenderTexture2D screen, Vector2 position);
void draw_screen(RenderTexture2D screen);
void draw_screen_top_right(RenderTexture2D screen);
void draw_mesh_t(Transform transform, Material material, Mesh mesh);
void draw_mesh_m(Matrix m, Material material, Mesh mesh);
