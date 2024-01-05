#include <stdio.h>

#define RAYGIZMO_IMPLEMENTATION
#include "../src/camera.h"
#include "../src/picking.h"
#include "../src/resources.h"
#include "../src/scene.h"
#include "raygizmo.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_USE_GLFW
#define CIMGUI_USE_OPENGL3
#include "cimgui.h"
#include "cimgui_impl.h"
#include <GLFW/glfw3.h>

Camera3D EDITOR_CAMERA;
RenderTexture EDITOR_SCREEN;
RenderTexture GAME_SCREEN;
RGizmo GIZMO;

static int PICKED_ID = -1;
static Vector3 PICKED_POSITION;

static void begin_imgui(void) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    igNewFrame();
}

static void end_imgui(void) {
    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}

static void draw_scene(void) {
    int id = SCENE.golova.id;
    Model model = SCENE.models[id];
    Transform t = SCENE.transforms[id];
    rlPushMatrix();
    rlTranslatef(t.translation.x, t.translation.y, t.translation.z);
    DrawModel(model, Vector3Zero(), 1.0, WHITE);
    rlPopMatrix();
    

    // -------------------------------------------------------------------
    // Draw items
    // Matrix transform = SCENE.models[SCENE.ground.id].transform;
    // float sx = transform.m0;
    // float sz = transform.m10;
    // int item_idx = 0;
    // for (size_t i = 0; i < SCENE.ground.grid_size; ++i) {
    //     float z = (float)i / (SCENE.ground.grid_size - 1) - 0.5;

    //     for (size_t j = 0; j < SCENE.ground.grid_size; ++j, ++item_idx) {
    //         float x = (float)j / (SCENE.ground.grid_size - 1) - 0.5;

    //         rlPushMatrix();
    //         {
    //             rlScalef(0.7, 0.7, 0.7);
    //             rlTranslatef(x, 0.8, z);
    //             rlScalef(0.1, 1.0, 0.1);
    //             rlRotatef(90.0, 1.0, 0.0, 0.0);
    //             rlMultMatrixf(MatrixToFloat(transform));

    //             int atlas_grid_size[2] = {8, 8};
    //             SetShaderValue(
    //                 ITEMS_SHADER,
    //                 GetShaderLocation(ITEMS_SHADER, "atlas_grid_size"),
    //                 atlas_grid_size,
    //                 SHADER_UNIFORM_IVEC2
    //             );
    //             SetShaderValue(
    //                 ITEMS_SHADER,
    //                 GetShaderLocation(ITEMS_SHADER, "item_idx"),
    //                 &item_idx,
    //                 SHADER_UNIFORM_INT
    //             );

    //             DrawModel(SCENE.ground.item_model, Vector3Zero(), 1.0, WHITE);
    //         }
    //         rlPopMatrix();
    //     }
    // }
}

static void draw_editor(void) {
    rlDisableBackfaceCulling();

    BeginTextureMode(EDITOR_SCREEN);
    {
        ClearBackground(DARKGRAY);
        BeginMode3D(EDITOR_CAMERA);
        {
            draw_scene();
            rlSetLineWidth(1.0);
            DrawGrid(10.0, 5.0);

            rlSetLineWidth(2.0);
            DrawLine3D(
                (Vector3){-25.0f, 0.0f, 0.0f}, (Vector3){25.0f, 0.0f, 0.0f}, RED
            );
            DrawLine3D(
                (Vector3){0.0f, -25.0f, 0.0f},
                (Vector3){0.0f, 25.0f, 0.0f},
                GREEN
            );
            DrawLine3D(
                (Vector3){0.0f, 0.0f, -25.0f},
                (Vector3){0.0f, 0.0f, 25.0f},
                DARKBLUE
            );
        }
        EndMode3D();

        // Draw game camera sphere and ray of view
        BeginMode3D(EDITOR_CAMERA);
        {
            Vector3 v = Vector3Subtract(
                SCENE.camera.c3d.target, SCENE.camera.c3d.position
            );
            v = Vector3Scale(Vector3Normalize(v), 2.0f);
            Vector3 start = SCENE.camera.c3d.position;
            Vector3 end = Vector3Add(start, v);
            rlSetLineWidth(5.0);
            DrawLine3D(start, end, PINK);
        }
        EndMode3D();

        // Draw gizmo on the picked model
        if (PICKED_ID != -1) {
            rgizmo_draw(GIZMO, EDITOR_CAMERA, PICKED_POSITION);
        }

        // Draw imgui
        begin_imgui();
        {

            // Place next window at top left
            ImVec2 position = {0.0, igGetFrameHeightWithSpacing() + 4.0};
            igSetNextWindowPos(position, ImGuiCond_Always, (ImVec2){0.0, 0.0});
            igSetNextWindowSize((ImVec2){0.0, 0.0}, ImGuiCond_Always);

            // Draw inspector
            if (igBegin("Inspector", NULL, 0)) {
                // Draw camera inspector
                int node = igCollapsingHeader_TreeNodeFlags("Camera", 0);
                igPushItemWidth(150.0);
                igDragFloat("FOV", &SCENE.camera.c3d.fovy, 1.0, 10.0, 170.0, "%.1f", 0);
                igPopItemWidth();

                // Draw picked model transform inspector
                node = igCollapsingHeader_TreeNodeFlags("Transform", 0);
                if (PICKED_ID != -1) {
                    Matrix *t = &SCENE.models[PICKED_ID].transform;
    // float m0, m4, m8, m12;  // Matrix first row (4 components)
    // float m1, m5, m9, m13;  // Matrix second row (4 components)
    // float m2, m6, m10, m14; // Matrix third row (4 components)
    // float m3, m7, m11, m15; // Matrix fourth row (4 components)
                    float scale[3] = {t->m0, t->m5, t->m10};
                    igPushItemWidth(150.0);
                    igDragFloat3("Scale", scale, 0.1, 0.1, 100.0, "%.1f", 0);
                    igPopItemWidth();

                    t->m0 = scale[0];
                    t->m5 = scale[1];
                    t->m10 = scale[2];
                }

                // Transformation* transformation = &SCENE.transformations[SCENE.camera];
                // Vec2 pos = transformation->curr_position;
                // float orient = transformation->curr_orientation;

                // ig_drag_float2("pos.", (float*)&pos, -FLT_MAX, FLT_MAX, 0.05, 0);
                // ig_drag_float("orient.", &orient, -PI, PI, 0.05, 0);
                // update_position(SCENE.camera, pos);
                // update_orientation(SCENE.camera, orient);

                // ig_drag_float(
                //     "view width", &SCENE.camera_view_width, 0.0, 1000.0, 0.2, 0
                // );
            }
            igEnd();


        }
        end_imgui();
    }
    EndTextureMode();
}

static void draw_game(void) {
    rlEnableBackfaceCulling();

    BeginTextureMode(GAME_SCREEN);
    {
        ClearBackground(BLACK);
        BeginMode3D(SCENE.camera.c3d);
        {
            draw_scene();
            //
        }
        EndMode3D();
    }
    EndTextureMode();
}

static void blit_screens(void) {
    BeginDrawing();
    {
        ClearBackground(BLACK);
        Rectangle editor_rect = {
            0, 0, EDITOR_SCREEN.texture.width, -EDITOR_SCREEN.texture.height};
        DrawTextureRec(
            EDITOR_SCREEN.texture, editor_rect, (Vector2){0, 0}, WHITE
        );

        int x = GetScreenWidth() - GAME_SCREEN.texture.width;
        Rectangle game_rect = {
            0, 0, GAME_SCREEN.texture.width, -GAME_SCREEN.texture.height};
        DrawTextureRec(GAME_SCREEN.texture, game_rect, (Vector2){x, 0}, WHITE);
    }
    EndDrawing();
}

static void update_editor(void) {
    update_editor_camera(&EDITOR_CAMERA);

    int picked_id = PICKED_ID;
    if (IsMouseButtonReleased(0) && GIZMO.state == RGIZMO_STATE_COLD) {
        picked_id = pick_model_3d(
            EDITOR_CAMERA, SCENE.models, SCENE.n_entities, GetMousePosition()
        );
    }

    if (picked_id == -1 && GIZMO.state == RGIZMO_STATE_COLD) {
        PICKED_ID = -1;
    } else if (picked_id != -1) {
        PICKED_ID = picked_id;
    }

    if (PICKED_ID != -1) {
        Transform *t = &SCENE.transforms[PICKED_ID];
        rgizmo_update(&GIZMO, EDITOR_CAMERA, t->translation);
        t->translation = Vector3Add(t->translation, GIZMO.update.translation);

        if (PICKED_ID == SCENE.camera.id) {
            if (GIZMO.update.axis.x == 1.0) {
                CameraPitch(
                    &SCENE.camera.c3d, GIZMO.update.angle, false, false, false
                );
            }
            if (GIZMO.update.axis.y == 1.0) {
                CameraYaw(&SCENE.camera.c3d, GIZMO.update.angle, false);
            }
            SCENE.camera.c3d.position = Vector3Add(
                SCENE.camera.c3d.position, GIZMO.update.translation
            );
            SCENE.camera.c3d.target = Vector3Add(
                SCENE.camera.c3d.target, GIZMO.update.translation
            );
        }
    }

    // if (IsKeyPressed(KEY_S) && IsKeyDown(KEY_LEFT_CONTROL)) {
    //     save_scene("scene_0.gsc");
    // }
}

void load_imgui(void) {
    igCreateContext(NULL);
    GLFWwindow* window = (GLFWwindow*)GetWindowHandle();
    glfwGetWindowUserPointer(window);
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    igStyleColorsDark(NULL);
}

int main(void) {
    // const int screen_width = 2560;
    // const int screen_height = 1440;
    const int screen_width = 1024;
    const int screen_height = 768;

    // Window
    InitWindow(screen_width, screen_height, "scene_example");
    SetTargetFPS(60);

    // Editor Camera
    EDITOR_CAMERA.fovy = 45.0f;
    EDITOR_CAMERA.target = (Vector3){0.0f, 0.0f, 0.0f};
    EDITOR_CAMERA.position = (Vector3){5.0f, 5.0f, 5.0f};
    EDITOR_CAMERA.up = (Vector3){0.0f, 1.0f, 0.0f};
    EDITOR_CAMERA.projection = CAMERA_PERSPECTIVE;

    // Screens
    EDITOR_SCREEN = LoadRenderTexture(screen_width, screen_height);
    GAME_SCREEN = LoadRenderTexture(screen_width / 3, screen_height / 3);

    // Scene
    load_resources();
    // load_scene("scene_0.gsc");
    load_scene(NULL);
    load_picking();
    load_imgui();
    GIZMO = rgizmo_create();

    // Main loop
    while (!WindowShouldClose()) {
        update_editor();
        draw_editor();
        draw_game();
        blit_screens();
    }

    // Unload resources
    UnloadRenderTexture(EDITOR_SCREEN);
    UnloadRenderTexture(GAME_SCREEN);

    unload_resources();
    unload_picking();
    rgizmo_unload();

    CloseWindow();
}
