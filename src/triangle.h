#ifndef TRIANGLE_H_
#define TRIANGLE_H_

#include "./rect.h"
#include "./point.h"

typedef struct triangle_t {
    point_t p1, p2, p3;
} triangle_t;

triangle_t triangle(point_t p1, point_t p2, point_t p3);
triangle_t equilateral_triangle(point_t position, float radius, float angle);
triangle_t triangle_sorted_by_y(triangle_t t);
void triangulate_rect(rect_t rect, triangle_t *triangles);

triangle_t triangle_mat3x3_product(triangle_t t, mat3x3 m);

#endif  // TRIANGLE_H_
