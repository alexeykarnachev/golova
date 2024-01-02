#include "scene.h"

#include "raylib.h"
#include "resources.h"
#include <stdlib.h>

Scene SCENE;

static size_t add_model(Model model) {
    if (SCENE.n_models == SCENE_MAX_N_MODELS) {
        TraceLog(LOG_ERROR, "Can't add more models to the scene");
        exit(1);
    }

    size_t model_idx = SCENE.n_models++;
    SCENE.models[model_idx] = model;
    return model_idx;
}

void load_scene(void) {
    float aspect = (float)GOLOVA_TEXTURE.width / GOLOVA_TEXTURE.height;

    Model golova_model = LoadModelFromMesh(GenMeshPlane(aspect, 1.0, 2, 2));
    golova_model.materials[0].maps[0].texture = GOLOVA_TEXTURE;
    golova_model.materials[0].shader = SPRITE_SHADER;
    SCENE.golova.model_idx = add_model(golova_model);

    Model ground_model = LoadModelFromMesh(GenMeshPlane(10.0, 10.0, 2, 2));
    ground_model.materials[0].shader = GROUND_SHADER;
    SCENE.ground.model_idx = add_model(ground_model);

    SCENE.camera.fovy = 60.0f;
    SCENE.camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    SCENE.camera.position = (Vector3){0.0f, 5.0f, 5.0f};
    SCENE.camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    SCENE.camera.projection = CAMERA_PERSPECTIVE;
}
