#ifndef RECT_H_
#define RECT_H_

#include <SDL.h>

#include "math/point.h"

typedef enum Rect_side {
    RECT_SIDE_LEFT = 0,
    RECT_SIDE_RIGHT,
    RECT_SIDE_TOP,
    RECT_SIDE_BOTTOM,

    RECT_SIDE_N
} Rect_side;

typedef struct Rect {
    float x, y, w, h;
} Rect;

typedef struct Line {
    Point p1;
    Point p2;
} Line;

Rect horizontal_thicc_line(float x1, float x2, float y, float thiccness);
Rect vertical_thicc_line(float y1, float y2, float x, float thiccness);

Rect rect(float x, float y, float w, float h);
Rect rect_from_vecs(Point position, Vec size);
Rect rect_from_points(Point p1, Point p2);
Rect rect_from_sdl(const SDL_Rect *rect);

Rect rects_overlap_area(Rect rect1, Rect rect2);

static inline
Rect rect_boundary2(Rect rect1, Rect rect2)
{
    return rect_from_points(
        vec(
            fminf(rect1.x, rect2.x),
            fminf(rect1.y, rect2.y)),
        vec(
            fmaxf(rect1.x + rect1.w, rect2.x + rect2.w),
            fmaxf(rect1.y + rect1.h, rect2.y + rect2.h)));
}

static inline Point rect_position(Rect rect)
{
    return vec(rect.x, rect.y);
}

static inline Rect rect_scale(Rect rect, float d)
{
    rect.x -= d;
    rect.y -= d;
    rect.w += d * 2.0f;
    rect.h += d * 2.0f;
    return rect;
}

int rect_contains_point(Rect rect, Point p);

int rects_overlap(Rect rect1, Rect rect2);

void rect_object_impact(Rect object,
                        Rect obstacle,
                        int *sides);

Line rect_side(Rect rect, Rect_side side);

Rect rect_from_point(Point p, float w, float h);

float line_length(Line line);

SDL_Rect rect_for_sdl(Rect rect);

Vec rect_center(Rect rect);

Vec rect_snap(Rect pivot, Rect *rect);
Vec rect_impulse(Rect *r1, Rect *r2);

#endif  // RECT_H_
