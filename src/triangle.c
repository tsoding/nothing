#include <math.h>

#include "./pi.h"
#include "./triangle.h"

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
