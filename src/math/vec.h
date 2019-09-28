#ifndef POINT_H_
#define POINT_H_

#include "math/pi.h"

typedef struct Point {
    float x, y;
} Point;

typedef Point Vec;

static inline
Vec vec(float x, float y)
{
    Vec result = {
        .x = x,
        .y = y
    };
    return result;
}

static inline
Vec vec_scala_mult(Vec v, float scalar)
{
    Vec result = {
        .x = v.x * scalar,
        .y = v.y * scalar
    };
    return result;
}

static inline
Vec vec_from_polar(float arg, float mag)
{
    return vec_scala_mult(
        vec(cosf(arg), sinf(arg)),
        mag);
}

static inline
Vec vec_from_ps(Point p1, Point p2)
{
    Vec result = {
        .x = p2.x - p1.x,
        .y = p2.y - p1.y
    };
    return result;
}

static inline
float vec_arg(Vec v)
{
    return atan2f(v.y, v.x);
}

static inline
float vec_mag(Vec v)
{
    return sqrtf(v.x * v.x + v.y * v.y);
}

static inline
Vec vec_sum(Vec v1, Vec v2)
{
    Vec result = {
        .x = v1.x + v2.x,
        .y = v1.y + v2.y
    };
    return result;
}

static inline
Vec vec_sub(Vec v1, Vec v2)
{
    Vec result = {
        .x = v1.x - v2.x,
        .y = v1.y - v2.y
    };
    return result;
}

static inline
Vec vec_neg(Vec v)
{
    Vec result = {
        .x = -v.x,
        .y = -v.y
    };

    return result;
}

static inline
float vec_length(Vec v)
{
    return sqrtf(v.x * v.x + v.y * v.y);
}

static inline
void vec_add(Vec *v1, Vec v2)
{
    v1->x += v2.x;
    v1->y += v2.y;
}

static inline
Vec vec_entry_mult(Vec v1, Vec v2)
{
    Vec result = {
        .x = v1.x * v2.x,
        .y = v1.y * v2.y
    };

    return result;
}

static inline
Vec vec_entry_div(Vec v1, Vec v2)
{
    Vec result = {
        .x = v1.x / v2.x,
        .y = v1.y / v2.y
    };

    return result;
}

static inline
float rad_to_deg(float a)
{
    return 180 / PI * a;
}

static inline
Vec vec_norm(Vec v)
{
    // TODO(#657): math/point/vec_norm: using vec_length is too expensive
    //   It involves multiplication and sqrt. We can just check if its components are close to 0.0f.

    const float l = vec_length(v);

    if (l < 1e-6) {
        return vec(0.0f, 0.0f);
    }

    return vec(v.x / l, v.y / l);
}

static inline
float vec_sqr_norm(Vec v)
{
    return v.x * v.x + v.y * v.y;
}

#define vec_scale vec_scala_mult

#endif  // POINT_H_
