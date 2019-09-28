#ifndef TRIANGLE_H_
#define TRIANGLE_H_

#include "math/vec.h"
#include "math/rect.h"

typedef struct Triangle {
    Vec2f p1, p2, p3;
} Triangle;

Triangle triangle(Vec2f p1, Vec2f p2, Vec2f p3);
Triangle equilateral_triangle(void);
Triangle random_triangle(float radius);
Triangle triangle_sorted_by_y(Triangle t);
void rect_as_triangles(Rect rect, Triangle triangles[2]);

#endif  // TRIANGLE_H_
