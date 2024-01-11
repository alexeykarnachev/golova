#pragma once

#include "raylib.h"

#define MAX_N_SCREENS 4

extern RenderTexture2D SCREENS[MAX_N_SCREENS];

extern RenderTexture2D* FULL_SCREEN;
extern RenderTexture2D* THIRD_SCREEN;
extern RenderTexture2D* PICKING_SCREEN;

void load_drawing(void);
void unload_drawing(void);
void draw_entities(void);
void draw_screen_ex(RenderTexture2D* screen, Vector2 position);
void draw_screen(RenderTexture2D* screen);
void draw_screen_top_right(RenderTexture2D* screen);
