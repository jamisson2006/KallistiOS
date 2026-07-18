/*
 * matrix.c — porte fiel de code/Math/Matrix.java (Quantum Engine J2ME)
 */
#include "matrix.h"
#include "vector3d.h"

#include <math.h>
#include <stdlib.h>

/* Java: static final short[] sin/cos = new short[360] */
static int16_t g_sin[360];
static int16_t g_cos[360];
static int     g_tables_init = 0;

void Matrix_initTables(void) {
    if (g_tables_init) return;
    for (int i = 0; i < 360; i++) {
        double rad = i * (3.14159265358979323846 / 180.0);
        g_sin[i] = (int16_t)(sin(rad) * QE_MATRIX_FP_ONE);
        g_cos[i] = (int16_t)(cos(rad) * QE_MATRIX_FP_ONE);
    }
    g_tables_init = 1;
}

static int qe_fixDegree(int degree) {
    while (degree < 0)    degree += 360;
    while (degree >= 360) degree -= 360;
    return degree;
}

Matrix *Matrix_new(void) {
    Matrix *m = (Matrix*) malloc(sizeof(Matrix));
    Matrix_setIdentity(m);
    m->m30 = 0; m->m31 = 0; m->m32 = 0; m->m33 = QE_MATRIX_FP_ONE;
    return m;
}
void Matrix_free(Matrix *m) { free(m); }

void Matrix_setIdentity(Matrix *self) {
    self->m00 = QE_MATRIX_FP_ONE; self->m01 = 0; self->m02 = 0; self->m03 = 0;
    self->m10 = 0; self->m11 = QE_MATRIX_FP_ONE; self->m12 = 0; self->m13 = 0;
    self->m20 = 0; self->m21 = 0; self->m22 = QE_MATRIX_FP_ONE; self->m23 = 0;
}

void Matrix_set_m(Matrix *self, const Matrix *m) {
    self->m00=m->m00; self->m01=m->m01; self->m02=m->m02; self->m03=m->m03;
    self->m10=m->m10; self->m11=m->m11; self->m12=m->m12; self->m13=m->m13;
    self->m20=m->m20; self->m21=m->m21; self->m22=m->m22; self->m23=m->m23;
}

void Matrix_set(Matrix *self,
                int m00,int m01,int m02,int m03,
                int m10,int m11,int m12,int m13,
                int m20,int m21,int m22,int m23) {
    self->m00=m00; self->m01=m01; self->m02=m02; self->m03=m03;
    self->m10=m10; self->m11=m11; self->m12=m12; self->m13=m13;
    self->m20=m20; self->m21=m21; self->m22=m22; self->m23=m23;
}

void Matrix_getPosition(const Matrix *self, Vector3D *v) {
    Vector3D_set(v, self->m03, self->m13, self->m23);
}

void Matrix_invert(Matrix *self) {
    int fp = QE_MATRIX_FP;
    long v1 = ((long)self->m00 * (long)self->m03 >> fp) + ((long)self->m10 * (long)self->m13 >> fp) + ((long)self->m20 * (long)self->m23 >> fp);
    long v3 = ((long)self->m01 * (long)self->m03 >> fp) + ((long)self->m11 * (long)self->m13 >> fp) + ((long)self->m21 * (long)self->m23 >> fp);
    long v5 = ((long)self->m02 * (long)self->m03 >> fp) + ((long)self->m12 * (long)self->m13 >> fp) + ((long)self->m22 * (long)self->m23 >> fp);
    self->m03 = (int)(-v1);
    self->m13 = (int)(-v3);
    self->m23 = (int)(-v5);
    int t = self->m01; self->m01 = self->m10; self->m10 = t;
    t = self->m02; self->m02 = self->m20; self->m20 = t;
    t = self->m12; self->m12 = self->m21; self->m21 = t;
}

void Matrix_setRotX(Matrix *self, int degree) {
    degree = qe_fixDegree(degree);
    self->m00 = QE_MATRIX_FP_ONE; self->m01 = 0;              self->m02 = 0;              self->m03 = 0;
    self->m10 = 0;                self->m11 = g_cos[degree];  self->m12 = -g_sin[degree]; self->m13 = 0;
    self->m20 = 0;                self->m21 = g_sin[degree];  self->m22 = g_cos[degree];  self->m23 = 0;
}

void Matrix_setRotY(Matrix *self, int degree) {
    degree = qe_fixDegree(degree);
    self->m00 = g_cos[degree];  self->m01 = 0; self->m02 = g_sin[degree];  self->m03 = 0;
    self->m10 = 0;              self->m11 = QE_MATRIX_FP_ONE; self->m12 = 0; self->m13 = 0;
    self->m20 = -g_sin[degree]; self->m21 = 0; self->m22 = g_cos[degree];  self->m23 = 0;
}

void Matrix_setRotZ(Matrix *self, int degree) {
    degree = qe_fixDegree(degree);
    self->m00 = g_cos[degree]; self->m01 = -g_sin[degree]; self->m02 = 0; self->m03 = 0;
    self->m10 = g_sin[degree]; self->m11 =  g_cos[degree]; self->m12 = 0; self->m13 = 0;
    self->m20 = 0;              self->m21 = 0;              self->m22 = QE_MATRIX_FP_ONE; self->m23 = 0;
}

void Matrix_rotY(Matrix *self, int degree) {
    int fp = QE_MATRIX_FP;
    degree = qe_fixDegree(degree);
    int c = g_cos[degree], s = g_sin[degree];

    int xX = (self->m00 * c + self->m20 * s) >> fp;
    int xY = (self->m01 * c + self->m21 * s) >> fp;
    int xZ = (self->m02 * c + self->m22 * s) >> fp;

    int zX = (self->m20 * c - self->m00 * s) >> fp;
    int zY = (self->m21 * c - self->m01 * s) >> fp;
    int zZ = (self->m22 * c - self->m02 * s) >> fp;

    self->m00 = xX; self->m01 = xY; self->m02 = xZ;
    self->m20 = zX; self->m21 = zY; self->m22 = zZ;
}

void Matrix_rotZ(Matrix *self, int degree) {
    int fp = QE_MATRIX_FP;
    degree = qe_fixDegree(degree);
    int c = g_cos[degree], s = g_sin[degree];

    int yX = (self->m10 * c + self->m00 * s) >> fp;
    int yY = (self->m11 * c + self->m01 * s) >> fp;
    int yZ = (self->m12 * c + self->m02 * s) >> fp;

    int xX = (self->m00 * c - self->m10 * s) >> fp;
    int xY = (self->m01 * c - self->m11 * s) >> fp;
    int xZ = (self->m02 * c - self->m12 * s) >> fp;

    self->m10 = yX; self->m11 = yY; self->m12 = yZ;
    self->m00 = xX; self->m01 = xY; self->m02 = xZ;
}

void Matrix_mul(Matrix *self, const Matrix *m) { Matrix_mul2(self, self, m); }

void Matrix_mul2(Matrix *self, const Matrix *m2, const Matrix *m1) {
    int fp = QE_MATRIX_FP;
    long t00 = (m2->m00*m1->m00       +       m2->m01*m1->m10 +       m2->m02*m1->m20 + (long)m2->m03*m1->m30) >> fp;
    long t01 = (m2->m00*m1->m01       +       m2->m01*m1->m11 +       m2->m02*m1->m21 + (long)m2->m03*m1->m31) >> fp;
    long t02 = (m2->m00*m1->m02       +       m2->m01*m1->m12 +       m2->m02*m1->m22 + (long)m2->m03*m1->m32) >> fp;
    long t03 = ((long)m2->m00*m1->m03 + (long)m2->m01*m1->m13 + (long)m2->m02*m1->m23 + (long)m2->m03*m1->m33) >> fp;

    long t10 = (m2->m10*m1->m00       +       m2->m11*m1->m10 +       m2->m12*m1->m20 + (long)m2->m13*m1->m30) >> fp;
    long t11 = (m2->m10*m1->m01       +       m2->m11*m1->m11 +       m2->m12*m1->m21 + (long)m2->m13*m1->m31) >> fp;
    long t12 = (m2->m10*m1->m02       +       m2->m11*m1->m12 +       m2->m12*m1->m22 + (long)m2->m13*m1->m32) >> fp;
    long t13 = ((long)m2->m10*m1->m03 + (long)m2->m11*m1->m13 + (long)m2->m12*m1->m23 + (long)m2->m13*m1->m33) >> fp;

    long t20 = (m2->m20*m1->m00       +       m2->m21*m1->m10 +       m2->m22*m1->m20 + (long)m2->m23*m1->m30) >> fp;
    long t21 = (m2->m20*m1->m01       +       m2->m21*m1->m11 +       m2->m22*m1->m21 + (long)m2->m23*m1->m31) >> fp;
    long t22 = (m2->m20*m1->m02       +       m2->m21*m1->m12 +       m2->m22*m1->m22 + (long)m2->m23*m1->m32) >> fp;
    long t23 = ((long)m2->m20*m1->m03 + (long)m2->m21*m1->m13 + (long)m2->m22*m1->m23 + (long)m2->m23*m1->m33) >> fp;

    self->m00 = (int)t00; self->m01 = (int)t01; self->m02 = (int)t02; self->m03 = (int)t03;
    self->m10 = (int)t10; self->m11 = (int)t11; self->m12 = (int)t12; self->m13 = (int)t13;
    self->m20 = (int)t20; self->m21 = (int)t21; self->m22 = (int)t22; self->m23 = (int)t23;
}

void Matrix_setPosition (Matrix *self, int x, int y, int z) { self->m03=x; self->m13=y; self->m23=z; }
void Matrix_setPositionV(Matrix *self, const Vector3D *p) { self->m03=Vector3D_x(p); self->m13=Vector3D_y(p); self->m23=Vector3D_z(p); }
void Matrix_addPositionV(Matrix *self, const Vector3D *p) { self->m03+=Vector3D_x(p); self->m13+=Vector3D_y(p); self->m23+=Vector3D_z(p); }
void Matrix_divPosition (Matrix *self, int x, int y, int z) { self->m03/=x; self->m13/=y; self->m23/=z; }
void Matrix_subPosition (Matrix *self, int x, int y, int z) { self->m03-=x; self->m13-=y; self->m23-=z; }
void Matrix_addPosition (Matrix *self, int x, int y, int z) { self->m03+=x; self->m13+=y; self->m23+=z; }

void Matrix_setSide(Matrix *self, int x, int y, int z) { self->m00=x; self->m10=y; self->m20=z; }
void Matrix_setUp  (Matrix *self, int x, int y, int z) { self->m01=x; self->m11=y; self->m21=z; }
void Matrix_setDir (Matrix *self, int x, int y, int z) { self->m02=x; self->m12=y; self->m22=z; }

void Matrix_scale(Matrix *self, int x, int y, int z) {
    int fp = QE_MATRIX_FP;
    self->m00 = self->m00 * x >> fp;
    self->m10 = self->m10 * x >> fp;
    self->m20 = self->m20 * x >> fp;

    self->m01 = self->m01 * y >> fp;
    self->m11 = self->m11 * y >> fp;
    self->m21 = self->m21 * y >> fp;

    self->m02 = self->m02 * z >> fp;
    self->m12 = self->m12 * z >> fp;
    self->m22 = self->m22 * z >> fp;
}

void Matrix_translate(Matrix *self, int x, int y, int z) {
    int fp = QE_MATRIX_FP;
    if (x != 0) {
        self->m03 += self->m00 * x >> fp;
        self->m13 += self->m10 * x >> fp;
        self->m23 += self->m20 * x >> fp;
    }
    if (y != 0) {
        self->m03 += self->m01 * y >> fp;
        self->m13 += self->m11 * y >> fp;
        self->m23 += self->m21 * y >> fp;
    }
    if (z != 0) {
        self->m03 += self->m02 * z >> fp;
        self->m13 += self->m12 * z >> fp;
        self->m23 += self->m22 * z >> fp;
    }
}

int Matrix_cos(int deg) {
    while (deg < 0) deg += 360;
    return g_cos[deg % 360];
}
int Matrix_sin(int deg) {
    while (deg < 0) deg += 360;
    return g_sin[deg % 360];
}

/* Forward declaracao — implementado em math_utils.c */
extern int MathUtils_getAnglez(int x, int y, int x1, int y1);
extern int MathUtils_getAnglezHQ(int x, int y, int x1, int y1);
extern int MathUtils_fixDegree(int degree);

int Matrix_getRotZ(const Matrix *self) {
    int rt = MathUtils_getAnglez(0, 0, -self->m02, -self->m22);
    return MathUtils_fixDegree(rt);
}
int Matrix_getRotZHQ(const Matrix *self) {
    return MathUtils_getAnglezHQ(0, 0, -self->m02, -self->m22);
}
