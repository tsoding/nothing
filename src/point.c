#include "./point.h"

vec_t vec_sum(const vec_t *v1, const vec_t *v2)
{
    vec_t result = {
        .x = v1->x + v2->x,
        .y = v1->y + v2->y
    };
    return result;
}

vec_t vec_scala_mult(const vec_t *v, float scalar)
{
    vec_t result = {
        .x = v->x * scalar,
        .y = v->y * scalar
    };
    return result;
}
