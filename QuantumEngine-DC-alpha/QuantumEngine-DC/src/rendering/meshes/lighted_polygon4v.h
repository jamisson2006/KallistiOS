#ifndef QE_RENDERING_MESHES_LIGHTED_POLYGON4V_H
#define QE_RENDERING_MESHES_LIGHTED_POLYGON4V_H
#include "polygon4v.h"

typedef struct LightedPolygon4V {
    Polygon4V base;
    int la, lb, lc, ld;
} LightedPolygon4V;

typedef struct ColorLightedPolygon4V ColorLightedPolygon4V;

LightedPolygon4V *LightedPolygon4V_new(Vertex *a, Vertex *b, Vertex *c, Vertex *d,
    int8_t au, int8_t av, int8_t bu, int8_t bv, int8_t cu, int8_t cv, int8_t du, int8_t dv);
LightedPolygon4V *LightedPolygon4V_new_copy(const LightedPolygon4V *p);
LightedPolygon4V *LightedPolygon4V_new_fromColor(const ColorLightedPolygon4V *p);
void              LightedPolygon4V_free(LightedPolygon4V *self);
const RenderObjectVTable *LightedPolygon4V_vt(void);

#endif
