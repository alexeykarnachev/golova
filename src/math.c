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

BoundingBox get_mesh_bounding_box(Mesh mesh, Transform transform) {
    BoundingBox box = GetMeshBoundingBox(mesh);
    Matrix m = get_transform_matrix(transform);
    box.max = Vector3Transform(box.max, m);
    box.min = Vector3Transform(box.min, m);
    return box;
}
