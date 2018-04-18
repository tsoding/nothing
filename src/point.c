#include <assert.h>
#include <math.h>
#include "./point.h"

vec_t vec(float x, float y)
{
    vec_t result = {
        .x = x,
        .y = y
    };
    return result;
}

vec_t vec_from_ps(point_t p1, point_t p2)
{
    vec_t result = {
        .x = p2.x - p1.x,
        .y = p2.y - p1.y
    };
    return result;
}

float vec_arg(vec_t v)
{
    return atan2f(v.y, v.x);
}

float vec_mag(vec_t v)
{
    return sqrtf(v.x * v.x + v.y * v.y);
}

vec_t vec_sum(vec_t v1, vec_t v2)
{
    vec_t result = {
        .x = v1.x + v2.x,
        .y = v1.y + v2.y
    };
    return result;
}

vec_t vec_neg(vec_t v)
{
    vec_t result = {
        .x = -v.x,
        .y = -v.y
    };

    return result;
}

float vec_length(vec_t v)
{
    return sqrtf(v.x * v.x + v.y * v.y);
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

vec_t vec_entry_mult(vec_t v1, vec_t v2)
{
    vec_t result = {
        .x = v1.x * v2.x,
        .y = v1.y * v2.y
    };

    return result;
}

vec_t vec_entry_div(vec_t v1, vec_t v2)
{
    vec_t result = {
        .x = v1.x / v2.x,
        .y = v1.y / v2.y
    };

    return result;
}

float rad_to_deg(float a)
{
    return 180 / PI * a;
}

point_t point_mat3x3_product(point_t p, mat3x3 m)
{
    /* Convert p to Homogeneous coordinates */
    const float homo_p[3] = {p.x, p.y, 1};

    /* Transform p with matrix m */
    const float trans_p[3] = {
        homo_p[0] * m.M[0][0] + homo_p[1] * m.M[0][1] + homo_p[2] * m.M[0][2],
        homo_p[0] * m.M[1][0] + homo_p[1] * m.M[1][1] + homo_p[2] * m.M[1][2],
        homo_p[0] * m.M[2][0] + homo_p[1] * m.M[2][1] + homo_p[2] * m.M[2][2]
    };

    /* Convert p back to Cartesian coordinates */
    const point_t result_p = {
        .x = trans_p[0] / trans_p[2],
        .y = trans_p[1] / trans_p[2]
    };

    return result_p;
}
