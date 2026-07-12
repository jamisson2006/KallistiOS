/*
 * vector3d.c — porte fiel de code/Math/Vector3D.java (Quantum Engine J2ME)
 */
#include "vector3d.h"
#include "matrix.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

/* fps.h para QFPS.frameTime */
#include "../utils/fps.h"

extern int MathUtils_ceil(double d);

static inline int qe_abs(int a) { return a < 0 ? -a : a; }

Vector3D *Vector3D_new(void) {
    Vector3D *v = (Vector3D*) calloc(1, sizeof(Vector3D));
    return v;
}
Vector3D *Vector3D_new_xyz(int x, int y, int z) {
    Vector3D *v = (Vector3D*) malloc(sizeof(Vector3D));
    v->x = x; v->y = y; v->z = z;
    return v;
}
Vector3D *Vector3D_new_arr(const int *ps, int len) {
    Vector3D *v = (Vector3D*) malloc(sizeof(Vector3D));
    if (len >= 3) { v->x = ps[0]; v->y = ps[1]; v->z = ps[2]; }
    else          { v->x = v->y = v->z = 0; }
    return v;
}
void Vector3D_free(Vector3D *v) { free(v); }

void Vector3D_set   (Vector3D *self, int x, int y, int z) { self->x=x; self->y=y; self->z=z; }
void Vector3D_setAll(Vector3D *self, int x)                { self->x=self->y=self->z=x; }
void Vector3D_setV  (Vector3D *self, const Vector3D *v)    { self->x=v->x; self->y=v->y; self->z=v->z; }
void Vector3D_add   (Vector3D *self, int x, int y, int z)  { self->x+=x; self->y+=y; self->z+=z; }
void Vector3D_div   (Vector3D *self, int x, int y, int z)  { self->x/=x; self->y/=y; self->z/=z; }
void Vector3D_mul_i (Vector3D *self, int x, int y, int z)  { self->x*=x; self->y*=y; self->z*=z; }

int Vector3D_length       (const Vector3D *self) {
    return (int) sqrt((double)(self->x*self->x + self->y*self->y + self->z*self->z));
}
int Vector3D_lengthSquared(const Vector3D *self) {
    return self->x*self->x + self->y*self->y + self->z*self->z;
}

void Vector3D_setLength(Vector3D *self, int len) {
    if ((self->x*self->x + self->y*self->y + self->z*self->z) != len*len) {
        while (qe_abs(self->x) + qe_abs(self->y) + qe_abs(self->z) > 30000) {
            self->x /= 2; self->y /= 2; self->z /= 2;
        }
        double sqr = sqrt((double)(self->x*self->x + self->y*self->y + self->z*self->z));
        self->x = (int)(self->x / sqr * len);
        self->y = (int)(self->y / sqr * len);
        self->z = (int)(self->z / sqr * len);
    }
}

void Vector3D_setLength2(Vector3D *self, int len) {
    if (len == 0) { self->x = self->y = self->z = 0; return; }
    int a = qe_abs(self->x), b = qe_abs(self->y), c = qe_abs(self->z);
    int oldlen = a > b ? a : b; if (c > oldlen) oldlen = c;
    if (oldlen == 0) { fprintf(stderr, "Cant set vector length: old length is zero\n"); return; }
    self->x = self->x * len / oldlen;
    self->y = self->y * len / oldlen;
    self->z = self->z * len / oldlen;
}

void Vector3D_setLengthRound(Vector3D *self, int len) {
    if ((self->x*self->x + self->y*self->y + self->z*self->z) != len*len) {
        while (qe_abs(self->x) + qe_abs(self->y) + qe_abs(self->z) > 30000) {
            self->x /= 2; self->y /= 2; self->z /= 2;
        }
        double sqr = sqrt((double)(self->x*self->x + self->y*self->y + self->z*self->z));
        self->x = MathUtils_ceil(self->x / sqr * len);
        self->y = MathUtils_ceil(self->y / sqr * len);
        self->z = MathUtils_ceil(self->z / sqr * len);
    }
}

void Vector3D_interpolation(Vector3D *self, const Vector3D *v, int s) {
    int ft = FPS_frameTime == 0 ? 1 : FPS_frameTime;
    self->x += (v->x - self->x) * 50 / s / ft;
    self->y += (v->y - self->y) * 50 / s / ft;
    self->z += (v->z - self->z) * 50 / s / ft;
}

int Vector3D_dot(const Vector3D *self, const Vector3D *v) {
    return self->x*v->x + self->y*v->y + self->z*v->z;
}
long Vector3D_dotLong(const Vector3D *self, const Vector3D *v) {
    return (long)self->x*v->x + self->y*v->y + self->z*v->z;
}

void Vector3D_cross(Vector3D *self, const Vector3D *a, const Vector3D *b) {
    self->x = (int)((float)(a->y * b->z / 32768 - b->y * a->z / 32768));
    self->y = (int)((float)(a->z * b->x / 32768 - b->z * a->x / 32768));
    self->z = (int)((float)(a->x * b->y / 32768 - b->x * a->y / 32768));
}
void Vector3D_crossFP(Vector3D *self, const Vector3D *a, const Vector3D *b, int fp) {
    self->x = (a->y * b->z - b->y * a->z) >> fp;
    self->y = (a->z * b->x - b->z * a->x) >> fp;
    self->z = (a->x * b->y - b->x * a->y) >> fp;
}

void Vector3D_mulMatrix(Vector3D *self, const Matrix *matrix) {
    /* Java Bug-for-bug: usa 'x' recem-atualizado no calculo de y/z e etc. */
    const int m00 = matrix->m00 >> 2;
    const int m01 = matrix->m01 >> 2;
    const int m02 = matrix->m02 >> 2;
    const int m03 = matrix->m03;

    const int m10 = matrix->m10 >> 2;
    const int m11 = matrix->m11 >> 2;
    const int m12 = matrix->m12 >> 2;
    const int m13 = matrix->m13;

    const int m20 = matrix->m20 >> 2;
    const int m21 = matrix->m21 >> 2;
    const int m22 = matrix->m22 >> 2;
    const int m23 = matrix->m23;

    const int fp = QE_MATRIX_FP - 2;

    self->x = m03;
    if (m00 != 0) self->x += (self->x * m00 >> fp);
    if (m01 != 0) self->x += (self->y * m01 >> fp);
    if (m02 != 0) self->x += (self->z * m02 >> fp);

    self->y = -m13;
    if (m10 != 0) self->y -= (self->x * m10 >> fp);
    if (m11 != 0) self->y -= (self->y * m11 >> fp);
    if (m12 != 0) self->y -= (self->z * m12 >> fp);

    self->z = m23;
    if (m20 != 0) self->z += (self->x * m20 >> fp);
    if (m21 != 0) self->z += (self->y * m21 >> fp);
    if (m22 != 0) self->z += (self->z * m22 >> fp);
}

int  Vector3D_average(const Vector3D *self) { return (self->x + self->y + self->z) / 3; }
int  Vector3D_equals (const Vector3D *self, const Vector3D *v) {
    return self->x == v->x && self->y == v->y && self->z == v->z;
}
