#pragma once

#include "raylib.h"

extern Shader SPRITE_SHADER;
extern Shader GROUND_SHADER;
extern Shader ITEMS_SHADER;
extern Texture GOLOVA_TEXTURE;
extern Texture ITEMS_TEXTURE;

void load_resources(void);
void unload_resources(void);
