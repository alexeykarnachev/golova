#include "scene.h"

#include "raymath.h"
#include "utils.h"
#include <stdlib.h>

Scene SCENE;

void create_scene(void) {
    create_material(LoadMaterialDefault());
    create_mesh(GenMeshPlane(1.0, 1.0, 2, 2));
}

int create_screen(RenderTexture screen) {
    if (SCENE.resource.n_screens == MAX_N_SCREENS) {
        TraceLog(LOG_ERROR, "Can't create more screens");
        exit(1);
    }
    int id = SCENE.resource.n_screens++;
    SCENE.resource.screen[id] = screen;
    return id;
}

int create_material(Material material) {
    if (SCENE.resource.n_materials == MAX_N_MATERIALS) {
        TraceLog(LOG_ERROR, "Can't create more materials");
        exit(1);
    }
    int id = SCENE.resource.n_materials++;
    SCENE.resource.material[id] = material;
    return id;
}

int create_mesh(Mesh mesh) {
    if (SCENE.resource.n_meshes == MAX_N_MESHES) {
        TraceLog(LOG_ERROR, "Can't create more meshes");
        exit(1);
    }
    int id = SCENE.resource.n_meshes++;
    SCENE.resource.mesh[id] = mesh;
    return id;
}

int create_entity(void) {
    if (SCENE.entity.n_entities == MAX_N_ENTITIES) {
        TraceLog(LOG_ERROR, "Can't create more entities");
        exit(1);
    }
    int id = SCENE.entity.n_entities++;
    SCENE.entity.transform[id].scale = Vector3One();
    SCENE.entity.transform[id].rotation.w = 1.0;
    return id;
}

int create_camera(void) {
    if (SCENE.n_cameras == MAX_N_CAMERAS) {
        TraceLog(LOG_ERROR, "Can't create more cameras");
        exit(1);
    }
    int id = SCENE.n_cameras++;
    SCENE.camera[id].fovy = 60.0f;
    SCENE.camera[id].up = (Vector3){0.0f, 1.0f, 0.0f};
    SCENE.camera[id].projection = CAMERA_PERSPECTIVE;
    return id;
}

RenderTexture* get_screen(int screen_id) {
    CHECK_ID_INBOUND(screen_id, SCENE.resource.n_screens, screen);
    return &SCENE.resource.screen[screen_id];
}

Camera* get_camera(int camera_id) {
    CHECK_ID_INBOUND(camera_id, SCENE.n_cameras, camera);
    return &SCENE.camera[camera_id];
}

Mesh* get_mesh(int mesh_id) {
    CHECK_ID_INBOUND(mesh_id, SCENE.resource.n_meshes, mesh);
    return &SCENE.resource.mesh[mesh_id];
}

Material* get_material(int material_id) {
    CHECK_ID_INBOUND(material_id, SCENE.resource.n_materials, material);
    return &SCENE.resource.material[material_id];
}

Transform* get_entity_transform(int entity_id) {
    CHECK_ID_INBOUND(entity_id, SCENE.entity.n_entities, entity);
    return &SCENE.entity.transform[entity_id];
}

Mesh* get_entity_mesh(int entity_id) {
    CHECK_ID_INBOUND(entity_id, SCENE.entity.n_entities, entity);
    if (!check_if_entity_has_component(entity_id, MESH_COMPONENT)) {
        TraceLog(LOG_WARNING, "Entity %d doesn't have a mesh", entity_id);
        return &SCENE.resource.mesh[0];
    }

    return &SCENE.resource.mesh[SCENE.entity.mesh[entity_id]];
}

Material* get_entity_material(int entity_id) {
    CHECK_ID_INBOUND(entity_id, SCENE.entity.n_entities, entity);
    if (!check_if_entity_has_component(entity_id, MATERIAL_COMPONENT)) {
        TraceLog(LOG_WARNING, "Entity %d doesn't have a material", entity_id);
        return &SCENE.resource.material[0];
    }

    return &SCENE.resource.material[SCENE.entity.material[entity_id]];
}

void attach_entity_mesh(int entity_id, int mesh_id) {
    SCENE.entity.mesh[entity_id] = mesh_id;
    SCENE.entity.component[entity_id] |= MESH_COMPONENT;
}

void attach_entity_material(int entity_id, int material_id) {
    SCENE.entity.material[entity_id] = material_id;
    SCENE.entity.component[entity_id] |= MATERIAL_COMPONENT;
}

void set_entity_scale(int entity_id, Vector3 scale) {
    SCENE.entity.transform[entity_id].scale = scale;
}

void set_entity_scalef(int entity_id, float scale) {
    set_entity_scale(entity_id, (Vector3){scale, scale, scale});
}

void set_entity_component(int entity_id, Component component) {
    SCENE.entity.component[entity_id] |= component;
}

bool check_if_entity_has_component(int entity_id, Component component) {
    return (SCENE.entity.component[entity_id] & component) == component;
}

void unload_scene(void) {
    for (int i = 0; i < SCENE.resource.n_materials; ++i)
        UnloadMaterial(SCENE.resource.material[i]);
    for (int i = 0; i < SCENE.resource.n_meshes; ++i)
        UnloadMesh(SCENE.resource.mesh[i]);
    for (int i = 0; i < SCENE.resource.n_screens; ++i)
        UnloadRenderTexture(SCENE.resource.screen[i]);
}
