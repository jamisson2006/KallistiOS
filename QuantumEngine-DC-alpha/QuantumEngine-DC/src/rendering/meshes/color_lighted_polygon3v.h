#ifndef QE_RENDERING_MESHES_COLOR_LIGHTED_POLYGON3V_H
#define QE_RENDERING_MESHES_COLOR_LIGHTED_POLYGON3V_H
#include "polygon3v.h"

typedef struct ColorLightedPolygon3V {
    Polygon3V base;
    int ar, ag, ab;
    int br, bg, bb;
    int cr, cg, cb;
} ColorLightedPolygon3V;

ColorLightedPolygon3V *ColorLightedPolygon3V_new(Vertex *a, Vertex *b, Vertex *c,
    int8_t au, int8_t av, int8_t bu, int8_t bv, int8_t cu, int8_t cv);
ColorLightedPolygon3V *ColorLightedPolygon3V_new_copy(const ColorLightedPolygon3V *p);
void                   ColorLightedPolygon3V_free(ColorLightedPolygon3V *self);
const RenderObjectVTable *ColorLightedPolygon3V_vt(void);

#endif
