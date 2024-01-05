#include "resources.h"

#include "raylib.h"

Shader SPRITE_SHADER;
Shader GROUND_SHADER;
Shader ITEMS_SHADER;
Texture GOLOVA_TEXTURE;
Texture ITEMS_TEXTURE;

void load_resources(void) {
    SPRITE_SHADER = LoadShader(0, "resources/shaders/sprite.frag");
    GROUND_SHADER = LoadShader(0, "resources/shaders/ground.frag");
    ITEMS_SHADER = LoadShader(0, "resources/shaders/items.frag");

    GOLOVA_TEXTURE = LoadTexture("resources/textures/golova.png");
    ITEMS_TEXTURE = LoadTexture("resources/textures/items_0.png");
}

void unload_resources(void) {
    UnloadShader(SPRITE_SHADER);
    UnloadShader(GROUND_SHADER);
    UnloadShader(ITEMS_SHADER);

    UnloadTexture(GOLOVA_TEXTURE);
    UnloadTexture(ITEMS_TEXTURE);
}
