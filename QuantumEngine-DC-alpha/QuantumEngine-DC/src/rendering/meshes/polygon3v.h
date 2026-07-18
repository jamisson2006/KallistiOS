/*
 * polygon3v.h — porte fiel de code/Rendering/Meshes/Polygon3V.java
 */
#ifndef QE_RENDERING_MESHES_POLYGON3V_H
#define QE_RENDERING_MESHES_POLYGON3V_H

#include "../renderobject.h"
#include <stdint.h>

typedef struct Vertex   Vertex;
typedef struct DirectX7 DirectX7;
typedef struct Texture  Texture;

typedef struct Polygon3V {
    RenderObject base;
    Vertex *a, *b, *c;
    int  au, av, bu, bv, cu, cv;
    int  tex;   /* Java: byte tex=0 */
} Polygon3V;

Polygon3V *Polygon3V_new(Vertex *a, Vertex *b, Vertex *c,
                          int8_t au, int8_t av, int8_t bu, int8_t bv, int8_t cu, int8_t cv);
Polygon3V *Polygon3V_new_copy(const Polygon3V *p);
void       Polygon3V_free(Polygon3V *self);

const RenderObjectVTable *Polygon3V_vt(void);

/* helpers */
void Polygon3V_szCalcfogAdd(Polygon3V *self);

#endif
