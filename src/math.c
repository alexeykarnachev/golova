#include "math.h"

#include "raymath.h"
#include <stdio.h>

Transform get_default_transform(void) {
    Transform transform = {0};
    transform.scale = Vector3One();
    transform.rotation.w = 1.0;
    return transform;
}

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
