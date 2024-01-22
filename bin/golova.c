#include "../src/math.h"
#include "../src/scene.h"
#include "../src/utils.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <math.h>
#include <stdio.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#if defined(PLATFORM_DESKTOP) && defined(DEBUG)
#define DRAW_IMGUI
#endif

#ifdef DRAW_IMGUI
#include "../src/cimgui_utils.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#endif

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
// #define SCREEN_WIDTH 2560
// #define SCREEN_HEIGHT 1440

#define EYES_SPEED 0.08

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

typedef enum Pivot {
    LEFT_TOP,
    CENTER_TOP,
    CENTER_CENTER,
    CENTER_BOT,
} Pivot;

typedef struct Position {
    int x;
    int y;
    Pivot pivot;
} Position;

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

static float GAME_STATE_TO_TIME[] = {10.0, 1.0, 0.0, 0.0};

static char *SCENES_DIR = "resources/scenes";
static int CURR_SCENE_ID;
static char **SCENE_FILE_NAMES;
static int N_SCENES;

static Texture2D TEXTURE_QUESTION_MARK;

static PauseState PAUSE_STATE;
static PauseState NEXT_PAUSE_STATE;
static GameState GAME_STATE;
static GameState NEXT_GAME_STATE;
static float TIME_REMAINING;
static bool IS_NEXT_SCENE;
static bool IS_EXIT_GAME;
static bool IS_BLURED;

static float DT;
static float TIME;
static Vector2 MOUSE_POSITION;
static bool IS_ESCAPE_PRESSED;
static bool IS_LMB_PRESSED;
static bool IS_ALTF4_PRESSED;
static Ray MOUSE_RAY;

static Item *PICKED_ITEM;

static int N_DEAD_CORRECT_ITEMS;
static Item *DEAD_CORRECT_ITEMS[MAX_N_BOARD_ITEMS];

static int N_DEAD_WRONG_ITEMS;
static Item *DEAD_WRONG_ITEMS[MAX_N_BOARD_ITEMS];

static float EYES_TARGET_SHIFT;
static float EYES_TARGET_UPLIFT;

static void load_curr_scene(void);
static void main_update(void);
static void update_game(void);
static void draw_ggui(void);
static void draw_imgui(void);

static bool ggui_button(Position pos, const char *text, int font_size);
static Rectangle ggui_get_rec(Position pos, int width, int height);
static void ggui_text(Position pos, const char *text, int font_size, Color color);

static void update_value(float dt, float speed, float *target, float *curr);
static void update_value2(
    float dt, float speed, float target_x, float target_y, float *curr_x, float *curr_y
);

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Golova");
    init_core(SCREEN_WIDTH, SCREEN_HEIGHT);
    SCENE_FILE_NAMES = get_file_names_in_dir(SCENES_DIR, &N_SCENES);

    TEXTURE_QUESTION_MARK = LoadTexture("resources/sprites/question.png");

#ifdef DRAW_IMGUI
    load_imgui();
#endif

    load_curr_scene();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(main_update, 0, 1);
#else
    SetTargetFPS(60);
    while (!IS_EXIT_GAME) {
        main_update();
    }
#endif

    return 0;
}

static void main_update(void) {
    update_game();

    draw_scene(true);

    // Draw postfx and ui
    BeginDrawing();
    draw_postfx(IS_BLURED);
    draw_ggui();
    draw_imgui();
    EndDrawing();
}

static void load_curr_scene(void) {
    static char fp[2048];
    sprintf(fp, "%s/%s", SCENES_DIR, SCENE_FILE_NAMES[CURR_SCENE_ID]);

    load_scene(fp);

    N_DEAD_CORRECT_ITEMS = 0;
    N_DEAD_WRONG_ITEMS = 0;
    NEXT_GAME_STATE = PLAYER_IS_PICKING;
    TIME_REMAINING = GAME_STATE_TO_TIME[GAME_STATE];

    for (int i = 0; i < SCENE.board.n_hint_items; ++i) {
        DEAD_CORRECT_ITEMS[N_DEAD_CORRECT_ITEMS++] = &SCENE.board.hint_items[i];
    }
}

static void update_game(void) {
    DT = GetFrameTime();
    TIME = GetTime();
    MOUSE_POSITION = GetMousePosition();
    IS_ESCAPE_PRESSED = IsKeyPressed(KEY_ESCAPE);
    IS_LMB_PRESSED = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    IS_ALTF4_PRESSED = IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_F4);
    MOUSE_RAY = GetMouseRay(MOUSE_POSITION, SCENE.camera);

#if !defined(PLATFORM_WEB)
    if ((WindowShouldClose() || IS_ALTF4_PRESSED) && !IS_ESCAPE_PRESSED) {
        IS_EXIT_GAME = true;
    }
#endif

    if (IS_ESCAPE_PRESSED) {
        if (PAUSE_STATE == NOT_PAUSED) NEXT_PAUSE_STATE = MAIN_PAUSE;
        else NEXT_PAUSE_STATE = NOT_PAUSED;
    }

    if (IS_NEXT_SCENE) {
        IS_NEXT_SCENE = false;
        CURR_SCENE_ID += 1;
        load_curr_scene();
    }

    if (NEXT_GAME_STATE != GAME_STATE) {
        GAME_STATE = NEXT_GAME_STATE;
        NEXT_PAUSE_STATE = NOT_PAUSED;
        TIME_REMAINING = GAME_STATE_TO_TIME[GAME_STATE];
    }

    PAUSE_STATE = NEXT_PAUSE_STATE;
    IS_BLURED = PAUSE_STATE > 0 || GAME_STATE == SCENE_OVER || GAME_STATE == GAME_OVER;

    // -------------------------------------------------------------------
    // Update board items
    Board *b = &SCENE.board;
    Transform t = b->transform;
    t.scale = Vector3Scale(Vector3One(), t.scale.x);
    Matrix board_matrix = get_transform_matrix(t);
    float *board_matrix_f = MatrixToFloat(board_matrix);

    int n_rows = sqrt(b->n_items);
    int n_cols = ceil((float)b->n_items / n_rows);
    for (size_t i = 0; i < b->n_items; ++i) {
        Item *item = &b->items[i];

        int i_row = i / n_cols;
        int i_col = i % n_cols;

        float z = n_rows > 1 ? (float)i_row / (n_rows - 1) - 0.5 : 0.0;
        float x = n_cols > 1 ? (float)i_col / (n_cols - 1) - 0.5 : 0.0;

        rlPushMatrix();
        {
            rlMultMatrixf(board_matrix_f);
            rlScalef(b->board_scale, b->board_scale, b->board_scale);
            rlTranslatef(x, 0.0, z);
            rlScalef(b->item_scale, b->item_scale, b->item_scale);
            float elevation_offset = 0.05 * (sinf(2.0 * TIME) + 1.0);
            rlTranslatef(0.0, b->item_elevation + elevation_offset, 0.0);

            // Rotate dying item
            if (item->state == ITEM_DYING) rlRotatef(TIME * 360.0, 0.0, 0.0, 1.0);

            rlRotatef(90.0, 1.0, 0.0, 0.0);
            item->matrix = rlGetMatrixTransform();
        }
        rlPopMatrix();
    }

    // -------------------------------------------------------------------
    // Update trees
    for (size_t i = 0; i < SCENE.forest.n_trees; ++i) {
        Tree *tree = &SCENE.forest.trees[i];
        rlPushMatrix();
        rlTranslatef(
            tree->transform.translation.x,
            tree->transform.translation.y,
            tree->transform.translation.z
        );
        float a = sinf(TIME) * 2.5;
        rlRotatef(a, 1.0, 0.0, 0.0);
        rlRotatef(a, 0.0, 1.0, 0.0);
        rlRotatef(a, 0.0, 0.0, 1.0);
        rlTranslatef(
            -tree->transform.translation.x,
            -tree->transform.translation.y,
            -tree->transform.translation.z
        );
        tree->matrix = rlGetMatrixTransform();
        rlPopMatrix();
    }

    // -------------------------------------------------------------------
    // Update Golova
    int health = CLAMP(SCENE.board.n_misses_allowed, 0, 3);
    SCENE.golova.cracks.strength = (3 - health) / 3.0f;

    rlPushMatrix();
    rlTranslatef(0.0, sinf(TIME * 2.0) * 0.015, 0.0);
    SCENE.golova.matrix = rlGetMatrixTransform();
    rlPopMatrix();

    // -------------------------------------------------------------------
    // Update Golova gaze
    Vector3 target;
    bool has_target = false;

    // Look at hot, active or dying item
    for (size_t i = 0; i < SCENE.board.n_items; ++i) {
        Item *item = &SCENE.board.items[i];
        if (item->state > ITEM_COLD && item->state < ITEM_DEAD) {
            Matrix mat = MatrixMultiply(
                get_transform_matrix(SCENE.board.transform), item->matrix
            );
            Vector3 pos = (Vector3){mat.m12, mat.m13, mat.m14};
            target = pos;
            has_target = true;

            // Don't check other items, we already look at the picked (active) item
            if (item == PICKED_ITEM) break;
        }
    }

    // If there is no target item, just follow the mouse cursor (board collision)
    if (!has_target) {
        RayCollision collision = GetRayCollisionMesh(
            MOUSE_RAY, SCENE.board.mesh, get_transform_matrix(SCENE.board.transform)
        );
        has_target = collision.hit;
        target = collision.point;
    }

    if (has_target) {
        float golova_x = SCENE.golova.transform.translation.x;
        float golova_z = SCENE.golova.transform.translation.z;
        EYES_TARGET_SHIFT = SCENE.golova.eyes_idle_shift + 0.075 * (target.x - golova_x);
        EYES_TARGET_UPLIFT = SCENE.golova.eyes_idle_uplift - 0.01 * (target.z - golova_z);
    } else {
        EYES_TARGET_SHIFT = SCENE.golova.eyes_idle_shift;
        EYES_TARGET_UPLIFT = SCENE.golova.eyes_idle_uplift;
    }

    // Apply Golova gaze
    update_value2(
        DT,
        EYES_SPEED,
        EYES_TARGET_SHIFT,
        EYES_TARGET_UPLIFT,
        &SCENE.golova.eyes_curr_shift,
        &SCENE.golova.eyes_curr_uplift
    );

    // -------------------------------------------------------------------
    // Update game state
    if (PAUSE_STATE > 0) return;
    TIME_REMAINING -= GetFrameTime();

    if (GAME_STATE == PLAYER_IS_PICKING) {
        // Set up Golova state
        SCENE.golova.state = GOLOVA_IDLE;

        // Handle mouse input and update item states
        bool is_hit_any = false;
        for (size_t i = 0; i < SCENE.board.n_items; ++i) {
            Item *item = &SCENE.board.items[i];

            // Don't update dead items
            if (item->state == ITEM_DEAD) continue;

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
            NEXT_GAME_STATE = GOLOVA_IS_EATING;
            if (!PICKED_ITEM) {
                for (size_t i = 0; i < SCENE.board.n_items; ++i) {
                    Item *item = &SCENE.board.items[i];
                    if (!(item->state == ITEM_DEAD) && !item->is_correct) {
                        PICKED_ITEM = item;
                    }
                }
            }

            PICKED_ITEM->state = ITEM_DYING;
        }
    } else if (GAME_STATE == GOLOVA_IS_EATING) {
        // Set up Golova state
        SCENE.golova.state = GOLOVA_EAT;

        // Cool down all non-dying items when golova is eating
        for (size_t i = 0; i < SCENE.board.n_items; ++i) {
            Item *item = &SCENE.board.items[i];
            if (item->state < ITEM_DYING) item->state = ITEM_COLD;
        }

        // GOLOVA_IS_EATING state is over
        if (TIME_REMAINING <= 0.0) {
            PICKED_ITEM->state = ITEM_DEAD;
            if (PICKED_ITEM->is_correct) {
                DEAD_CORRECT_ITEMS[N_DEAD_CORRECT_ITEMS++] = PICKED_ITEM;
                SCENE.board.n_hits_required -= 1;
            } else {
                DEAD_WRONG_ITEMS[N_DEAD_WRONG_ITEMS++] = PICKED_ITEM;
                SCENE.board.n_misses_allowed -= 1;
            }
            PICKED_ITEM = NULL;

            if (SCENE.board.n_misses_allowed < 0 || SCENE.board.n_hits_required == 0) {
                NEXT_GAME_STATE = CURR_SCENE_ID < N_SCENES - 1 ? SCENE_OVER : GAME_OVER;
            } else {
                NEXT_GAME_STATE = PLAYER_IS_PICKING;
            }
        }
    } else if (GAME_STATE == SCENE_OVER) {
    } else if (GAME_STATE == GAME_OVER) {
    }
}

static void draw_ggui(void) {
    int screen_height = GetScreenHeight();
    int screen_width = GetScreenWidth();
    int cx = screen_width / 2;
    int cy = screen_height / 2;

    if (PAUSE_STATE == MAIN_PAUSE) {
        int font_size = 60;
        int gap = 20;
        int pad = 50;

        const char *resume_text = "Resume";
        const char *options_text = "Options";
        const char *quit_text = "Quit";

        int width = MeasureText(options_text, font_size) * 1.2;
        int height = font_size * 3 + gap * 2 + pad * 2;

        Rectangle main_rec = ggui_get_rec(
            (Position){cx, cy, CENTER_CENTER}, width, height
        );
        DrawRectangleRounded(main_rec, 0.2, 16, (Color){100, 100, 100, 150});
        DrawRectangleRoundedLines(main_rec, 0.2, 16, 4, WHITE);

        int y = main_rec.y + pad;
        if (ggui_button((Position){cx, y, CENTER_TOP}, resume_text, font_size)) {
            NEXT_PAUSE_STATE = NOT_PAUSED;
        }

        y += font_size + gap;
        ggui_button((Position){cx, y, CENTER_TOP}, options_text, font_size);

        y += font_size + gap;
        IS_EXIT_GAME = ggui_button((Position){cx, y, CENTER_TOP}, quit_text, font_size);
    } else if (GAME_STATE == SCENE_OVER || GAME_STATE == GAME_OVER) {
        const char *text;
        Color color;
        Position pos;

        if (SCENE.board.n_misses_allowed < 0) {
            text = "Golova feels bad...";
            color = MAROON;
        } else {
            text = "Golova is happy...";
            color = GREEN;
        }

        int font_size = 100;

        pos = (Position){cx, cy, CENTER_CENTER};
        ggui_text(pos, text, font_size, color);

        pos = (Position){cx, cy - font_size / 2, CENTER_BOT};
        ggui_text(pos, SCENE.board.rule, font_size / 3, LIGHTGRAY);

        pos = (Position){cx, cy + font_size, CENTER_TOP};
        if (GAME_STATE == SCENE_OVER) {
            IS_NEXT_SCENE = ggui_button(pos, "Continue", font_size / 2);
        } else if (GAME_STATE == GAME_OVER) {
            ggui_text(pos, "Game Over", font_size / 2, LIGHTGRAY);
        }
    }

    // -------------------------------------------------------------------
    // Draw correctly picked items
    int n_items = N_DEAD_CORRECT_ITEMS + SCENE.board.n_hits_required;
    int item_size = 64;
    int pad = 20;
    int mid_x = screen_width / 2;
    int x = mid_x - (item_size * n_items + pad * (n_items - 1)) / 2;

    for (int i = 0; i < n_items; ++i) {
        Texture texture;
        Color color = WHITE;
        if (i < N_DEAD_CORRECT_ITEMS) {
            Item *item = DEAD_CORRECT_ITEMS[i];
            texture = item->texture;
        } else {
            texture = TEXTURE_QUESTION_MARK;
            color = PURPLE;
        }
        Rectangle src = {0.0, 0.0, texture.width, texture.height};
        Rectangle dst = {x, pad, item_size, item_size};
        DrawTexturePro(texture, src, dst, Vector2Zero(), 0.0, color);

        x += pad + item_size;
    }
}

static Rectangle ggui_get_rec(Position pos, int width, int height) {
    int x = pos.x;
    int y = pos.y;

    if (pos.pivot == CENTER_TOP) {
        x -= width / 2;
    } else if (pos.pivot == CENTER_CENTER) {
        x -= width / 2;
        y -= height / 2;
    } else if (pos.pivot == CENTER_BOT) {
        x -= width / 2;
        y -= height;
    }

    Rectangle rec = {x, y, width, height};
    return rec;
}

static bool ggui_button(Position pos, const char *text, int font_size) {
    int width = MeasureText(text, font_size);
    Rectangle rec = ggui_get_rec(pos, width, font_size);

    bool is_hit = CheckCollisionPointRec(MOUSE_POSITION, rec);
    Color color = is_hit ? WHITE : LIGHTGRAY;
    DrawText(text, rec.x, rec.y, font_size, color);

    return is_hit && IS_LMB_PRESSED;
}

static void ggui_text(Position pos, const char *text, int font_size, Color color) {
    int width = MeasureText(text, font_size);
    Rectangle rec = ggui_get_rec(pos, width, font_size);
    DrawText(text, rec.x, rec.y, font_size, color);
}

static void update_value(float dt, float speed, float *target, float *curr) {
    float todo = *target - *curr;
    float step = speed * dt;
    if (step > fabs(todo)) *curr = *target;
    else if (todo > 0.0) *curr += step;
    else *curr -= step;
}

static void update_value2(
    float dt, float speed, float target_x, float target_y, float *curr_x, float *curr_y
) {
    Vector2 d = {target_x - *curr_x, target_y - *curr_y};
    float len = Vector2Length(d);
    float step = speed * dt;
    if (step > len) {
        *curr_x = target_x;
        *curr_y = target_y;
    } else {
        d = Vector2Scale(Vector2Normalize(d), step);
        *curr_x += d.x;
        *curr_y += d.y;
    }
}

#ifdef DRAW_IMGUI
static void draw_imgui(void) {
    begin_imgui();
    ig_fix_window_top_left();
    if (igBegin("Debug info", NULL, GHOST_WINDOW_FLAGS)) {
        igText("scene_file_name: %s", SCENE_FILE_NAMES[CURR_SCENE_ID]);
        igText("FPS: %d", GetFPS());
        igText("GAME_STATE: %s", GAME_STATE_TO_NAME(GAME_STATE));
        igText("PAUSE_STATE: %s", PAUSE_STATE_TO_NAME(PAUSE_STATE));
        igText("TIME_REMAINING: %.2f", TIME_REMAINING);
        igText("rule: %s", SCENE.board.rule);
        igText("n_hits_required: %d", SCENE.board.n_hits_required);
        igText("n_misses_allowed: %d", SCENE.board.n_misses_allowed);

        const char *picked_item_name = "";
        const char *picked_item_state = "";
        if (PICKED_ITEM) {
            picked_item_name = PICKED_ITEM->name;
            picked_item_state = ITEM_STATE_TO_NAME(PICKED_ITEM->state);
        }
        igText("picked_item_name: %s", picked_item_name);
        igText("picked_item_state: %s", picked_item_state);
    }
    igEnd();
    end_imgui();
}
#else
static void draw_imgui(void) {}
#endif
