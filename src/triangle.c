#include <stdio.h>
#include <math.h>

#include "./pi.h"
#include "./triangle.h"
#include "./algo.h"

triangle_t triangle(point_t p1, point_t p2, point_t p3)
{
    const triangle_t result = {
        .p1 = p1,
        .p2 = p2,
        .p3 = p3
    };

    return result;
}

triangle_t equilateral_triangle(void)
{
    const float d = PI_2 / 3.0f;

    const triangle_t result = {
        .p1 = vec(cosf(0.0f), sinf(0.0f)),
        .p2 = vec(cosf(d), sinf(d)),
        .p3 = vec(cosf(2.0f * d), sinf(2.0f * d))
    };

    return result;
}

/* TODO(#151): some of the random triangles are too thin */
triangle_t random_triangle(float radius)
{
    return triangle(
        vec_from_polar(rand_float(2 * PI), rand_float(radius)),
        vec_from_polar(rand_float(2 * PI), rand_float(radius)),
        vec_from_polar(rand_float(2 * PI), rand_float(radius)));
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

void rect_as_triangles(rect_t rect, triangle_t triangles[2])
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

triangle_t triangle_mat3x3_product(triangle_t t, mat3x3 m)
{
    triangle_t t1 = {
        .p1 = point_mat3x3_product(t.p1, m),
        .p2 = point_mat3x3_product(t.p2, m),
        .p3 = point_mat3x3_product(t.p3, m)
    };

    return t1;
}
