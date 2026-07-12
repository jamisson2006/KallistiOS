/*
 * vertex.c — porte fiel de code/Rendering/Vertex.java + VertexNormal.java
 */
#include "vertex.h"
#include "directx7.h"

#include <stdlib.h>

Vertex *Vertex_new(void) { return (Vertex*) calloc(1, sizeof(Vertex)); }
Vertex *Vertex_new_xyz(int x, int y, int z) {
    Vertex *v = (Vertex*) calloc(1, sizeof(Vertex));
    v->x = x; v->y = y; v->z = z;
    return v;
}
Vertex *Vertex_new_copy(const Vertex *o) {
    Vertex *v = (Vertex*) malloc(sizeof(Vertex));
    *v = *o;
    return v;
}
Vertex *Vertex_new_arr(const int *ver) {
    Vertex *v = (Vertex*) calloc(1, sizeof(Vertex));
    v->x = ver[0]; v->y = ver[1]; v->z = ver[2];
    return v;
}
void Vertex_free(Vertex *v) { free(v); }

void Vertex_set (Vertex *s, int x, int y, int z) { s->x=x; s->y=y; s->z=z; }
void Vertex_setV(Vertex *s, const Vertex *o)     { *s = *o; }

static inline int qe_max(int a, int b) { return a > b ? a : b; }
static inline int qe_min(int a, int b) { return a < b ? a : b; }

void Vertex_pmax (Vertex *s, int x, int y, int z) { s->sx = qe_max(x, s->sx); s->sy = qe_max(y, s->sy); s->rz = qe_max(z, s->rz); }
void Vertex_min  (Vertex *s, int x, int y, int z) { s->x  = qe_min(x, s->x);  s->y  = qe_min(y, s->y);  s->z  = qe_min(z, s->z); }
void Vertex_mul_i(Vertex *s, int x, int y, int z) { s->x *= x; s->y *= y; s->z *= z; }
void Vertex_sub  (Vertex *s, int x, int y, int z) { s->x -= x; s->y -= y; s->z -= z; }
void Vertex_add  (Vertex *s, int x, int y, int z) { s->x += x; s->y += y; s->z += z; }
void Vertex_div_i(Vertex *s, int x, int y, int z) {
    if (x != 0) s->x /= x;
    if (y != 0) s->y /= y;
    if (z != 0) s->z /= z;
}

void Vertex_transform(Vertex *self, const Matrix *matrix) {
    int fpss = QE_MATRIX_FP - 2;
    int m00 = matrix->m00 >> 2, m01 = matrix->m01 >> 2, m02 = matrix->m02 >> 2, m03 = matrix->m03;
    int m10 = matrix->m10 >> 2, m11 = matrix->m11 >> 2, m12 = matrix->m12 >> 2, m13 = matrix->m13;
    int m20 = matrix->m20 >> 2, m21 = matrix->m21 >> 2, m22 = matrix->m22 >> 2, m23 = matrix->m23;
    self->sx = ((self->x * m00 >> fpss) + (self->y * m01 >> fpss) + (self->z * m02 >> fpss) + m03);
    self->sy = ((self->x * m10 >> fpss) + (self->y * m11 >> fpss) + (self->z * m12 >> fpss) + m13);
    self->rz = ((self->x * m20 >> fpss) + (self->y * m21 >> fpss) + (self->z * m22 >> fpss) + m23);
}

void Vertex_transformFE(Vertex *self, const Matrix *matrix) {
    /* Bug-for-bug: sobrescreve x/y/z durante o calculo (Java preserva
     * este comportamento — vertex.x atualizado antes de calcular vertex.y). */
    int fpss = QE_MATRIX_FP - 2;
    int m00=matrix->m00>>2, m01=matrix->m01>>2, m02=matrix->m02>>2, m03=matrix->m03;
    int m10=matrix->m10>>2, m11=matrix->m11>>2, m12=matrix->m12>>2, m13=matrix->m13;
    int m20=matrix->m20>>2, m21=matrix->m21>>2, m22=matrix->m22>>2, m23=matrix->m23;
    self->sx = self->x; self->sy = self->y; self->rz = self->z;
    self->x = (self->x * m00 >> fpss) + (self->y * m01 >> fpss) + (self->z * m02 >> fpss) + m03;
    self->y = (self->x * m10 >> fpss) + (self->y * m11 >> fpss) + (self->z * m12 >> fpss) + m13;
    self->z = (self->x * m20 >> fpss) + (self->y * m21 >> fpss) + (self->z * m22 >> fpss) + m23;
}

void Vertex_project(Vertex *self, const DirectX7 *g3d) {
    if (self->rz <= 0) {
        self->sx =  self->sx * g3d->distX / (-self->rz + g3d->distX) + g3d->centreX;
        self->sy = -self->sy * g3d->distY / (-self->rz + g3d->distY) + g3d->centreY;
    } else {
        self->sx += g3d->centreX;
        self->sy = -self->sy + g3d->centreY;
    }
}
