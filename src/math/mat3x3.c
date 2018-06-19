#include <math.h>

#include "mat3x3.h"

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

mat3x3 mat3x3_product2(mat3x3 m1, mat3x3 m2, mat3x3 m3)
{
    return mat3x3_product(m1, mat3x3_product(m2, m3));
}

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
