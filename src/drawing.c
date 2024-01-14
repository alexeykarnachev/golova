#include "drawing.h"

#include "math.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <stdio.h>

void draw_screen_ex(RenderTexture2D screen, Vector2 position) {
    int w, h;
    Rectangle r;

    w = screen.texture.width;
    h = screen.texture.height;
    r = (Rectangle){0, 0, w, -h};
    DrawTextureRec(screen.texture, r, position, WHITE);
}

void draw_screen(RenderTexture2D screen) {
    draw_screen_ex(screen, Vector2Zero());
}

void draw_screen_top_right(RenderTexture2D screen) {
    Vector2 position = {GetScreenWidth() - screen.texture.width, 0.0};
    draw_screen_ex(screen, position);
}

void draw_mesh_t(Transform transform, Material material, Mesh mesh) {
    draw_mesh_m(get_transform_matrix(transform), material, mesh);
}

void draw_mesh_m(Matrix matrix, Material material, Mesh mesh) {
    rlPushMatrix();
    {
        rlMultMatrixf(MatrixToFloat(matrix));
        DrawMesh(mesh, material, MatrixIdentity());
    }
    rlPopMatrix();
}
