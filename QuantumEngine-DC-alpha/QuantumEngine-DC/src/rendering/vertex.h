/*
 * vertex.h — porte fiel de code/Rendering/Vertex.java + VertexNormal.java
 */
#ifndef QE_RENDERING_VERTEX_H
#define QE_RENDERING_VERTEX_H

#include "../math/matrix.h"

typedef struct DirectX7 DirectX7;

typedef struct Vertex {
    int x, y, z;
    int sx, sy, rz;
} Vertex;

/* VertexNormal extends Vertex — mesmo layout. */
typedef Vertex VertexNormal;

Vertex *Vertex_new(void);
Vertex *Vertex_new_xyz(int x, int y, int z);
Vertex *Vertex_new_copy(const Vertex *ver);
Vertex *Vertex_new_arr(const int *ver);
void    Vertex_free(Vertex *v);

/* Getters usados por outros modulos (Math etc.). */
static inline int Vertex_x(const Vertex *v) { return v->x; }
static inline int Vertex_y(const Vertex *v) { return v->y; }
static inline int Vertex_z(const Vertex *v) { return v->z; }

void   Vertex_set    (Vertex *self, int x, int y, int z);
void   Vertex_setV   (Vertex *self, const Vertex *ver);
void   Vertex_pmax   (Vertex *self, int x, int y, int z);
void   Vertex_min    (Vertex *self, int x, int y, int z);
void   Vertex_mul_i  (Vertex *self, int x, int y, int z);
void   Vertex_sub    (Vertex *self, int x, int y, int z);
void   Vertex_add    (Vertex *self, int x, int y, int z);
void   Vertex_div_i  (Vertex *self, int x, int y, int z);
void   Vertex_transform  (Vertex *self, const Matrix *matrix);
void   Vertex_transformFE(Vertex *self, const Matrix *matrix);
void   Vertex_project    (Vertex *self, const DirectX7 *g3d);

#endif
