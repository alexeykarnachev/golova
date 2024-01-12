#pragma once

#include "raylib.h"

Matrix get_transform_matrix(Transform t);
BoundingBox get_mesh_bounding_box(Mesh mesh, Transform transform);
