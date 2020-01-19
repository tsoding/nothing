#ifndef MAT3X3_H_
#define MAT3X3_H_

#include "vec.h"

typedef struct mat3x3 {
    float M[3][3];
} mat3x3;

static inline
mat3x3 make_mat3x3(float a11, float a12, float a13,
                   float a21, float a22, float a23,
                   float a31, float a32, float a33)
{
    const mat3x3 m = {
        .M = {
            {a11, a12, a13},
            {a21, a22, a23},
            {a31, a32, a33}
        }
    };

    return m;
}

static inline
mat3x3 mat3x3_product(mat3x3 m1, mat3x3 m2)
{
    mat3x3 result;

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            result.M[i][j] = 0;
            for (int k = 0; k < 3; ++k) {
                result.M[i][j] += m1.M[i][k] * m2.M[k][j];
            }
        }
    }

    return result;
}

static inline
mat3x3 mat3x3_product2(mat3x3 m1, mat3x3 m2, mat3x3 m3)
{
    return mat3x3_product(m1, mat3x3_product(m2, m3));
}

static inline
mat3x3 trans_mat(float x, float y)
{
    const mat3x3 m = {
        .M = {
            {1.0f, 0.0f, x},
            {0.0f, 1.0f, y},
            {0.0f, 0.0f, 1.0f}
        }
    };

    return m;
}

static inline
mat3x3 trans_mat_vec(Vec2f v)
{
    return trans_mat(v.x, v.y);
}

static inline
mat3x3 rot_mat(float angle)
{
    const mat3x3 m = {
        .M = {
            {cosf(angle), -sinf(angle), 0.0f},
            {sinf(angle), cosf(angle), 0.0f},
            {0.0f, 0.0f, 1.0f}
        }
    };

    return m;
}

static inline
mat3x3 scale_mat(float factor)
{
    const mat3x3 m = {
        .M = {
            {factor, 0.0f, 0.0f},
            {0.0f, factor, 0.0f},
            {0.0f, 0.0f, 1.0f}
        }
    };

    return m;
}

static inline
Vec2f point_mat3x3_product(Vec2f p, mat3x3 m)
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
    const Vec2f result_p = {
        .x = trans_p[0] / trans_p[2],
        .y = trans_p[1] / trans_p[2]
    };

    return result_p;
}

static inline
Triangle triangle_mat3x3_product(Triangle t, mat3x3 m)
{
    Triangle t1 = {
        .p1 = point_mat3x3_product(t.p1, m),
        .p2 = point_mat3x3_product(t.p2, m),
        .p3 = point_mat3x3_product(t.p3, m)
    };

    return t1;
}

#endif  // MAT3X3_H_
