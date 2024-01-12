#include "drawing.h"

#include "raymath.h"
#include <stdio.h>

static int SCREEN_WIDTH = 1024;
static int SCREEN_HEIGHT = 768;

size_t N_SCREENS;

RenderTexture2D SCREENS[MAX_N_SCREENS];

RenderTexture2D* FULL_SCREEN;
RenderTexture2D* THIRD_SCREEN;
RenderTexture2D* PICKING_SCREEN;

void load_drawing(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Golova");
    SetTargetFPS(60);

    FULL_SCREEN = &SCREENS[N_SCREENS++];
    *FULL_SCREEN = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);

    THIRD_SCREEN = &SCREENS[N_SCREENS++];
    *THIRD_SCREEN = LoadRenderTexture(SCREEN_WIDTH / 3, SCREEN_HEIGHT / 3);

    PICKING_SCREEN = &SCREENS[N_SCREENS++];
    *PICKING_SCREEN = LoadRenderTexture(SCREEN_WIDTH / 3, SCREEN_HEIGHT / 3);
}

void unload_drawing() {
    for (size_t i = 0; N_SCREENS != 0; ++i, --N_SCREENS)
        UnloadRenderTexture(SCREENS[i]);
}

void draw_screen_ex(RenderTexture2D* screen, Vector2 position) {
    int w, h;
    Rectangle r;

    w = screen->texture.width;
    h = screen->texture.height;
    r = (Rectangle){0, 0, w, -h};
    DrawTextureRec(screen->texture, r, position, WHITE);
}

void draw_screen(RenderTexture2D* screen) {
    draw_screen_ex(screen, Vector2Zero());
}

void draw_screen_to_right(RenderTexture2D* screen) {
    Vector2 position = {SCREEN_WIDTH - screen->texture.width, 0.0};
    draw_screen_ex(screen, position);
}
