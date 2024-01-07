#include "../src/cimgui_utils.h"
#include "../src/nfd_utils.h"
#include "nfd.h"
#include "raylib.h"
#include <stdio.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

// int SCREEN_WIDTH = 1024;
// int SCREEN_HEIGHT = 768;
int SCREEN_WIDTH = 2560;
int SCREEN_HEIGHT = 1440;

Color BACKGROUND_COLOR = (Color){50, 100, 100, 255};
nfdfilteritem_t NFD_TEXTURE_FILTER[1] = {{"Texture", "png"}};

static void draw_imgui(void) {
    ig_fix_window_top_left();
    if (igBegin("Inspector", NULL, 0)) {
        if (ig_collapsing_header("Texture", true)) {
            if (igButton("Import", (ImVec2){100, 20})) {
                char* fp = open_nfd(".", NFD_TEXTURE_FILTER, 1);
                if (fp != NULL) {
                    printf("%s\n", fp);
                }
                // NFD_FreePathN(fp);
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
        }
        EndDrawing();
    }

    return 0;
}
