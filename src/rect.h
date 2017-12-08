#ifndef RECT_H_
#define RECT_H_

#include "./point.h"

typedef enum rect_side_t {
    RECT_SIDE_LEFT = 0,
    RECT_SIDE_RIGHT,
    RECT_SIDE_TOP,
    RECT_SIDE_BOTTOM,

    RECT_SIDE_N
} rect_side_t;

typedef struct rect_t {
    float x, y, w, h;
} rect_t;

typedef struct line_t {
    point_t p1;
    point_t p2;
} line_t;

rect_t rect_int_area(rect_t rect1, rect_t rect2);

int is_rect_int(rect_t rect1, rect_t rect2);

void rect_object_impact(const rect_t *object,
                        const rect_t *obstacle,
                        int *sides);

line_t rect_side(rect_t rect, rect_side_t side);

rect_t rect_from_point(point_t p, float w, float h);

#endif  // RECT_H_
