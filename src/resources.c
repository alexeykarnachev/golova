#include "resources.h"

#include "raylib.h"
#include <stddef.h>

static size_t N_MATERIALS;
static size_t N_MESHES;

Material MATERIALS[MAX_N_MATERILAS];
Mesh MESHES[MAX_N_MESHES];

Material* DEFAULT_MATERIAL;

Mesh* DEFAULT_PLANE_MESH;
Mesh* DEFAULT_SPHERE_MESH;
Mesh* DEFAULT_CUBE_MESH;

static Mesh* new_mesh_ptr(void) {
    return &MESHES[N_MESHES++];
}

static Material* new_material_ptr(void) {
    return &MATERIALS[N_MATERIALS++];
}

void load_resources(void) {
    // Default resources
    DEFAULT_MATERIAL = new_material_ptr();
    *DEFAULT_MATERIAL = LoadMaterialDefault();

    DEFAULT_PLANE_MESH = new_mesh_ptr();
    *DEFAULT_PLANE_MESH = GenMeshPlane(1.0, 1.0, 2, 2);

    DEFAULT_SPHERE_MESH = new_mesh_ptr();
    *DEFAULT_SPHERE_MESH = GenMeshSphere(1.0, 16, 16);

    DEFAULT_CUBE_MESH = new_mesh_ptr();
    *DEFAULT_CUBE_MESH = GenMeshCube(1.0, 1.0, 1.0);
}

Material* load_material(void) {
    Material* material = new_material_ptr();
    *material = LoadMaterialDefault();

    return material;
}

Material* load_sprite_material(const char* texture_file_path) {
    Material* material = load_material();

    if (texture_file_path) {
        Texture2D texture = LoadTexture(texture_file_path);
        SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);
        material->maps[0].texture = texture;
    }

    material->maps[0].color = WHITE;
    material->shader = LoadShader(0, "resources/shaders/sprite.frag");
    return material;
}

Material* load_shader_material(const char* vs_file_path, const char* fs_file_path) {
    Material* material = load_material();

    material->shader = LoadShader(vs_file_path, fs_file_path);
    return material;
}

Mesh* load_sphere_mesh(float radius) {
    Mesh* mesh = new_mesh_ptr();
    *mesh = GenMeshSphere(radius, 16, 16);
    return mesh;
}

Mesh* load_plane_mesh(float aspect) {
    Mesh* mesh = new_mesh_ptr();
    *mesh = GenMeshPlane(aspect, 1.0, 2, 2);
    return mesh;
}

void unload_resources(void) {
    for (size_t i = 0; N_MATERIALS != 0; ++i, --N_MATERIALS)
        UnloadMaterial(MATERIALS[i]);
    for (size_t i = 0; N_MESHES != 0; ++i, --N_MESHES)
        UnloadMesh(MESHES[i]);
}
