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
        .w = fmaxf(0.0f, x2 - x1 + 1),
        .h = fmaxf(0.0f, y2 - y1 + 1)
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

void rect_object_impact(const rect_t *object,
                        const rect_t *obstacle,
                        int *sides)
{
    assert(object);
    assert(obstacle);
    assert(sides);

    rect_t int_area = rect_int_area(object, obstacle);

    memset(sides, 0, sizeof(int) * RECT_SIDE_N);

    if (int_area.w * int_area.h > 0.0f) {
        for (int i = 0; i < RECT_SIDE_N; ++i) {

        }
    }
}

line_t rect_side(const rect_t *rect,
                 enum rect_side_t side)
{
    assert(rect);
    assert(0 <= side && side < RECT_SIDE_N);

    line_t result;
    return result;
}
