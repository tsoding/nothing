#ifndef MAT3X3_H_
#define MAT3X3_H_

typedef struct mat3x3 {
    float M[3][3];
} mat3x3;

mat3x3 mat3x3_product(mat3x3 m1, mat3x3 m2);
mat3x3 trans_mat(float x, float y);
mat3x3 rot_mat(float angle);

#endif  // MAT3X3_H_
