#ifndef POINT_H_
#define POINT_H_

#include "math/pi.h"

typedef struct Point {
    float x, y;
} Point;

typedef Point Vec;

Vec vec(float x, float y);
Vec vec_from_polar(float arg, float mag);
Vec vec_from_ps(Point p1, Point p2);

float vec_arg(Vec v);
float vec_mag(Vec v);

float rad_to_deg(float a);

Vec vec_sum(Vec v1, Vec v2);
Vec vec_sub(Vec v1, Vec v2);
Vec vec_neg(Vec v);
void vec_add(Vec *v1, Vec v2);
float vec_length(Vec v);
float vec_sqr_norm(Vec v);
Vec vec_scala_mult(Vec v, float scalar);
#define vec_scale vec_scala_mult
Vec vec_entry_mult(Vec v1, Vec v2);
Vec vec_entry_div(Vec v1, Vec v2);
Vec vec_norm(Vec v);



#endif  // POINT_H_
