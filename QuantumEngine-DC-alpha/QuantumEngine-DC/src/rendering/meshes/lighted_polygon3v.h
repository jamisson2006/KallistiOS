/*
 * lighted_polygon3v.h — porte fiel de code/Rendering/Meshes/LightedPolygon3V.java
 * @author Roman Lahin
 */
#ifndef QE_RENDERING_MESHES_LIGHTED_POLYGON3V_H
#define QE_RENDERING_MESHES_LIGHTED_POLYGON3V_H

#include "polygon3v.h"

typedef struct LightedPolygon3V {
    Polygon3V base;
    int la, lb, lc;
} LightedPolygon3V;

typedef struct ColorLightedPolygon3V ColorLightedPolygon3V;

LightedPolygon3V *LightedPolygon3V_new(Vertex *a, Vertex *b, Vertex *c,
                                        int8_t au, int8_t av, int8_t bu, int8_t bv, int8_t cu, int8_t cv);
LightedPolygon3V *LightedPolygon3V_new_copy(const LightedPolygon3V *p);
LightedPolygon3V *LightedPolygon3V_new_fromColor(const ColorLightedPolygon3V *p);
void              LightedPolygon3V_free(LightedPolygon3V *self);

const RenderObjectVTable *LightedPolygon3V_vt(void);

#endif
