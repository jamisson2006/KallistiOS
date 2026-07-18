/*
 * lighted_polygon3v.c — porte fiel de code/Rendering/Meshes/LightedPolygon3V.java
 */
#include "lighted_polygon3v.h"
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

static void lp3v_renderFast(RenderObject *ro, DirectX7 *g3d, Texture *tex) {
    LightedPolygon3V *self = (LightedPolygon3V*) ro;
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
        self->la, self->lb, self->lc,
        self->la, self->lb, self->lc,
        self->la, self->lb, self->lc,
        ro->nx, ro->ny, ro->nz);
}

static void lp3v_render(RenderObject *ro, DirectX7 *g3d, Texture *tex) {
    LightedPolygon3V *self = (LightedPolygon3V*) ro;
    Polygon3V *p = (Polygon3V*) ro;
    if (Main_persQ == 0 && Main_mipMapping == 0) { lp3v_renderFast(ro, g3d, tex); return; }
    fog_g = tex->drawmode;

    sizex_g = size3(p->a->sx, p->b->sx, p->c->sx);
    sizey_g = size3(p->a->sy, p->b->sy, p->c->sy);

    int la2 = RenderObject_getLight(ro, p->a, self->la, g3d);
    int lb2 = RenderObject_getLight(ro, p->b, self->lb, g3d);
    int lc2 = RenderObject_getLight(ro, p->c, self->lc, g3d);

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
        TexturingPers_paint(g3d, tex,
            p->a, p->au, p->av, p->b, p->bu, p->bv, p->c, p->cu, p->cv,
            DirectX7_fogc, fog_g, sz2_g, q_g,
            la2, lb2, lc2, la2, lb2, lc2, la2, lb2, lc2,
            ro->nx, ro->ny, ro->nz);
    } else {
        TexturingAffine_paint(g3d, tex,
            p->a, p->au, p->av, p->b, p->bu, p->bv, p->c, p->cu, p->cv,
            DirectX7_fogc, fog_g, sz2_g,
            la2, lb2, lc2, la2, lb2, lc2, la2, lb2, lc2,
            ro->nx, ro->ny, ro->nz);
    }
}

static int lp3v_isVisible(RenderObject *ro, int x1, int y1, int x2, int y2) {
    /* Herdado de Polygon3V — reusamos a vtable base. */
    return Polygon3V_vt()->isVisible(ro, x1, y1, x2, y2);
}

static void lp3v_destroy(RenderObject *ro) { LightedPolygon3V_free((LightedPolygon3V*) ro); }

static const RenderObjectVTable g_lp3v_vt = {
    lp3v_render, lp3v_renderFast, lp3v_isVisible, lp3v_destroy
};
const RenderObjectVTable *LightedPolygon3V_vt(void) { return &g_lp3v_vt; }

LightedPolygon3V *LightedPolygon3V_new(Vertex *a, Vertex *b, Vertex *c,
                                        int8_t au, int8_t av, int8_t bu, int8_t bv, int8_t cu, int8_t cv) {
    LightedPolygon3V *lp = (LightedPolygon3V*) calloc(1, sizeof(LightedPolygon3V));
    Polygon3V *p3 = &lp->base;
    RenderObject_initWithNormals(&p3->base, &g_lp3v_vt, a, b, c);
    p3->a = a; p3->au = au & 0xff; p3->av = av & 0xff;
    p3->b = b; p3->bu = bu & 0xff; p3->bv = bv & 0xff;
    p3->c = c; p3->cu = cu & 0xff; p3->cv = cv & 0xff;
    lp->la = lp->lb = lp->lc = 255;
    return lp;
}

LightedPolygon3V *LightedPolygon3V_new_copy(const LightedPolygon3V *o) {
    LightedPolygon3V *lp = (LightedPolygon3V*) calloc(1, sizeof(LightedPolygon3V));
    lp->base = *(const Polygon3V*) &o->base;
    lp->base.base.vt = &g_lp3v_vt;
    lp->la = o->la; lp->lb = o->lb; lp->lc = o->lc;
    return lp;
}

LightedPolygon3V *LightedPolygon3V_new_fromColor(const ColorLightedPolygon3V *cp) {
    LightedPolygon3V *lp = (LightedPolygon3V*) calloc(1, sizeof(LightedPolygon3V));
    lp->base = *(const Polygon3V*) &cp->base;
    lp->base.base.vt = &g_lp3v_vt;
    lp->la = (cp->ar + cp->ag + cp->ab) / 3;
    lp->lb = (cp->br + cp->bg + cp->bb) / 3;
    lp->lc = (cp->cr + cp->cg + cp->cb) / 3;
    return lp;
}

void LightedPolygon3V_free(LightedPolygon3V *self) { free(self); }
