#include "./mat3x3.h"

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
