#include "raylib.h"
#include <rlgl.h>
#include <stddef.h>
#include <stdio.h>

int main(void) {

    const int screen_width = 800;
    const int screen_height = 600;
    InitWindow(screen_width, screen_height, "Golova");
    rlEnableDepthTest();
    SetTargetFPS(60);

    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, -1.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 70.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Texture texture = LoadTexture("resources/golova.png");
    float texture_aspect = (float)texture.width / texture.height;

    Shader golova_shader = LoadShader(0, "resources/shaders/sprite.frag");
    Shader ground_shader = LoadShader(0, "resources/shaders/ground.frag");

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(BLACK);

        BeginMode3D(camera);

            // -----------------------------------------------------------
            // Draw Golova:
            BeginShaderMode(golova_shader);
            rlSetTexture(texture.id);
            rlPushMatrix();
                rlTranslatef(0.0f, 1.0f, -3.0f);
                rlRotatef(180.0f, 0.0f, 0.0f, 1.0f);
                rlScalef(texture_aspect, 1.0f, 1.0f);

                rlBegin(RL_QUADS);
                    rlColor4ub(255, 255, 255, 255);
                    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(-1.0, -1.0, 0.0);  // BL
                    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(1.0, -1.0, 0.0);  // BR
                    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(1.0, 1.0, 0.0);  // TR
                    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(-1.0, 1.0, 0.0);  // TL
                rlEnd();

            rlPopMatrix();
            rlSetTexture(0);
            EndShaderMode();

            // -----------------------------------------------------------
            // Draw ground:
            BeginShaderMode(ground_shader);
            rlPushMatrix();
                rlTranslatef(0.0f, -5.0f, 0.0f);
                rlRotatef(-45.0f, 1.0f, 0.0f, 0.0f);
                rlScalef(5.0, 6.0f, 1.0f);
                rlBegin(RL_QUADS);
                    rlColor4ub(255, 0, 0, 255);
                    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(-1.0, -1.0, 0.0);  // BL
                    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(1.0, -1.0, 0.0);  // BR
                    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(1.0, 1.0, 0.0);  // TR
                    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(-1.0, 1.0, 0.0);  // TL
                rlEnd();
            rlPopMatrix();
            EndShaderMode();
        
        EndMode3D();

        DrawText("TEST", 640, 10, 20, RED);

        EndDrawing();
    }

    UnloadTexture(texture);
    UnloadShader(golova_shader);
    UnloadShader(ground_shader);
    CloseWindow();

    return 0;
}
