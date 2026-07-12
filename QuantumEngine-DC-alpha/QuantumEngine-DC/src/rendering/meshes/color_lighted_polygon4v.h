#ifndef QE_RENDERING_MESHES_COLOR_LIGHTED_POLYGON4V_H
#define QE_RENDERING_MESHES_COLOR_LIGHTED_POLYGON4V_H
#include "polygon4v.h"

typedef struct ColorLightedPolygon4V {
    Polygon4V base;
    int ar, ag, ab;
    int br, bg, bb;
    int cr, cg, cb;
    int dr, dg, db;
} ColorLightedPolygon4V;

ColorLightedPolygon4V *ColorLightedPolygon4V_new(Vertex *a, Vertex *b, Vertex *c, Vertex *d,
    int8_t au, int8_t av, int8_t bu, int8_t bv, int8_t cu, int8_t cv, int8_t du, int8_t dv);
ColorLightedPolygon4V *ColorLightedPolygon4V_new_copy(const ColorLightedPolygon4V *p);
void                   ColorLightedPolygon4V_free(ColorLightedPolygon4V *self);
const RenderObjectVTable *ColorLightedPolygon4V_vt(void);

#endif
