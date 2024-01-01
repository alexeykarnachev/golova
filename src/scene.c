#include "scene.h"

#include "resources.h"

Scene SCENE;

void load_scene(void) {
    float aspect = (float)GOLOVA_TEXTURE.width / GOLOVA_TEXTURE.height;

    Golova golova;
    golova.model = LoadModelFromMesh(GenMeshPlane(aspect, 1.0, 2, 2));
    golova.model.materials[0].maps[0].texture = GOLOVA_TEXTURE;
    golova.model.materials[0].shader = SPRITE_SHADER;

    Ground ground;
    ground.model = LoadModelFromMesh(GenMeshPlane(10.0, 10.0, 2, 2));
    ground.model.materials[0].shader = GROUND_SHADER;

    SCENE.golova = golova;
    SCENE.ground = ground;
}
