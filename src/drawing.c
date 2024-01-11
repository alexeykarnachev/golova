#include "drawing.h"

#include "entities.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <stddef.h>

static int SCREEN_WIDTH = 1024;
static int SCREEN_HEIGHT = 768;

size_t N_SCREENS;

RenderTexture2D SCREENS[MAX_N_SCREENS];

RenderTexture2D* FULL_SCREEN;
RenderTexture2D* THIRD_SCREEN;
RenderTexture2D* PICKING_SCREEN;

static void rl_transform(Transform transform);

void load_drawing(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Golova");

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

void draw_entities(void) {
    for (size_t i = 0; i < N_ENTITIES; ++i) {
        Entity* e = &ENTITIES[i];

        rlPushMatrix();
        {
            rl_transform(e->transform);
            DrawMesh(*e->mesh, *e->material, MatrixIdentity());
        }
        rlPopMatrix();
    }
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

static void rl_transform(Transform transform) {
    Vector3 axis;
    float angle;
    Vector3 t = transform.translation;
    Vector3 s = transform.scale;
    Quaternion q = transform.rotation;
    QuaternionToAxisAngle(q, &axis, &angle);

    rlTranslatef(t.x, t.y, t.z);
    rlRotatef(angle * RAD2DEG, axis.x, axis.y, axis.z);
    rlScalef(s.x, s.y, s.z);
}
