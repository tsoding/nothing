#include <math.h>
#include <stdio.h>

#include "math/pi.h"
#include "math/rand.h"
#include "triangle.h"

Triangle triangle(Vec2f p1, Vec2f p2, Vec2f p3)
{
    const Triangle result = {
        .p1 = p1,
        .p2 = p2,
        .p3 = p3
    };

    return result;
}

Triangle equilateral_triangle(void)
{
    const float d = PI_2 / 3.0f;

    const Triangle result = {
        .p1 = vec(cosf(0.0f), sinf(0.0f)),
        .p2 = vec(cosf(d), sinf(d)),
        .p3 = vec(cosf(2.0f * d), sinf(2.0f * d))
    };

    return result;
}

Triangle random_triangle(float radius)
{
    return triangle(
        vec_from_polar(rand_float(2 * PI), rand_float(radius)),
        vec_from_polar(rand_float(2 * PI), rand_float(radius)),
        vec_from_polar(rand_float(2 * PI), rand_float(radius)));
}

static void swap_points(Vec2f *p1, Vec2f *p2)
{
    Vec2f t = *p1;
    *p1 = *p2;
    *p2 = t;
}

Triangle triangle_sorted_by_y(Triangle t)
{
    if (t.p1.y > t.p2.y) { swap_points(&t.p1, &t.p2); }
    if (t.p2.y > t.p3.y) { swap_points(&t.p2, &t.p3); }
    if (t.p1.y > t.p2.y) { swap_points(&t.p1, &t.p2); }

    return t;
}

void rect_as_triangles(Rect rect, Triangle triangles[2])
{
    Triangle t1 = {
        .p1 = { .x = rect.x, .y = rect.y },
        .p2 = { .x = rect.x + rect.w, .y = rect.y },
        .p3 = { .x = rect.x, .y = rect.y + rect.h }
    };

    Triangle t2 = {
        .p1 = { .x = rect.x + rect.w, .y = rect.y },
        .p2 = { .x = rect.x, .y = rect.y + rect.h },
        .p3 = { .x = rect.x + rect.w, .y = rect.y + rect.h }
    };

    triangles[0] = t1;
    triangles[1] = t2;
}
