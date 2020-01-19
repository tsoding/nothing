#ifndef RECT_H_
#define RECT_H_

#include <SDL.h>
#include <math.h>
#include <stdbool.h>

#include "math/vec.h"
#include "system/stacktrace.h"

typedef enum Rect_side {
    RECT_SIDE_TOP = 0,
    RECT_SIDE_LEFT,
    RECT_SIDE_BOTTOM,
    RECT_SIDE_RIGHT,

    RECT_SIDE_N
} Rect_side;

typedef struct Rect {
    float x, y, w, h;
} Rect;

typedef struct Line {
    Vec2f p1;
    Vec2f p2;
} Line;

Rect horizontal_thicc_line(float x1, float x2, float y, float thiccness);
Rect vertical_thicc_line(float y1, float y2, float x, float thiccness);

Rect rect(float x, float y, float w, float h);
Rect rect_from_vecs(Vec2f position, Vec2f size);
Rect rect_from_points(Vec2f p1, Vec2f p2);
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

static inline Vec2f rect_position(Rect rect)
{
    return vec(rect.x, rect.y);
}

static inline Vec2f rect_position2(Rect rect)
{
    return vec(rect.x + rect.w, rect.y + rect.h);
}

static inline Rect rect_pad(Rect rect, float d)
{
    rect.x -= d;
    rect.y -= d;
    rect.w += d * 2.0f;
    rect.h += d * 2.0f;
    return rect;
}

int rect_contains_point(Rect rect, Vec2f p);

int rects_overlap(Rect rect1, Rect rect2);

void rect_object_impact(Rect object,
                        Rect obstacle,
                        int *sides);

Line rect_side(Rect rect, Rect_side side);

Rect rect_from_point(Vec2f p, float w, float h);

float line_length(Line line);

SDL_Rect rect_for_sdl(Rect rect);

Vec2f rect_center(Rect rect);

Vec2f rect_snap(Rect pivot, Rect *rect);
Vec2f rect_impulse(Rect *r1, Rect *r2);

static inline
float rect_side_distance(Rect rect, Vec2f point, Rect_side side)
{
    switch (side) {
    case RECT_SIDE_LEFT: {
        return fabsf(rect.x - point.x);
    } break;

    case RECT_SIDE_RIGHT: {
        return fabsf((rect.x + rect.w) - point.x);
    } break;

    case RECT_SIDE_TOP: {
        return fabsf(rect.y - point.y);
    } break;

    case RECT_SIDE_BOTTOM: {
        return fabsf((rect.y + rect.h) - point.y);
    } break;

    case RECT_SIDE_N: {
        trace_assert(0 && "Incorrect rect side");
    } break;
    }

    return 0;
}

static inline
int segment_overlap(Vec2f a, Vec2f b)
{
    if (a.x > a.y) {
        float t = a.x;
        a.x = a.y;
        a.y = t;
    }

    if (b.x > b.y) {
        float t = b.x;
        b.x = b.y;
        b.y = t;
    }

    return a.y >= b.x && b.y >= a.x;
}

static inline
int snap_var(float *x,        // the value we are snapping
             float y,         // the target we are snapping x to
             float xo,        // x offset
             float yo,        // y offset
             float st)        // snap threshold
{
    if (fabsf((*x + xo) - (y + yo)) < st) {
        *x = y + yo - xo;
        return true;
    }
    return false;
}

static inline
int snap_var2seg(float *x, float y,
               float xo, float yo,
               float st)
{
    // note: do not use || because we do *not* want short-circuiting, so use |.
    return snap_var(x, y, xo,  0, st) | snap_var(x, y, xo, yo, st);
}

static inline
void snap_seg2seg(float *x, float y, float xo, float yo, float st)
{
    snap_var(x, y,  0,  0, st);
    snap_var(x, y,  0, yo, st);
    snap_var(x, y, xo,  0, st);
    snap_var(x, y, xo, yo, st);
}

#endif  // RECT_H_
