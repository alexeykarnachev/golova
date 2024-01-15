#pragma once

#include "raylib.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define CLAMP(value, min, max) \
    (((value) < (min)) ? (min) : (((value) > (max)) ? (max) : (value)))

Transform get_default_transform(void);
Matrix get_transform_matrix(Transform transform);
