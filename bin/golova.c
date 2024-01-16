#include "../src/cimgui_utils.h"
#include "../src/math.h"
#include "../src/scene.h"
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

static float GAME_STATE_TO_TIME[] = {4.0, 4.0, 0.0};

static GameState GAME_STATE;
static GameState NEXT_STATE;
static float TIME_REMAINING;

static Vector2 MOUSE_POSITION;
static bool IS_LMB_PRESSED;
static Ray MOUSE_RAY;

static Item* PICKED_ITEM;

static void update_game(void);
static void draw_imgui(void);

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Editor");
    SetTargetFPS(60);

    load_scene("resources/scenes/tmp.scn");
    load_imgui();

    GAME_STATE = PLAYER_IS_PICKING;
    TIME_REMAINING = GAME_STATE_TO_TIME[GAME_STATE];

    while (!WindowShouldClose()) {
        update_scene();
        update_game();

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode3D(SCENE.camera);
        draw_scene();
        EndMode3D();

        begin_imgui();
        draw_imgui();
        end_imgui();

        EndDrawing();
    }

    return 0;
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

        // Unpick item if lmb pressed
        if (IS_LMB_PRESSED && PICKED_ITEM) {
            PICKED_ITEM->state = ITEM_COLD;
            PICKED_ITEM = NULL;
        }

        // Handle mouse input and update item states
        for (size_t i = 0; i < SCENE.board.n_items; ++i) {
            Item* item = &SCENE.board.items[i];
            if (item->is_dead) continue;
            RayCollision collision = GetRayCollisionMesh(
                MOUSE_RAY, SCENE.board.item_mesh, item->matrix
            );

            bool is_hit = collision.hit;
            if (is_hit && IS_LMB_PRESSED) {
                if (PICKED_ITEM) PICKED_ITEM->state = ITEM_COLD;
                item->state = ITEM_ACTIVE;
                PICKED_ITEM = item;
            } else if (is_hit) {
                item->state = MAX(item->state, ITEM_HOT);
            } else if (item->state == ITEM_HOT) {
                item->state = ITEM_COLD;
            }
        }
    }

    if (GAME_STATE == GOLOVA_IS_EATING) {

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

        // Cool down all non-dying items when golova is eating
        for (size_t i = 0; i < SCENE.board.n_items; ++i) {
            Item* item = &SCENE.board.items[i];
            if (item->state != ITEM_DYING) item->state = ITEM_COLD;
        }
    }
}

static void draw_imgui(void) {
    ig_fix_window_top_left();
    if (igBegin("Debug info", NULL, GHOST_WINDOW_FLAGS)) {
        igText("GAME_STATE: %s", GAME_STATE_TO_NAME(GAME_STATE));
        igText("TIME_REMAINING: %.2f", TIME_REMAINING);
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
