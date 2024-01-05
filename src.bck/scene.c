#include "scene.h"

#include "raylib.h"
#include "raymath.h"
#include "resources.h"
#include <stdlib.h>

Scene SCENE;

static size_t add_entity(void) {
    if (SCENE.n_entities == SCENE_MAX_N_ENTITIES) {
        TraceLog(LOG_ERROR, "Can't add more entities to the scene");
        exit(1);
    }
    return SCENE.n_entities++;
}

void load_scene(const char* file_path) {
    float aspect = (float)GOLOVA_TEXTURE.width / GOLOVA_TEXTURE.height;

    int id;

    // -------------------------------------------------------------------
    // Load Golova
    id = add_entity();
    SCENE.golova.id = id;
    SCENE.models[id] = LoadModelFromMesh(GenMeshPlane(aspect, 1.0, 2, 2));
    SCENE.models[id].transform = MatrixIdentity();
    SCENE.models[id].materials[0].maps[0].texture = GOLOVA_TEXTURE;
    SCENE.models[id].materials[0].shader = SPRITE_SHADER;
    SCENE.transforms[id].scale = Vector3One();

    // -------------------------------------------------------------------
    // Load ground
    id = add_entity();
    SCENE.ground.id = id;
    SCENE.ground.grid_size = 5;
    SCENE.models[id] = LoadModelFromMesh(GenMeshPlane(aspect, 1.0, 2, 2));
    SCENE.models[id].transform = MatrixIdentity();
    SCENE.models[id].materials[0].shader = GROUND_SHADER;
    SCENE.transforms[id].scale = Vector3One();

    // Load items
    Model item_model = LoadModelFromMesh(GenMeshPlane(1.0, 1.0, 2, 2));
    item_model.materials[0].maps[0].texture = ITEMS_TEXTURE;
    item_model.materials[0].shader = ITEMS_SHADER;
    SCENE.ground.item_model = item_model;

    // -------------------------------------------------------------------
    // Load camera
    id = add_entity();
    SCENE.camera.id = id;
    Vector3 camera_position = (Vector3){0.0f, 2.5f, 2.5f};
    SCENE.camera.c3d.fovy = 60.0f;
    SCENE.camera.c3d.target = (Vector3){0.0f, 0.0f, -1.0f};
    SCENE.camera.c3d.position = camera_position;
    SCENE.camera.c3d.up = (Vector3){0.0f, 1.0f, 0.0f};
    SCENE.camera.c3d.projection = CAMERA_PERSPECTIVE;
    SCENE.models[id] = LoadModelFromMesh(GenMeshSphere(0.15, 16, 16));
    SCENE.models[id].transform = MatrixTranslate(
        camera_position.x, camera_position.y, camera_position.z
    );

    // -------------------------------------------------------------------
    // Load data from save file
    // if (file_path != NULL) {
    //     int data_size;
    //     SceneSaveData data = *(SceneSaveData*)LoadFileData(
    //         file_path, &data_size
    //     );
    //     SCENE.models[SCENE.golova.model_id].transform = data.golova_transform;
    //     SCENE.models[SCENE.ground.model_id].transform = data.ground_transform;
    //     SCENE.camera.c3d = data.c3d;

    //     TraceLog(LOG_INFO, "GOLOVA: Scene loaded from %s", file_path);
    // }
}

void save_scene(const char* file_path) {
    // SceneSaveData data = {0};
    // data.golova_transform = SCENE.models[SCENE.golova.model_id].transform;
    // data.ground_transform = SCENE.models[SCENE.ground.model_id].transform;
    // data.c3d = SCENE.camera.c3d;
    // SaveFileData(file_path, &data, sizeof(SceneSaveData));

    // TraceLog(LOG_INFO, "GOLOVA: Scene saved in %s", file_path);
}
