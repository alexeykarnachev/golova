#include "picking.h"

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <stdio.h>
#include <stdlib.h>

#define PICKING_FBO_WIDTH 500
#define PICKING_FBO_HEIGHT 500

static bool IS_PICKING_LOADED = false;
static unsigned int PICKING_FBO;
static unsigned int PICKING_TEXTURE;
static unsigned int PICKING_DEPTH_TEXTURE;
static Material PICKING_MATERIAL;

static int PICKING_SHADER_ID_LOC;
static const char* PICKING_SHADER_FRAG = "\
#version 330\n\
uniform int u_id; \
out vec4 finalColor; \
void main() { \
    finalColor = vec4(float(u_id)/255.0, 0, 0, 0); \
} \
";

static int pick_model(Model* models, size_t n_models, Vector2 mouse_position) {
    if (!IS_PICKING_LOADED) {
        TraceLog(LOG_ERROR, "GOLOVA: Picking is not loaded");
        exit(1);
    }

    rlEnableFramebuffer(PICKING_FBO);
    rlViewport(0, 0, PICKING_FBO_WIDTH, PICKING_FBO_HEIGHT);
    rlClearScreenBuffers();
    rlDisableColorBlend();

    for (size_t i = 0; i < n_models; ++i) {
        int id_plus_one = i + 1;
        Model* model = &models[i];

        Material material = model->materials[0];
        model->materials[0] = PICKING_MATERIAL;
        for (size_t mesh_id = 0; mesh_id < model->meshCount; ++mesh_id) {
            SetModelMeshMaterial(model, mesh_id, 0);
        }

        SetShaderValue(
            PICKING_MATERIAL.shader,
            PICKING_SHADER_ID_LOC,
            &id_plus_one,
            SHADER_UNIFORM_INT
        );
        DrawModel(*model, (Vector3){0.0, 0.0, 0.0}, 1.0, BLANK);

        model->materials[0] = material;
    }

    rlDisableFramebuffer();

    // -------------------------------------------------------------------
    // Pick the pixel under the mouse cursor
    unsigned char* pixels = (unsigned char*)rlReadTexturePixels(
        PICKING_TEXTURE,
        PICKING_FBO_WIDTH,
        PICKING_FBO_HEIGHT,
        RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    );

    float screen_width = (float)GetScreenWidth();
    float screen_height = (float)GetScreenHeight();

    if (screen_width < 1.0 || screen_height < 1.0) return 0;

    float x_fract = Clamp(mouse_position.x / screen_width, 0.0, 1.0);
    float y_fract = Clamp(1.0 - (mouse_position.y / screen_height), 0.0, 1.0);
    int x = (int)(PICKING_FBO_WIDTH * x_fract);
    int y = (int)(PICKING_FBO_HEIGHT * y_fract);
    int idx = 4 * (y * PICKING_FBO_WIDTH + x);
    int id_plus_one = pixels[idx];
    int id = id_plus_one - 1;

    free(pixels);
    return id;
}

int pick_model_3d(
    Camera3D camera, Model* models, size_t n_models, Vector2 mouse_position
) {
    int id = -1;
    BeginDrawing();
    {
        BeginMode3D(camera);
        { id = pick_model(models, n_models, mouse_position); }
        EndMode3D();
    }
    EndDrawing();

    return id;
}

void load_picking(void) {
    if (IS_PICKING_LOADED) {
        TraceLog(LOG_WARNING, "GOLOVA: Picking is already loaded");
        return;
    };

    PICKING_FBO = rlLoadFramebuffer(PICKING_FBO_WIDTH, PICKING_FBO_HEIGHT);
    if (!PICKING_FBO) {
        TraceLog(LOG_ERROR, "GOLOVA: Failed to create picking fbo");
        exit(1);
    }
    rlEnableFramebuffer(PICKING_FBO);

    PICKING_TEXTURE = rlLoadTexture(
        NULL,
        PICKING_FBO_WIDTH,
        PICKING_FBO_HEIGHT,
        RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        1
    );
    rlFramebufferAttach(
        PICKING_FBO,
        PICKING_TEXTURE,
        RL_ATTACHMENT_COLOR_CHANNEL0,
        RL_ATTACHMENT_TEXTURE2D,
        0
    );
    rlActiveDrawBuffers(1);

    PICKING_DEPTH_TEXTURE = rlLoadTextureDepth(
        PICKING_FBO_WIDTH, PICKING_FBO_HEIGHT, true
    );
    rlFramebufferAttach(
        PICKING_FBO,
        PICKING_DEPTH_TEXTURE,
        RL_ATTACHMENT_DEPTH,
        RL_ATTACHMENT_RENDERBUFFER,
        0
    );

    if (!rlFramebufferComplete(PICKING_FBO)) {
        TraceLog(LOG_ERROR, "GOLOVA: Picking fbo is not complete");
        exit(1);
    }

    Shader shader = LoadShaderFromMemory(0, PICKING_SHADER_FRAG);
    if (!IsShaderReady(shader)) {
        TraceLog(LOG_ERROR, "GOLOVA: Failed to load picking shader");
        exit(1);
    }
    PICKING_SHADER_ID_LOC = GetShaderLocation(shader, "u_id");

    PICKING_MATERIAL = LoadMaterialDefault();
    PICKING_MATERIAL.shader = shader;

    IS_PICKING_LOADED = true;
}

void unload_picking(void) {
    if (!IS_PICKING_LOADED) {
        TraceLog(LOG_WARNING, "GOLOVA: Picking is not loaded");
        return;
    };

    rlUnloadTexture(PICKING_TEXTURE);
    rlUnloadFramebuffer(PICKING_FBO);
    UnloadMaterial(PICKING_MATERIAL);
    IS_PICKING_LOADED = false;
}
