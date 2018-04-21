#ifndef POINT_H_
#define POINT_H_

#include "./pi.h"
#include "./mat3x3.h"

typedef struct point_t {
    float x, y;
} point_t;

typedef point_t vec_t;

vec_t vec(float x, float y);
vec_t vec_from_polar(float arg, float mag);
vec_t vec_from_ps(point_t p1, point_t p2);

float vec_arg(vec_t v);
float vec_mag(vec_t v);

float rad_to_deg(float a);

vec_t vec_sum(vec_t v1, vec_t v2);
vec_t vec_neg(vec_t v);
void vec_add(vec_t *v1, vec_t v2);
float vec_length(vec_t v);
vec_t vec_scala_mult(vec_t v, float scalar);
vec_t vec_entry_mult(vec_t v1, vec_t v2);
vec_t vec_entry_div(vec_t v1, vec_t v2);

point_t point_mat3x3_product(point_t p, mat3x3 m);

#endif  // POINT_H_
