#ifndef POINT_H_
#define POINT_H_

typedef struct point_t {
    float x, y;
} point_t;

typedef point_t vec_t;

vec_t vec_sum(vec_t v1, vec_t v2);
void vec_add(vec_t *v1, vec_t v2);
vec_t vec_scala_mult(vec_t v, float scalar);

#endif  // POINT_H_
