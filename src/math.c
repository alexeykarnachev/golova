#include "math.h"

#include "raymath.h"

Matrix get_transform_matrix(Transform transform) {
    Vector3 t = transform.translation;
    Vector3 s = transform.scale;
    Quaternion q = transform.rotation;

    Matrix mt = MatrixTranslate(t.x, t.y, t.z);
    Matrix mr = QuaternionToMatrix(q);
    Matrix ms = MatrixScale(s.x, s.y, s.z);
    Matrix m = MatrixMultiply(ms, MatrixMultiply(mr, mt));

    return m;
}

BoundingBox get_mesh_bounding_box(Mesh mesh, Matrix mat) {
    BoundingBox box = GetMeshBoundingBox(mesh);
    box.max = Vector3Transform(box.max, mat);
    box.min = Vector3Transform(box.min, mat);
    return box;
}
