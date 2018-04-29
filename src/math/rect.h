#ifndef RECT_H_
#define RECT_H_

#include <SDL2/SDL.h>

#include "math/point.h"

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

rect_t rect(float x, float y, float w, float h);
rect_t rect_from_vecs(point_t position, vec_t size);
rect_t rect_from_sdl(const SDL_Rect *rect);

rect_t rects_overlap_area(rect_t rect1, rect_t rect2);

int rect_contains_point(rect_t rect, point_t p);

int rects_overlap(rect_t rect1, rect_t rect2);

void rect_object_impact(const rect_t *object,
                        const rect_t *obstacle,
                        int *sides);

line_t rect_side(rect_t rect, rect_side_t side);

rect_t rect_from_point(point_t p, float w, float h);

float line_length(line_t line);

SDL_Rect rect_for_sdl(rect_t rect);

#endif  // RECT_H_
