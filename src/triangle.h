#pragma once

#include "./point.h"

typedef struct triangle_t {
    point_t p1, p2, p3;
} triangle_t;

triangle_t equilateral_triangle(point_t position, float radius, float angle);
