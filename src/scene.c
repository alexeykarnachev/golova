#include "scene.h"

#include "raymath.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Scene SCENE;

static bool read_int(const char* data, int* dst, size_t* p) {
    *dst = data[*p];
    *p += sizeof(int);
    if (data[*p] != 0x1F) return false;
    *p += 1;
    return true;
}

static void preload_boards(char* file_path) {
    long n_bytes;
    char* data = read_cstr_file(file_path, "rb", &n_bytes);
    size_t p = 0;

    while (data[p] != 0x1C) {
        Board board = {0};

        if (!read_int(data, &board.n_misses_allowed, &p)) goto fail;
        if (!read_int(data, &board.n_hits_required, &p)) goto fail;
        if (!read_int(data, &board.n_correct_items, &p)) goto fail;

        // items
        while (data[p] != 0x1F) {
            Item item;
            item.is_alive = true;
            item.is_correct = board.n_items < board.n_correct_items;
            strcpy(item.name, &data[p]);

            board.items[board.n_items] = item;
            p += strlen(item.name) + 1;
            board.n_items += 1;
        }
        p += 1;

        // rule
        strcpy(board.rule, &data[p]);
        p += strlen(board.rule) + 1;
        if (data[p] != 0x1F) goto fail;
        p += 1;

        if (data[p] != 0x1E) goto fail;
        p += 1;

        board.board_scale = 0.7;
        board.item_scale = 0.1;
        board.item_elevation = 0.5;
        SCENE.board[SCENE.n_boards] = board;

        SCENE.n_boards += 1;
    }

    load_board(0);
    return;
fail:
    TraceLog(LOG_ERROR, "Failed to parse board %d", SCENE.n_boards);
    exit(1);
}

static void unload_board(void) {
    Board* board = &SCENE.board[SCENE.loaded_board_id];
    for (size_t i = 0; i < board->n_items; ++i) {
        Texture2D texture = board->items[i].texture;
        if (IsTextureReady(texture)) {
            UnloadTexture(texture);
        }
    }
}

void load_board(int board_id) {
    if (board_id == SCENE.loaded_board_id) {
        TraceLog(LOG_WARNING, "Board %d is already loaded", board_id);
    }

    CHECK_ID_INBOUND(board_id, SCENE.n_boards, board);

    unload_board();

    SCENE.loaded_board_id = board_id;
    Board* board = &SCENE.board[board_id];

    static char file_path[1024];
    for (size_t i = 0; i < board->n_items; ++i) {
        sprintf(
            file_path, "resources/items/sprites/%s.png", board->items[i].name
        );
        Image image = LoadImage(file_path);
        board->items[i].texture = LoadTextureFromImage(image);
        UnloadImage(image);
    }
}

void create_scene(void) {
    create_material(LoadMaterialDefault());
    create_mesh(GenMeshPlane(1.0, 1.0, 2, 2));
    preload_boards("resources/boards");
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

Board* get_loaded_board(void) {
    return &SCENE.board[SCENE.loaded_board_id];
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
    unload_board();

    for (int i = 0; i < SCENE.resource.n_materials; ++i)
        UnloadMaterial(SCENE.resource.material[i]);
    for (int i = 0; i < SCENE.resource.n_meshes; ++i)
        UnloadMesh(SCENE.resource.mesh[i]);
    for (int i = 0; i < SCENE.resource.n_screens; ++i)
        UnloadRenderTexture(SCENE.resource.screen[i]);
}
