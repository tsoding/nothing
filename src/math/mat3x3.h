#ifndef MAT3X3_H_
#define MAT3X3_H_

typedef struct mat3x3 {
    float M[3][3];
} mat3x3;

mat3x3 make_mat3x3(float a11, float a12, float a13,
                   float a21, float a22, float a23,
                   float a31, float a32, float a33);
mat3x3 mat3x3_product(mat3x3 m1, mat3x3 m2);
mat3x3 mat3x3_product2(mat3x3 m1, mat3x3 m2, mat3x3 m3);
mat3x3 trans_mat(float x, float y);
mat3x3 rot_mat(float angle);
mat3x3 scale_mat(float factor);

#endif  // MAT3X3_H_
