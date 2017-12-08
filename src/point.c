#include "./point.h"

vec_t vec_sum(vec_t v1, vec_t v2)
{
    vec_t result = {
        .x = v1.x + v2.x,
        .y = v1.y + v2.y
    };
    return result;
}

void vec_add(vec_t *v1, vec_t v2)
{
    v1->x += v2.x;
    v1->y += v2.y;
}

vec_t vec_scala_mult(vec_t v, float scalar)
{
    vec_t result = {
        .x = v.x * scalar,
        .y = v.y * scalar
    };
    return result;
}
