#pragma once

#include "raylib.h"
#include "rcamera.h"

void update_orbital_camera(
    Camera3D* camera, bool is_mmb_down, bool is_shift_down, Vector2 mouse_delta
);
