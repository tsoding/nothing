#include <math.h>
#include "./rect.h"

int rects_intersect(const struct rect_t *rect1,
                    const struct rect_t *rect2)
{
    float x1 = fmaxf(rect1->x, rect2->x);
    float y1 = fmaxf(rect1->y, rect2->y);
    float x2 = fminf(rect1->x + rect1->w, rect2->x + rect2->w);
    float y2 = fminf(rect1->y + rect1->h, rect2->y + rect2->h);
    return x1 <= x2 && y1 <= y2;
}
