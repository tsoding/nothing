#include <SDL2/SDL.h>
#include <assert.h>
#include <math.h>
#include <string.h>

#include "./rect.h"

rect_t rect(float x, float y, float w, float h)
{
    const rect_t result = {
        .x = x,
        .y = y,
        .w = w,
        .h = h
    };

    return result;
}

rect_t rect_from_vecs(point_t position, vec_t size)
{
    return rect(position.x, position.y, size.x, size.y);
}

rect_t rect_from_sdl(const SDL_Rect *rect)
{
    const rect_t result = {
        .x = (float) rect->x,
        .y = (float) rect->y,
        .w = (float) rect->w,
        .h = (float) rect->h
    };

    return result;
}

rect_t rects_overlap_area(rect_t rect1, rect_t rect2)
{
    float x1 = fmaxf(rect1.x, rect2.x);
    float y1 = fmaxf(rect1.y, rect2.y);
    float x2 = fminf(rect1.x + rect1.w, rect2.x + rect2.w);
    float y2 = fminf(rect1.y + rect1.h, rect2.y + rect2.h);

    rect_t result = {
        .x = x1,
        .y = y1,
        .w = fmaxf(0.0f, x2 - x1),
        .h = fmaxf(0.0f, y2 - y1)
    };
    return result;
}

int rects_overlap(rect_t rect1, rect_t rect2)
{
    return rect1.x + rect1.w >= rect2.x
        && rect2.x + rect2.w >= rect1.x
        && rect2.y + rect2.h >= rect1.y
        && rect1.y + rect1.h >= rect2.y;
}

float line_length(line_t line)
{
    float dx = line.p1.x - line.p2.x;
    float dy = line.p1.y - line.p2.y;
    return sqrtf(dx * dx + dy * dy);
}

void rect_object_impact(const rect_t *object,
                        const rect_t *obstacle,
                        int *sides)
{
    assert(object);
    assert(obstacle);
    assert(sides);

    rect_t int_area = rects_overlap_area(*object, *obstacle);

    if (int_area.w * int_area.h > 0.0f) {
        for (int side = 0; side < RECT_SIDE_N; ++side) {
            line_t object_side = rect_side(*object, side);
            line_t int_side = rect_side(int_area, side);

            if (line_length(int_side) > 10.0f) {
                sides[side] = sides[side] ||
                    (fabs(object_side.p1.x - object_side.p2.x) < 1e-6
                     && fabs(object_side.p1.x - int_side.p1.x) < 1e-6
                     && fabs(object_side.p1.x - int_side.p2.x) < 1e-6)
                    || (fabs(object_side.p1.y - object_side.p2.y) < 1e-6
                        && fabs(object_side.p1.y - int_side.p1.y) < 1e-6
                        && fabs(object_side.p1.y - int_side.p2.y) < 1e-6);
            }
        }
    }
}

line_t rect_side(rect_t rect, rect_side_t side)
{
    const float x1 = rect.x;
    const float y1 = rect.y;
    const float x2 = rect.x + rect.w;
    const float y2 = rect.y + rect.h;

    line_t result;

    switch (side) {
    case RECT_SIDE_LEFT:
        result.p1.x = x1;
        result.p1.y = y1;
        result.p2.x = x1;
        result.p2.y = y2;
        break;
    case RECT_SIDE_RIGHT:
        result.p1.x = x2;
        result.p1.y = y1;
        result.p2.x = x2;
        result.p2.y = y2;
        break;
    case RECT_SIDE_TOP:
        result.p1.x = x1;
        result.p1.y = y1;
        result.p2.x = x2;
        result.p2.y = y1;
        break;
    case RECT_SIDE_BOTTOM:
        result.p1.x = x1;
        result.p1.y = y2;
        result.p2.x = x2;
        result.p2.y = y2;
        break;
    default: {}
    }

    return result;
}

rect_t rect_from_point(point_t p, float w, float h)
{
    rect_t result = {
        .x = p.x,
        .y = p.y,
        .w = w,
        .h = h
    };

    return result;
}

int rect_contains_point(rect_t rect, point_t p)
{
    return rect.x <= p.x && p.x <= rect.x + rect.w
        && rect.y <= p.y && p.y <= rect.y + rect.h;
}

SDL_Rect rect_for_sdl(rect_t rect)
{
    const SDL_Rect result = {
        .x = (int) roundf(rect.x),
        .y = (int) roundf(rect.y),
        .w = (int) roundf(rect.w),
        .h = (int) roundf(rect.h)
    };

    return result;
}
