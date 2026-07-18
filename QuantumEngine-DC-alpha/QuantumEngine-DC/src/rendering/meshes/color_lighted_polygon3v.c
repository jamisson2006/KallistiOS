/*
 * color_lighted_polygon3v.c — porte fiel de code/Rendering/Meshes/ColorLightedPolygon3V.java
 */
#include "color_lighted_polygon3v.h"
#include "../vertex.h"
#include "../directx7.h"
#include "../texture.h"
#include "../raw_image.h"
#include "../texturing_affine.h"
#include "../texturing_pers.h"
#include "../renderobject.h"

#include "../../math/math_utils.h"
#include "../../utils/main.h"

#include <stdlib.h>

static int q_g, sz2_g, sizex_g, sizey_g, sizeu_g, sizev_g, addmip_g, fog_g;

static int size3(int a, int b, int c) {
    int t;
    if (b < a) { t=a; a=b; b=t; }
    if (c < a) { t=c; c=a; a=t; }
    if (c < b) { t=b; b=c; c=t; }
    return c - a;
}

static void clp3v_renderFast(RenderObject *ro, DirectX7 *g3d, Texture *tex) {
    ColorLightedPolygon3V *self = (ColorLightedPolygon3V*) ro;
    Polygon3V *p = (Polygon3V*) ro;
    if (tex->drawmode == 5) {
        sz2_g = 0xff - MathUtils_calcLight(ro->nx, ro->ny, ro->nz,
            DirectX7_lightdirx, DirectX7_lightdiry, DirectX7_lightdirz);
        if (sz2_g < 0) sz2_g = 0;
    }
    if (tex->drawmode == 1) if (fog_g == 1) Polygon3V_szCalcfogAdd(p);

    TexturingAffine_paint(g3d, tex,
        p->a, p->au, p->av, p->b, p->bu, p->bv, p->c, p->cu, p->cv,
        DirectX7_fogc, tex->drawmode, sz2_g,
        self->ar, self->br, self->cr,
        self->ag, self->bg, self->cg,
        self->ab, self->bb, self->cb,
        ro->nx, ro->ny, ro->nz);
}

static void clp3v_render(RenderObject *ro, DirectX7 *g3d, Texture *tex) {
    ColorLightedPolygon3V *self = (ColorLightedPolygon3V*) ro;
    Polygon3V *p = (Polygon3V*) ro;
    if (Main_persQ == 0 && Main_mipMapping == 0) { clp3v_renderFast(ro, g3d, tex); return; }
    fog_g = tex->drawmode;

    sizex_g = size3(p->a->sx, p->b->sx, p->c->sx);
    sizey_g = size3(p->a->sy, p->b->sy, p->c->sy);

    int al[3], bl[3], cl[3];
    RenderObject_getLight3(ro, p->a, self->ar, self->ag, self->ab, g3d, al);
    RenderObject_getLight3(ro, p->b, self->br, self->bg, self->bb, g3d, bl);
    RenderObject_getLight3(ro, p->c, self->cr, self->cg, self->cb, g3d, cl);

    if (fog_g == 5)
        sz2_g = 0xff - MathUtils_calcLight(ro->nx, ro->ny, ro->nz,
            DirectX7_lightdirx, DirectX7_lightdiry, DirectX7_lightdirz);
    if (fog_g == 1) Polygon3V_szCalcfogAdd(p);

    q_g = (ro->ny > 4000 || ro->ny < -4000) ? 9999999 : Main_q;

    if (Main_mipMapping && tex->mip != NULL) {
        tex->rImg = tex->mip[0];
        if (tex->drawmode < 10 || tex->drawmode > 12) {
            addmip_g = q_g > 999 ? 2 : 3;
            sizeu_g = size3(p->au, p->bu, p->cu) * tex->rImg->w >> 8;
            sizev_g = size3(p->av, p->bv, p->cv) * tex->rImg->h >> 8;
            if (((sizex_g + sizey_g) >> 1) * addmip_g < sizeu_g + sizev_g) {
                tex->rImg = tex->mip[1];
                if (tex->mip != NULL) {
                    sizeu_g >>= 1; sizev_g >>= 1;
                    if (((sizex_g + sizey_g) >> 1) * addmip_g < sizeu_g + sizev_g)
                        tex->rImg = tex->mip[2];
                }
            }
        } else if (sizex_g < tex->rImg->w || sizey_g < tex->rImg->w) {
            tex->rImg = tex->mip[1];
            if (tex->mip != NULL && (sizex_g < tex->rImg->w || sizey_g < tex->rImg->w))
                tex->rImg = tex->mip[2];
        }
    }

    if ((tex->perspectiveCorrection
         && (sizex_g > 30 - (Main_persQ < 3 ? 0 : 15) || sizey_g > 30 - (Main_persQ < 3 ? 0 : 15)))
        && Main_persQ != 0 || Main_persQ == 4) {
        TexturingPers_paint(g3d, tex, p->a, p->au, p->av, p->b, p->bu, p->bv, p->c, p->cu, p->cv,
            DirectX7_fogc, fog_g, sz2_g, q_g,
            al[0], bl[0], cl[0], al[1], bl[1], cl[1], al[2], bl[2], cl[2],
            ro->nx, ro->ny, ro->nz);
    } else {
        TexturingAffine_paint(g3d, tex, p->a, p->au, p->av, p->b, p->bu, p->bv, p->c, p->cu, p->cv,
            DirectX7_fogc, fog_g, sz2_g,
            al[0], bl[0], cl[0], al[1], bl[1], cl[1], al[2], bl[2], cl[2],
            ro->nx, ro->ny, ro->nz);
    }
}

static int clp3v_isVisible(RenderObject *ro, int x1, int y1, int x2, int y2) {
    return Polygon3V_vt()->isVisible(ro, x1, y1, x2, y2);
}
static void clp3v_destroy(RenderObject *ro) { ColorLightedPolygon3V_free((ColorLightedPolygon3V*) ro); }

static const RenderObjectVTable g_clp3v_vt = { clp3v_render, clp3v_renderFast, clp3v_isVisible, clp3v_destroy };
const RenderObjectVTable *ColorLightedPolygon3V_vt(void) { return &g_clp3v_vt; }

ColorLightedPolygon3V *ColorLightedPolygon3V_new(Vertex *a, Vertex *b, Vertex *c,
    int8_t au, int8_t av, int8_t bu, int8_t bv, int8_t cu, int8_t cv) {
    ColorLightedPolygon3V *cp = (ColorLightedPolygon3V*) calloc(1, sizeof(ColorLightedPolygon3V));
    Polygon3V *p3 = &cp->base;
    RenderObject_initWithNormals(&p3->base, &g_clp3v_vt, a, b, c);
    p3->a=a; p3->au=au&0xff; p3->av=av&0xff;
    p3->b=b; p3->bu=bu&0xff; p3->bv=bv&0xff;
    p3->c=c; p3->cu=cu&0xff; p3->cv=cv&0xff;
    cp->ar=cp->ag=cp->ab=cp->br=cp->bg=cp->bb=cp->cr=cp->cg=cp->cb=255;
    return cp;
}

ColorLightedPolygon3V *ColorLightedPolygon3V_new_copy(const ColorLightedPolygon3V *o) {
    ColorLightedPolygon3V *cp = (ColorLightedPolygon3V*) calloc(1, sizeof(ColorLightedPolygon3V));
    cp->base = o->base;
    cp->base.base.vt = &g_clp3v_vt;
    cp->ar=o->ar; cp->ag=o->ag; cp->ab=o->ab;
    cp->br=o->br; cp->bg=o->bg; cp->bb=o->bb;
    cp->cr=o->cr; cp->cg=o->cg; cp->cb=o->cb;
    return cp;
}

void ColorLightedPolygon3V_free(ColorLightedPolygon3V *self) { free(self); }
