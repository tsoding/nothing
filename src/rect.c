#include <string.h>
#include <assert.h>
#include <math.h>
#include "./rect.h"

rect_t rect_int_area(const rect_t *rect1,
                     const rect_t *rect2)
{
    assert(rect1);
    assert(rect2);

    float x1 = fmaxf(rect1->x, rect2->x);
    float y1 = fmaxf(rect1->y, rect2->y);
    float x2 = fminf(rect1->x + rect1->w, rect2->x + rect2->w);
    float y2 = fminf(rect1->y + rect1->h, rect2->y + rect2->h);

    rect_t result = {
        .x = x1,
        .y = y1,
        .w = fmaxf(0.0f, x2 - x1),
        .h = fmaxf(0.0f, y2 - y1)
    };
    return result;
}

int is_rect_int(const rect_t *rect1,
                const rect_t *rect2)
{
    assert(rect1);
    assert(rect2);

    rect_t int_area = rect_int_area(rect1, rect2);
    return int_area.w * int_area.h > 0.0f;
}

float line_length(const line_t *line)
{
    float dx = line->p1.x - line->p2.x;
    float dy = line->p1.y - line->p2.y;
    return sqrtf(dx * dx + dy * dy);
}

void rect_object_impact(const rect_t *object,
                        const rect_t *obstacle,
                        int *sides)
{
    assert(object);
    assert(obstacle);
    assert(sides);

    rect_t int_area = rect_int_area(object, obstacle);

    if (int_area.w * int_area.h > 0.0f) {
        for (int side = 0; side < RECT_SIDE_N; ++side) {
            line_t object_side = rect_side(object, side);
            line_t int_side = rect_side(&int_area, side);

            if (line_length(&int_side) > 2.0f) {
                sides[side] =
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

line_t rect_side(const rect_t *rect,
                 enum rect_side_t side)
{
    assert(rect);
    assert(0 <= side && side < RECT_SIDE_N);

    const float x1 = rect->x;
    const float y1 = rect->y;
    const float x2 = rect->x + rect->w;
    const float y2 = rect->y + rect->h;

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
