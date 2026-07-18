/*
 * vector3d.h — porte fiel de code/Math/Vector3D.java (Quantum Engine J2ME)
 */
#ifndef QE_MATH_VECTOR3D_H
#define QE_MATH_VECTOR3D_H

#include <stdint.h>

typedef struct Matrix Matrix;

typedef struct Vector3D {
    int x, y, z;
} Vector3D;

Vector3D *Vector3D_new(void);
Vector3D *Vector3D_new_xyz(int x, int y, int z);
Vector3D *Vector3D_new_arr(const int *ps, int len);
void      Vector3D_free(Vector3D *v);

/* accessors */
static inline int Vector3D_x(const Vector3D *v) { return v->x; }
static inline int Vector3D_y(const Vector3D *v) { return v->y; }
static inline int Vector3D_z(const Vector3D *v) { return v->z; }

void  Vector3D_set  (Vector3D *self, int x, int y, int z);
void  Vector3D_setAll(Vector3D *self, int x);
void  Vector3D_setV (Vector3D *self, const Vector3D *v);
void  Vector3D_add  (Vector3D *self, int x, int y, int z);
void  Vector3D_div  (Vector3D *self, int x, int y, int z);
void  Vector3D_mul_i(Vector3D *self, int x, int y, int z);

int   Vector3D_length       (const Vector3D *self);
int   Vector3D_lengthSquared(const Vector3D *self);
void  Vector3D_setLength    (Vector3D *self, int len);
void  Vector3D_setLength2   (Vector3D *self, int len);
void  Vector3D_setLengthRound(Vector3D *self, int len);

void  Vector3D_interpolation(Vector3D *self, const Vector3D *v, int s);

int   Vector3D_dot     (const Vector3D *self, const Vector3D *v);
long  Vector3D_dotLong (const Vector3D *self, const Vector3D *v);

void  Vector3D_cross   (Vector3D *self, const Vector3D *a, const Vector3D *b);
void  Vector3D_crossFP (Vector3D *self, const Vector3D *a, const Vector3D *b, int fp);

/* Multiplica por matriz (fiel ao bug do original que sobrescreve x/y/z durante). */
void  Vector3D_mulMatrix(Vector3D *self, const Matrix *m);

int   Vector3D_average (const Vector3D *self);
int   Vector3D_equals  (const Vector3D *self, const Vector3D *v);

#endif
