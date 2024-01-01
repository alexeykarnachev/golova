#include "resources.h"

#include "raylib.h"

Shader SPRITE_SHADER;
Texture GOLOVA_TEXTURE;

void load_resources(void) {
    SPRITE_SHADER = LoadShader(0, "resources/shaders/sprite.frag");
    GOLOVA_TEXTURE = LoadTexture("resources/textures/golova.png");
}

void unload_resources(void) {
    UnloadShader(SPRITE_SHADER);
    UnloadTexture(GOLOVA_TEXTURE);
}
