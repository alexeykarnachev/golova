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

void load_resources(void) {
    DEFAULT_MATERIAL = &MATERIALS[N_MATERIALS++];
    *DEFAULT_MATERIAL = LoadMaterialDefault();

    DEFAULT_PLANE_MESH = &MESHES[N_MESHES++];
    *DEFAULT_PLANE_MESH = GenMeshPlane(1.0, 1.0, 2, 2);

    DEFAULT_SPHERE_MESH = &MESHES[N_MESHES++];
    *DEFAULT_SPHERE_MESH = GenMeshSphere(1.0, 16, 16);

    DEFAULT_CUBE_MESH = &MESHES[N_MESHES++];
    *DEFAULT_CUBE_MESH = GenMeshCube(1.0, 1.0, 1.0);
}

void unload_resources(void) {
    for (size_t i = 0; N_MATERIALS != 0; ++i, --N_MATERIALS)
        UnloadMaterial(MATERIALS[i]);
    for (size_t i = 0; N_MESHES != 0; ++i, --N_MESHES)
        UnloadMesh(MESHES[i]);
}
