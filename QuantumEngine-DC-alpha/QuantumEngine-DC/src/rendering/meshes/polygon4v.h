/*
 * polygon4v.h — porte fiel de code/Rendering/Meshes/Polygon4V.java
 */
#ifndef QE_RENDERING_MESHES_POLYGON4V_H
#define QE_RENDERING_MESHES_POLYGON4V_H

#include "../renderobject.h"
#include <stdint.h>

typedef struct Vertex   Vertex;
typedef struct DirectX7 DirectX7;
typedef struct Texture  Texture;

typedef struct Polygon4V {
    RenderObject base;
    Vertex *a, *b, *c, *d;
    int au, av, bu, bv, cu, cv, du, dv;
    int tex;
} Polygon4V;

Polygon4V *Polygon4V_new(Vertex *a, Vertex *b, Vertex *c, Vertex *d,
                          int8_t au, int8_t av, int8_t bu, int8_t bv,
                          int8_t cu, int8_t cv, int8_t du, int8_t dv);
Polygon4V *Polygon4V_new_copy(const Polygon4V *p);
void       Polygon4V_free(Polygon4V *self);

const RenderObjectVTable *Polygon4V_vt(void);

void Polygon4V_szCalcfogAdd(Polygon4V *self);

#endif
