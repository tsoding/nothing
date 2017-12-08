#ifndef RECT_H_
#define RECT_H_

#include "./point.h"

enum rect_side_t {
    RECT_SIDE_LEFT = 0,
    RECT_SIDE_RIGHT,
    RECT_SIDE_TOP,
    RECT_SIDE_BOTTOM,

    RECT_SIDE_N
};

typedef struct {
    float x, y, w, h;
} rect_t;

typedef struct {
    point_t p1;
    point_t p2;
} line_t;

rect_t rect_int_area(const rect_t *rect1,
                     const rect_t *rect2);

int is_rect_int(const rect_t *rect1,
                const rect_t *rect2);

void rect_object_impact(const rect_t *object,
                        const rect_t *obstacle,
                        int *sides);

line_t rect_side(const rect_t *rect,
                 enum rect_side_t side);

rect_t rect_from_point(point_t p, float w, float h);

#endif  // RECT_H_
