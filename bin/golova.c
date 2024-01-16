#include "../src/cimgui_utils.h"
#include "../src/math.h"
#include "../src/scene.h"
#include "../src/utils.h"
#include "raylib.h"
#include "raymath.h"
#include <stdio.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
// #define SCREEN_WIDTH 2560
// #define SCREEN_HEIGHT 1440

typedef enum GameState {
    PLAYER_IS_PICKING = 0,
    GOLOVA_IS_EATING,
    GAME_OVER,
} GameState;

#define GAME_STATE_TO_NAME(state) \
    ((state == PLAYER_IS_PICKING)  ? "PLAYER_IS_PICKING" \
     : (state == GOLOVA_IS_EATING) ? "GOLOVA_IS_EATING" \
     : (state == GAME_OVER)        ? "GAME_OVER" \
                                   : "UNKNOWN")

static float GAME_STATE_TO_TIME[] = {1.0, 1.0, 0.0};

static char* SCENES_DIR = "resources/scenes";
static int CURR_SCENE_ID;
static char** SCENE_FILE_NAMES;
static int N_SCENES;

static GameState GAME_STATE;
static GameState NEXT_STATE;
static float TIME_REMAINING;

static Vector2 MOUSE_POSITION;
static bool IS_LMB_PRESSED;
static Ray MOUSE_RAY;

static Item* PICKED_ITEM;

static void load_curr_scene(void);
static void update_game(void);
static void draw_game_imgui(void);
static void draw_imgui(void);

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Golova");
    SetTargetFPS(60);

    SCENE_FILE_NAMES = get_file_names_in_dir(SCENES_DIR, &N_SCENES);

    load_curr_scene();
    load_imgui();

    while (!WindowShouldClose()) {
        update_scene();
        update_game();

        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode3D(SCENE.camera);
        draw_scene();
        EndMode3D();

        draw_game_imgui();

        begin_imgui();
        draw_imgui();
        end_imgui();

        EndDrawing();
    }

    return 0;
}

static void load_curr_scene(void) {
    static char fp[2048];
    sprintf(fp, "%s/%s", SCENES_DIR, SCENE_FILE_NAMES[CURR_SCENE_ID]);

    unload_scene();
    load_scene(fp);

    GAME_STATE = PLAYER_IS_PICKING;
    TIME_REMAINING = GAME_STATE_TO_TIME[GAME_STATE];
}

static void update_game(void) {
    MOUSE_POSITION = GetMousePosition();
    IS_LMB_PRESSED = IsMouseButtonPressed(0);
    MOUSE_RAY = GetMouseRay(MOUSE_POSITION, SCENE.camera);
    TIME_REMAINING -= GetFrameTime();

    if (NEXT_STATE != GAME_STATE) {
        GAME_STATE = NEXT_STATE;
        TIME_REMAINING = GAME_STATE_TO_TIME[GAME_STATE];
    }

    // -------------------------------------------------------------------
    // Update game state
    if (GAME_STATE == PLAYER_IS_PICKING) {

        // Handle mouse input and update item states
        bool is_hit_any = false;
        for (size_t i = 0; i < SCENE.board.n_items; ++i) {
            Item* item = &SCENE.board.items[i];

            // Don't update dead items
            if (item->is_dead) continue;

            // Collide mouse and item meshes
            RayCollision collision = GetRayCollisionMesh(
                MOUSE_RAY, SCENE.board.item_mesh, item->matrix
            );

            bool is_hit = collision.hit;
            is_hit_any |= is_hit;
            if (is_hit && IS_LMB_PRESSED) {
                // Unpick previous item and pick the new one
                if (PICKED_ITEM && PICKED_ITEM != item) {
                    PICKED_ITEM->state = ITEM_COLD;
                    PICKED_ITEM = item;
                    PICKED_ITEM->state = ITEM_ACTIVE;
                    // Unpick the item
                } else if (PICKED_ITEM) {
                    PICKED_ITEM->state = ITEM_COLD;
                    PICKED_ITEM = NULL;
                    // Pick the item
                } else {
                    PICKED_ITEM = item;
                    PICKED_ITEM->state = ITEM_ACTIVE;
                }
            } else if (is_hit) {
                // Heat up (or stay active) the item
                item->state = MAX(item->state, ITEM_HOT);
            } else if (item->state == ITEM_HOT) {
                // Cool down the hot item
                item->state = ITEM_COLD;
            }
        }

        // Unpick when clicked on empty space
        if (!is_hit_any && IS_LMB_PRESSED && PICKED_ITEM) {
            PICKED_ITEM->state = ITEM_COLD;
            PICKED_ITEM = NULL;
        }

        // PLAYER_IS_PICKING state is over
        if (TIME_REMAINING <= 0.0) {
            NEXT_STATE = GOLOVA_IS_EATING;
            if (!PICKED_ITEM) {
                for (size_t i = 0; i < SCENE.board.n_items; ++i) {
                    Item* item = &SCENE.board.items[i];
                    if (!item->is_dead && !item->is_correct) {
                        PICKED_ITEM = item;
                    }
                }
            }

            PICKED_ITEM->state = ITEM_DYING;
        }
    } else if (GAME_STATE == GOLOVA_IS_EATING) {
        // Cool down all non-dying items when golova is eating
        for (size_t i = 0; i < SCENE.board.n_items; ++i) {
            Item* item = &SCENE.board.items[i];
            if (item->state != ITEM_DYING) item->state = ITEM_COLD;
        }

        // GOLOVA_IS_EATING state is over
        if (TIME_REMAINING <= 0.0) {
            PICKED_ITEM->is_dead = true;
            if (PICKED_ITEM->is_correct) {
                SCENE.board.n_hits_required -= 1;
            } else {
                SCENE.board.n_misses_allowed -= 1;
            }
            PICKED_ITEM = NULL;

            if (SCENE.board.n_misses_allowed < 0 || SCENE.board.n_hits_required == 0) {
                NEXT_STATE = GAME_OVER;
            } else {
                NEXT_STATE = PLAYER_IS_PICKING;
            }
        }
    } else if (GAME_STATE == GAME_OVER) {
    }
}

static void draw_game_imgui(void) {
    int screen_height = GetScreenHeight();
    int screen_width = GetScreenWidth();

    if (GAME_STATE == GAME_OVER) {

        const char* text;
        Color color;
        if (SCENE.board.n_misses_allowed < 0) {
            text = "Golova feels bad...";
            color = MAROON;
        } else {
            text = "Golova is happy...";
            color = GREEN;
        }

        int font_size = 100;
        int text_width = MeasureText(text, font_size);
        int x = (screen_width - text_width) / 2;
        int y = screen_height / 2 - font_size;
        DrawText(text, x, y, font_size, color);

        text = SCENE.board.rule;
        font_size = 40;
        text_width = MeasureText(text, font_size);
        x = (screen_width - text_width) / 2;
        y = y - font_size * 1.2;
        DrawText(text, x, y, font_size, RAYWHITE);
    }
}

static void draw_imgui(void) {
    ig_fix_window_top_left();
    if (igBegin("Debug info", NULL, GHOST_WINDOW_FLAGS)) {
        igText("scene_file_name: %s", SCENE_FILE_NAMES[CURR_SCENE_ID]);
        igText("GAME_STATE: %s", GAME_STATE_TO_NAME(GAME_STATE));
        igText("TIME_REMAINING: %.2f", TIME_REMAINING);
        igText("rule: %s", SCENE.board.rule);
        igText("n_hits_required: %d", SCENE.board.n_hits_required);
        igText("n_misses_allowed: %d", SCENE.board.n_misses_allowed);

        const char* picked_item_name = "";
        const char* picked_item_state = "";
        if (PICKED_ITEM) {
            picked_item_name = PICKED_ITEM->name;
            picked_item_state = ITEM_STATE_TO_NAME(PICKED_ITEM->state);
        }
        igText("picked_item_name: %s", picked_item_name);
        igText("picked_item_state: %s", picked_item_state);
    }
    igEnd();
}
