#include "../src/cimgui_utils.h"
#include "../src/nfd_utils.h"
#include "nfd.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <stdio.h>
#include <stdlib.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

// int SCREEN_WIDTH = 1024;
// int SCREEN_HEIGHT = 768;
int SCREEN_WIDTH = 2560;
int SCREEN_HEIGHT = 1440;

Color BACKGROUND_COLOR = (Color){50, 100, 100, 255};
nfdfilteritem_t NFD_TEXTURE_FILTER[1] = {{"Texture", "png"}};

char* ATLAS_FILE_PATH = NULL;
Texture2D ATLAS_TEXTURE;

static Rectangle get_atlas_dest_rect(float scale) {
    float screen_width = (float)GetScreenWidth();
    float screen_height = (float)GetScreenHeight();
    Vector2 screen_mid = {screen_width / 2.0f, screen_height / 2.0};

    float width = (float)ATLAS_TEXTURE.width;
    float height = (float)ATLAS_TEXTURE.height;
    float aspect = width / height;

    Vector2 size;
    if (width > height) {
        size.x = scale * screen_width;
        size.y = size.x / aspect;
    } else {
        size.y = scale * screen_height;
        size.x = size.y * aspect;
    }

    Rectangle dest = {
        screen_mid.x - 0.5 * size.x,
        screen_mid.y - 0.5 * size.y,
        size.x,
        size.y};
    return dest;
}

static void draw_imgui(void) {
    ig_fix_window_top_left();
    if (igBegin("Inspector", NULL, 0)) {
        if (ig_collapsing_header("Atlas", true)) {
            igSeparatorText("Texture");
            if (igButton("Import", (ImVec2){100, 20})) {
                char* fp = open_nfd(".", NFD_TEXTURE_FILTER, 1);
                if (fp != NULL) {
                    if (ATLAS_FILE_PATH != NULL) {
                        free(ATLAS_FILE_PATH);
                    }
                    ATLAS_FILE_PATH = fp;

                    if (IsTextureReady(ATLAS_TEXTURE)) {
                        UnloadTexture(ATLAS_TEXTURE);
                    }

                    ATLAS_TEXTURE = LoadTexture(ATLAS_FILE_PATH);
                }
            }

            if (IsTextureReady(ATLAS_TEXTURE)) {
                Rectangle rect = get_atlas_dest_rect(0.1);
                igImage(
                    (ImTextureID)(unsigned long)ATLAS_TEXTURE.id,
                    (ImVec2){rect.width, rect.height},
                    (ImVec2){0.0, 0.0},
                    (ImVec2){1.0, 1.0},
                    (ImVec4){1.0, 1.0, 1.0, 1.0},
                    (ImVec4){0.0, 0.0, 0.0, 1.0}
                );
            }
        }
    }
    igEnd();
}

int main(void) {
    // -------------------------------------------------------------------
    // Load window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Items Editor");
    SetTargetFPS(60);

    load_imgui();

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);
        {
            begin_imgui();
            draw_imgui();
            end_imgui();

            if (IsTextureReady(ATLAS_TEXTURE)) {
                Rectangle dest = get_atlas_dest_rect(0.9);
                Rectangle source = {
                    0.0, 0.0, ATLAS_TEXTURE.width, ATLAS_TEXTURE.height};
                DrawTexturePro(
                    ATLAS_TEXTURE, source, dest, Vector2Zero(), 0.0, WHITE
                );
            }
        }
        EndDrawing();
    }

    return 0;
}
