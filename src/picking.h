#pragma once

#include "raylib.h"
#include <stddef.h>

int pick_model(Model** models, size_t n_models, Vector2 mouse_position);
void load_picking(void);
void unload_picking(void);
