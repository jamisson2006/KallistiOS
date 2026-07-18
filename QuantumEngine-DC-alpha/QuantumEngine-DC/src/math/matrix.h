/*
 * matrix.h — porte fiel de code/Math/Matrix.java (Quantum Engine J2ME)
 *
 * Matriz 3x4 (linha 3 implicita: 0 0 0 FP) em fixed-point 14.14.
 */
#ifndef QE_MATH_MATRIX_H
#define QE_MATH_MATRIX_H

#include <stdint.h>

#define QE_MATRIX_FP  14
#define QE_MATRIX_FP_ONE (1 << QE_MATRIX_FP)

typedef struct Vector3D Vector3D;

typedef struct Matrix {
    int m00, m01, m02, m03;
    int m10, m11, m12, m13;
    int m20, m21, m22, m23;
    int m30, m31, m32, m33;
} Matrix;

/* Java: static { ... } — chame antes de qualquer sin/cos/rot. */
void   Matrix_initTables(void);

/* Ctor: aloca e faz setIdentity. */
Matrix *Matrix_new(void);
void    Matrix_free(Matrix *m);

void    Matrix_setIdentity(Matrix *self);
void    Matrix_set_m(Matrix *self, const Matrix *m);
void    Matrix_set(Matrix *self,
                   int m00,int m01,int m02,int m03,
                   int m10,int m11,int m12,int m13,
                   int m20,int m21,int m22,int m23);

void    Matrix_getPosition(const Matrix *self, Vector3D *v);
void    Matrix_invert(Matrix *self);

void    Matrix_setRotX(Matrix *self, int degree);
void    Matrix_setRotY(Matrix *self, int degree);
void    Matrix_setRotZ(Matrix *self, int degree);
void    Matrix_rotY  (Matrix *self, int degree);
void    Matrix_rotZ  (Matrix *self, int degree);

/* mul: self = self * m  |  mul2: self = m2 * m1 */
void    Matrix_mul  (Matrix *self, const Matrix *m);
void    Matrix_mul2 (Matrix *self, const Matrix *m2, const Matrix *m1);

void    Matrix_setPosition (Matrix *self, int x, int y, int z);
void    Matrix_setPositionV(Matrix *self, const Vector3D *pos);
void    Matrix_addPositionV(Matrix *self, const Vector3D *pos);
void    Matrix_divPosition (Matrix *self, int x, int y, int z);
void    Matrix_subPosition (Matrix *self, int x, int y, int z);
void    Matrix_addPosition (Matrix *self, int x, int y, int z);

void    Matrix_setSide (Matrix *self, int x, int y, int z);
void    Matrix_setUp   (Matrix *self, int x, int y, int z);
void    Matrix_setDir  (Matrix *self, int x, int y, int z);

void    Matrix_scale     (Matrix *self, int x, int y, int z);
void    Matrix_translate (Matrix *self, int x, int y, int z);

int     Matrix_cos(int deg);
int     Matrix_sin(int deg);

int     Matrix_getRotZ  (const Matrix *self);
int     Matrix_getRotZHQ(const Matrix *self);

#endif
