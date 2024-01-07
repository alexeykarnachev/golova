#include "../src/cimgui_utils.h"
#include "../src/nfd_utils.h"
#include "nfd.h"
#include "raylib.h"
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

char* ITEMS_ATLAS_FILE_PATH = NULL;
Texture2D ITEMS_ATLAS;

static void draw_imgui(void) {
    ig_fix_window_top_left();
    if (igBegin("Inspector", NULL, 0)) {
        if (ig_collapsing_header("Items", true)) {
            igSeparatorText("Atlas");
            if (igButton("Import", (ImVec2){100, 20})) {
                char* fp = open_nfd(".", NFD_TEXTURE_FILTER, 1);
                if (fp != NULL) {
                    if (ITEMS_ATLAS_FILE_PATH != NULL)
                        free(ITEMS_ATLAS_FILE_PATH);
                    ITEMS_ATLAS_FILE_PATH = fp;

                    if (IsTextureReady(ITEMS_ATLAS)) {
                        UnloadTexture(ITEMS_ATLAS);
                    }

                    ITEMS_ATLAS = LoadTexture(ITEMS_ATLAS_FILE_PATH);
                }
            }

            if (IsTextureReady(ITEMS_ATLAS)) {
                igImage(
                    (ImTextureID)(unsigned long)ITEMS_ATLAS.id,
                    (ImVec2){400.0, 400.0},
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
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Golova");
    SetTargetFPS(60);

    load_imgui();

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);
        {
            begin_imgui();
            draw_imgui();
            end_imgui();

            if (IsTextureReady(ITEMS_ATLAS)) {
                DrawTexture(ITEMS_ATLAS, 0, 0, WHITE);
            }
        }
        EndDrawing();
    }

    return 0;
}
