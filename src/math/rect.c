#include <SDL.h>
#include <math.h>
#include <string.h>

#include "rect.h"
#include "system/stacktrace.h"

Rect rect(float x, float y, float w, float h)
{
    const Rect result = {
        .x = x,
        .y = y,
        .w = w,
        .h = h
    };

    return result;
}

Rect rect_from_vecs(Vec2f position, Vec2f size)
{
    return rect(position.x, position.y, size.x, size.y);
}

Rect rect_from_points(Vec2f p1, Vec2f p2)
{
    return rect_from_vecs(
        vec(fminf(p1.x, p2.x),
            fminf(p1.y, p2.y)),
        vec(fabsf(p1.x - p2.x),
            fabsf(p1.y - p2.y)));
}

Rect rect_from_sdl(const SDL_Rect *rect)
{
    const Rect result = {
        .x = (float) rect->x,
        .y = (float) rect->y,
        .w = (float) rect->w,
        .h = (float) rect->h
    };

    return result;
}

Rect rects_overlap_area(Rect rect1, Rect rect2)
{
    float x1 = fmaxf(rect1.x, rect2.x);
    float y1 = fmaxf(rect1.y, rect2.y);
    float x2 = fminf(rect1.x + rect1.w, rect2.x + rect2.w);
    float y2 = fminf(rect1.y + rect1.h, rect2.y + rect2.h);

    Rect result = {
        .x = x1,
        .y = y1,
        .w = fmaxf(0.0f, x2 - x1),
        .h = fmaxf(0.0f, y2 - y1)
    };
    return result;
}

int rects_overlap(Rect rect1, Rect rect2)
{
    return rect1.x + rect1.w > rect2.x
        && rect2.x + rect2.w > rect1.x
        && rect2.y + rect2.h > rect1.y
        && rect1.y + rect1.h > rect2.y;
}

float line_length(Line line)
{
    float dx = line.p1.x - line.p2.x;
    float dy = line.p1.y - line.p2.y;
    return sqrtf(dx * dx + dy * dy);
}

void rect_object_impact(Rect object, Rect obstacle, int *sides)
{
    trace_assert(sides);

    Rect int_area = rects_overlap_area(object, obstacle);

    if (int_area.w * int_area.h > 0.0f) {
        for (Rect_side side = 0; side < RECT_SIDE_N; ++side) {
            Line object_side = rect_side(object, side);
            Line int_side = rect_side(int_area, side);

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

Line rect_side(Rect rect, Rect_side side)
{
    const float x1 = rect.x;
    const float y1 = rect.y;
    const float x2 = rect.x + rect.w;
    const float y2 = rect.y + rect.h;

    Line result = {
        .p1 = {0, 0},
        .p2 = {0, 0}
    };

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

Rect rect_from_point(Vec2f p, float w, float h)
{
    Rect result = {
        .x = p.x,
        .y = p.y,
        .w = w,
        .h = h
    };

    return result;
}

int rect_contains_point(Rect rect, Vec2f p)
{
    return rect.x <= p.x && p.x <= rect.x + rect.w
        && rect.y <= p.y && p.y <= rect.y + rect.h;
}

SDL_Rect rect_for_sdl(Rect rect)
{
    const SDL_Rect result = {
        .x = (int) roundf(rect.x),
        .y = (int) roundf(rect.y),
        .w = (int) roundf(rect.w),
        .h = (int) roundf(rect.h)
    };

    return result;
}

Vec2f rect_center(Rect rect)
{
    return vec(rect.x + rect.w * 0.5f,
               rect.y + rect.h * 0.5f);
}

Vec2f rect_snap(Rect pivot, Rect *r)
{
    const Vec2f pivot_c = rect_center(pivot);
    const Vec2f r_c = rect_center(*r);

    const float sx = r_c.x < pivot_c.x ? -1.0f : 1.0f;
    const float sy = r_c.y < pivot_c.y ? -1.0f : 1.0f;
    const float cx = pivot_c.x + sx * (pivot.w + r->w) * 0.5f;
    const float cy = pivot_c.y + sy * (pivot.h + r->h) * 0.5f;

    if (fabsf(cx - r_c.x) < fabsf(cy - r_c.y)) {
        *r = rect(cx - r->w * 0.5f, r->y, r->w, r->h);
        return vec(0.0f, 1.0f);
    } else {
        *r = rect(r->x, cy - r->h * 0.5f, r->w, r->h);
        return vec(1.0f, 0.0f);
    }
}

Vec2f rect_impulse(Rect *r1, Rect *r2)
{
    trace_assert(r1);
    trace_assert(r2);

    const Vec2f c1 = rect_center(*r1);
    const Vec2f c2 = rect_center(*r2);
    const Rect overlap = rects_overlap_area(*r1, *r2);
    const Vec2f overlap_center = rect_center(overlap);
    const float dx = overlap_center.x;
    const float dy = overlap_center.y;
    const float sx = c1.x < c2.x ? 1.0f : -1.0f;
    const float sy = c1.y < c2.y ? 1.0f : -1.0f;
    const float cx1 = dx - sx * r1->w * 0.5f;
    const float cy1 = dy - sy * r1->h * 0.5f;
    const float cx2 = dx + sx * r2->w * 0.5f;
    const float cy2 = dy + sy * r2->h * 0.5f;

    if (vec_sqr_norm(vec_sum(vec(cx1, c1.y), vec_neg(vec(cx2, c2.y)))) <
        vec_sqr_norm(vec_sum(vec(c1.x, cy1), vec_neg(vec(c2.x, cy2))))) {
        r1->x = cx1 - r1->w * 0.5f;
        r2->x = cx2 - r2->w * 0.5f;
        return vec(0.0f, 1.0f);
    } else {
        r1->y = cy1 - r1->h * 0.5f;
        r2->y = cy2 - r2->h * 0.5f;
        return vec(1.0f, 0.0f);
    }
}

Rect horizontal_thicc_line(float x1, float x2, float y, float thiccness)
{
    if (x1 > x2) {
        float t = x1;
        x1 = x2;
        x2 = t;
    }

    return rect(
        x1 - thiccness * 0.5f,
        y - thiccness * 0.5f,
        x2 - x1 + thiccness,
        thiccness);
}

Rect vertical_thicc_line(float y1, float y2, float x, float thiccness)
{
    if (y1 > y2) {
        float t = y1;
        y1 = y2;
        y2 = t;
    }

    return rect(
        x - thiccness * 0.5f,
        y1 - thiccness * 0.5f,
        thiccness,
        y2 - y1 + thiccness);
}
