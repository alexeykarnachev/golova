#include "../src/cimgui_utils.h"
#include "../src/drawing.h"
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
    GOLOVA_IS_EATING = 1,
    SCENE_OVER = 2,
    GAME_OVER = 3,
} GameState;

typedef enum PauseState {
    NOT_PAUSED = 0,
    MAIN_PAUSE = 1,
    OPTIONS_PAUSE = 2,
} PauseState;

static RenderTexture2D SCREEN;
static Shader POSTFX_SHADER;

#define PAUSE_STATE_TO_NAME(state) \
    ((state == NOT_PAUSED)      ? "NOT_PAUSED" \
     : (state == MAIN_PAUSE)    ? "MAIN_PAUSE" \
     : (state == OPTIONS_PAUSE) ? "OPTIONS_PAUSE" \
                                : "UNKNOWN")

#define GAME_STATE_TO_NAME(state) \
    ((state == PLAYER_IS_PICKING)  ? "PLAYER_IS_PICKING" \
     : (state == GOLOVA_IS_EATING) ? "GOLOVA_IS_EATING" \
     : (state == SCENE_OVER)       ? "SCENE_OVER" \
     : (state == GAME_OVER)        ? "GAME_OVER" \
                                   : "UNKNOWN")

static float GAME_STATE_TO_TIME[] = {1.0, 1.0, 0.0};

static char* SCENES_DIR = "resources/scenes";
static int CURR_SCENE_ID;
static char** SCENE_FILE_NAMES;
static int N_SCENES;

static PauseState PAUSE_STATE;
static GameState GAME_STATE;
static GameState NEXT_STATE;
static float TIME_REMAINING;
static bool IS_NEXT_SCENE;
static bool IS_EXIT_GAME;

static Vector2 MOUSE_POSITION;
static bool IS_ESCAPE_PRESSED;
static bool IS_LMB_PRESSED;
static bool IS_ALTF4_PRESSED;
static Ray MOUSE_RAY;

static Item* PICKED_ITEM;

static void load_curr_scene(void);
static void update_game(void);
static void draw_postfx(void);
static void draw_ggui(void);
static void draw_imgui(void);
Rectangle get_text_rec(const char* text, int y, int font_size);

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Golova");
    SetTargetFPS(60);

    SCREEN = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    POSTFX_SHADER = LoadShader(0, "resources/shaders/postfx.frag");
    SCENE_FILE_NAMES = get_file_names_in_dir(SCENES_DIR, &N_SCENES);

    load_curr_scene();
    load_imgui();

    while (!IS_EXIT_GAME) {
        update_scene();
        update_game();

        // Draw main scene
        BeginTextureMode(SCREEN);
        ClearBackground(BLACK);

        BeginMode3D(SCENE.camera);
        draw_scene();
        EndMode3D();

        EndTextureMode();

        // Draw ui and postfx
        BeginDrawing();

        draw_postfx();
        draw_ggui();

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

    NEXT_STATE = PLAYER_IS_PICKING;
    TIME_REMAINING = GAME_STATE_TO_TIME[GAME_STATE];
}

static void update_game(void) {
    MOUSE_POSITION = GetMousePosition();
    IS_ESCAPE_PRESSED = IsKeyPressed(KEY_ESCAPE);
    IS_LMB_PRESSED = IsMouseButtonPressed(0);
    IS_ALTF4_PRESSED = IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_F4);
    MOUSE_RAY = GetMouseRay(MOUSE_POSITION, SCENE.camera);

    if ((WindowShouldClose() || IS_ALTF4_PRESSED) && !IS_ESCAPE_PRESSED) {
        IS_EXIT_GAME = true;
    }

    if (IS_ESCAPE_PRESSED) {
        if (PAUSE_STATE == NOT_PAUSED) PAUSE_STATE = MAIN_PAUSE;
        else PAUSE_STATE = NOT_PAUSED;
    }

    if (IS_NEXT_SCENE) {
        IS_NEXT_SCENE = false;
        CURR_SCENE_ID += 1;
        load_curr_scene();
    }

    if (NEXT_STATE != GAME_STATE) {
        GAME_STATE = NEXT_STATE;
        PAUSE_STATE = NOT_PAUSED;
        TIME_REMAINING = GAME_STATE_TO_TIME[GAME_STATE];
    }

    // -------------------------------------------------------------------
    // Update game state
    if (PAUSE_STATE > 0) return;
    TIME_REMAINING -= GetFrameTime();

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
                NEXT_STATE = CURR_SCENE_ID < N_SCENES - 1 ? SCENE_OVER : GAME_OVER;
            } else {
                NEXT_STATE = PLAYER_IS_PICKING;
            }
        }
    } else if (GAME_STATE == SCENE_OVER) {
    } else if (GAME_STATE == GAME_OVER) {
    }
}

static void draw_postfx(void) {
    BeginShaderMode(POSTFX_SHADER);
    int is_blured = PAUSE_STATE > 0 || GAME_STATE == SCENE_OVER
                    || GAME_STATE == GAME_OVER;
    SetShaderValue(
        POSTFX_SHADER,
        GetShaderLocation(POSTFX_SHADER, "u_is_blured"),
        &is_blured,
        SHADER_UNIFORM_INT
    );
    DrawTextureRec(
        SCREEN.texture,
        (Rectangle){0, 0, (float)SCREEN.texture.width, (float)-SCREEN.texture.height},
        (Vector2){0, 0},
        WHITE
    );
    EndShaderMode();
}

static void draw_ggui(void) {
    int screen_height = GetScreenHeight();
    int screen_width = GetScreenWidth();

    const char* text;
    Color color;
    Rectangle rec;

    if (PAUSE_STATE == MAIN_PAUSE) {
        int font_size = 60;

        const char* resume_text = "Resume";
        Rectangle resume_rec = get_text_rec(resume_text, 0, font_size);

        const char* options_text = "Options";
        Rectangle options_rec = get_text_rec(options_text, 0, font_size);

        const char* quit_text = "Quit";
        Rectangle quit_rec = get_text_rec(quit_text, 0, font_size);

        int gap = 10;
        int pad_x = 50;
        int pad_y = 100;

        int height = pad_y + resume_rec.height + gap + options_rec.height + gap
                     + quit_rec.height + pad_y;
        int width = pad_x + options_rec.width + pad_x;
        int x = (screen_width - width) / 2;
        int y = (screen_height - height) / 2;
        rec = (Rectangle){x, y, width, height};
        DrawRectangleRounded(rec, 0.2, 16, (Color){255, 255, 255, 180});

        x = (rec.width - resume_rec.width) / 2 + rec.x;
        y += pad_y;
        DrawText(resume_text, x, y, resume_rec.height, BLACK);
        y += resume_rec.height + gap;

        x = (rec.width - options_rec.width) / 2 + rec.x;
        DrawText(options_text, x, y, options_rec.height, BLACK);
        y += options_rec.height + gap;

        x = (rec.width - quit_rec.width) / 2 + rec.x;
        DrawText(quit_text, x, y, quit_rec.height, BLACK);
    } else if (GAME_STATE == SCENE_OVER || GAME_STATE == GAME_OVER) {
        // Main message
        if (SCENE.board.n_misses_allowed < 0) {
            text = "Golova feels bad...";
            color = MAROON;
        } else {
            text = "Golova is happy...";
            color = GREEN;
        }

        int y = screen_height / 2 - 100;
        rec = get_text_rec(text, y, 100);
        DrawText(text, rec.x, rec.y, rec.height, color);

        // Board rule
        text = SCENE.board.rule;
        rec = get_text_rec(text, y - 60, 40);
        DrawText(SCENE.board.rule, rec.x, rec.y, rec.height, RAYWHITE);

        if (GAME_STATE == SCENE_OVER) {
            text = "Continue";
            rec = get_text_rec(text, y + 150.0, 40);
            bool is_hit = CheckCollisionPointRec(MOUSE_POSITION, rec);
            color = is_hit ? RAYWHITE : LIGHTGRAY;
            DrawText(text, rec.x, rec.y, rec.height, color);
            IS_NEXT_SCENE = is_hit && IS_LMB_PRESSED;
        } else if (GAME_STATE == GAME_OVER) {
            text = "Game Over";
            rec = get_text_rec(text, y + 150.0, 40);
            DrawText(text, rec.x, rec.y, rec.height, RAYWHITE);
        }
    }
}

Rectangle get_text_rec(const char* text, int y, int font_size) {
    int screen_width = GetScreenWidth();
    int text_width = MeasureText(text, font_size);
    int x = (screen_width - text_width) / 2;
    Rectangle rec = {x, y, text_width, font_size};
    return rec;
}

static void draw_imgui(void) {
    ig_fix_window_top_left();
    if (igBegin("Debug info", NULL, GHOST_WINDOW_FLAGS)) {
        igText("scene_file_name: %s", SCENE_FILE_NAMES[CURR_SCENE_ID]);
        igText("GAME_STATE: %s", GAME_STATE_TO_NAME(GAME_STATE));
        igText("PAUSE_STATE: %s", PAUSE_STATE_TO_NAME(PAUSE_STATE));
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
