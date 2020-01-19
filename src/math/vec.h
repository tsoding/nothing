#ifndef POINT_H_
#define POINT_H_

#include <math.h>
#include "math/pi.h"

typedef struct {
    float x, y;
} Vec2f;

static inline
Vec2f vec(float x, float y)
{
    Vec2f result = {
        .x = x,
        .y = y
    };
    return result;
}

static inline
Vec2f vec_mult(Vec2f v1, Vec2f v2)
{
    return vec(v1.x * v2.x, v1.y * v2.y);
}

static inline
Vec2f vec_scala_mult(Vec2f v, float scalar)
{
    Vec2f result = {
        .x = v.x * scalar,
        .y = v.y * scalar
    };
    return result;
}

static inline
Vec2f vec_from_polar(float arg, float mag)
{
    return vec_scala_mult(
        vec(cosf(arg), sinf(arg)),
        mag);
}

static inline
Vec2f vec_from_ps(Vec2f p1, Vec2f p2)
{
    Vec2f result = {
        .x = p2.x - p1.x,
        .y = p2.y - p1.y
    };
    return result;
}

static inline
float vec_arg(Vec2f v)
{
    return atan2f(v.y, v.x);
}

static inline
float vec_mag(Vec2f v)
{
    return sqrtf(v.x * v.x + v.y * v.y);
}

static inline
Vec2f vec_sum(Vec2f v1, Vec2f v2)
{
    Vec2f result = {
        .x = v1.x + v2.x,
        .y = v1.y + v2.y
    };
    return result;
}

static inline
Vec2f vec_sub(Vec2f v1, Vec2f v2)
{
    Vec2f result = {
        .x = v1.x - v2.x,
        .y = v1.y - v2.y
    };
    return result;
}

static inline
Vec2f vec_neg(Vec2f v)
{
    Vec2f result = {
        .x = -v.x,
        .y = -v.y
    };

    return result;
}

static inline
float vec_length(Vec2f v)
{
    return sqrtf(v.x * v.x + v.y * v.y);
}

static inline
void vec_add(Vec2f *v1, Vec2f v2)
{
    v1->x += v2.x;
    v1->y += v2.y;
}

static inline
Vec2f vec_entry_mult(Vec2f v1, Vec2f v2)
{
    Vec2f result = {
        .x = v1.x * v2.x,
        .y = v1.y * v2.y
    };

    return result;
}

static inline
Vec2f vec_entry_div(Vec2f v1, Vec2f v2)
{
    Vec2f result = {
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
Vec2f vec_norm(Vec2f v)
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
float vec_sqr_norm(Vec2f v)
{
    return v.x * v.x + v.y * v.y;
}

#define vec_scale vec_scala_mult

typedef struct {
    int x, y;
} Vec2i;

static inline
Vec2i vec2i(int x, int y)
{
    Vec2i resoolt = {
        .x = x,
        .y = y
    };
    return resoolt;
}

#endif  // POINT_H_
