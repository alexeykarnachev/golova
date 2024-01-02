#include "raylib.h"
#include "rcamera.h"

#define RAYGIZMO_IMPLEMENTATION
#include "../src/camera.h"
#include "raygizmo.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_USE_GLFW
#define CIMGUI_USE_OPENGL3
#include "cimgui.h"
#include "cimgui_impl.h"
#include <GLFW/glfw3.h>

int main(void) {
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "cimgui_example");
    SetTargetFPS(60);

    Camera3D camera;
    camera.fovy = 45.0f;
    camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    camera.position = (Vector3){5.0f, 5.0f, 5.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.projection = CAMERA_PERSPECTIVE;

    Model model = LoadModelFromMesh(GenMeshTorus(0.3, 1.5, 16.0, 16.0));
    RGizmo gizmo = rgizmo_create();

    igCreateContext(NULL);
    GLFWwindow* window = (GLFWwindow*)GetWindowHandle();
    glfwGetWindowUserPointer(window);
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    igStyleColorsDark(NULL);

    while (!WindowShouldClose()) {
        update_editor_camera(&camera);

        BeginDrawing();
        {
            ClearBackground(DARKGRAY);
            rlEnableDepthTest();

            Vector3 position = {
                model.transform.m12, model.transform.m13, model.transform.m14};
            rgizmo_update(&gizmo, camera, position);
            model.transform = MatrixMultiply(
                model.transform, rgizmo_get_tranform(gizmo, position)
            );

            BeginMode3D(camera);
            {
                DrawModel(model, (Vector3){0.0, 0.0, 0.0}, 1.0, PURPLE);

                rlSetLineWidth(1.0);
                DrawGrid(100.0, 1.0);
                rlSetLineWidth(2.0);
                DrawLine3D(
                    (Vector3){-50.0f, 0.0f, 0.0f},
                    (Vector3){50.0f, 0.0f, 0.0f},
                    RED
                );
                DrawLine3D(
                    (Vector3){0.0f, -50.0f, 0.0f},
                    (Vector3){0.0f, 50.0f, 0.0f},
                    GREEN
                );
                DrawLine3D(
                    (Vector3){0.0f, 0.0f, -50.0f},
                    (Vector3){0.0f, 0.0f, 50.0f},
                    DARKBLUE
                );
            }
            EndMode3D();

            rgizmo_draw(gizmo, camera, position);

            DrawRectangle(0, 0, 280, 90, RAYWHITE);
            DrawText("CAMERA:", 5, 5, 20, RED);
            DrawText("    zoom: wheel", 5, 25, 20, RED);
            DrawText("    rotate: mmb", 5, 45, 20, RED);
            DrawText("    translate: shift + mmb", 5, 65, 20, RED);

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            igNewFrame();

            static float f = 0.0f;
            static int counter = 0;

            igBegin("Hello, world!", NULL, 0);
            igText("This is some useful text");

            igSliderFloat("Float", &f, 0.0f, 1.0f, "%.3f", 0);

            ImVec2 buttonSize;
            buttonSize.x = 0;
            buttonSize.y = 0;
            if (igButton("Button", buttonSize)) counter++;
            igSameLine(0.0f, -1.0f);
            igText("counter = %d", counter);

            igText(
                "Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / igGetIO()->Framerate,
                igGetIO()->Framerate
            );
            igEnd();

            igRender();
            ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
        }
        EndDrawing();
    }

    rgizmo_unload();
    UnloadModel(model);
    CloseWindow();

    return 0;
}
