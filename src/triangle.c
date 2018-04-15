#include <stdio.h>
#include <math.h>

#include "./pi.h"
#include "./triangle.h"

triangle_t triangle(point_t p1, point_t p2, point_t p3)
{
    const triangle_t result = {
        .p1 = p1,
        .p2 = p2,
        .p3 = p3
    };

    return result;
}

triangle_t equilateral_triangle(point_t position, float radius, float angle)
{
    const float d = PI_2 / 3.0f;

    const triangle_t result = {
        .p1 = vec_sum(position, vec(cosf(0.0f + angle) * radius, sinf(0.0f + angle) * radius)),
        .p2 = vec_sum(position, vec(cosf(d + angle) * radius, sinf(d + angle) * radius)),
        .p3 = vec_sum(position, vec(cosf(2.0f * d + angle) * radius, sinf(2.0f * d + angle) * radius))
    };

    return result;
}

static void swap_points(point_t *p1, point_t *p2)
{
    point_t t = *p1;
    *p1 = *p2;
    *p2 = t;
}

triangle_t triangle_sorted_by_y(triangle_t t)
{
    if (t.p1.y > t.p2.y) { swap_points(&t.p1, &t.p2); }
    if (t.p2.y > t.p3.y) { swap_points(&t.p2, &t.p3); }
    if (t.p1.y > t.p2.y) { swap_points(&t.p1, &t.p2); }

    return t;
}

void triangulate_rect(rect_t rect, triangle_t *triangles)
{
    triangle_t t1 = {
        .p1 = { .x = rect.x, .y = rect.y },
        .p2 = { .x = rect.x + rect.w, .y = rect.y },
        .p3 = { .x = rect.x, .y = rect.y + rect.h }
    };

    triangle_t t2 = {
        .p1 = { .x = rect.x + rect.w, .y = rect.y },
        .p2 = { .x = rect.x, .y = rect.y + rect.h },
        .p3 = { .x = rect.x + rect.w, .y = rect.y + rect.h }
    };

    triangles[0] = t1;
    triangles[1] = t2;
}
