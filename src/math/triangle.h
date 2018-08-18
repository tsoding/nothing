#ifndef TRIANGLE_H_
#define TRIANGLE_H_

#include "math/point.h"
#include "math/rect.h"

typedef struct Triangle {
    Point p1, p2, p3;
} Triangle;

Triangle triangle(Point p1, Point p2, Point p3);
Triangle equilateral_triangle(void);
Triangle random_triangle(float radius);
Triangle triangle_sorted_by_y(Triangle t);
void rect_as_triangles(Rect rect, Triangle triangles[2]);

Triangle triangle_mat3x3_product(Triangle t, mat3x3 m);

#endif  // TRIANGLE_H_
