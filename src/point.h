#ifndef POINT_H_
#define POINT_H_

typedef struct point_t {
    float x, y;
} point_t;

typedef point_t vec_t;

vec_t vec_sum(const vec_t *v1, const vec_t *v2);
vec_t vec_scala_mult(const vec_t *v, float scalar);

#endif  // POINT_H_
