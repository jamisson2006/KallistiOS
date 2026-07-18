/*
 * lighted_polygon4v.c — porte fiel de code/Rendering/Meshes/LightedPolygon4V.java
 */
#include "lighted_polygon4v.h"
#include "color_lighted_polygon4v.h"
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

static int size4(int a, int b, int c, int d) {
    int t;
    if (b < a) { t=a; a=b; b=t; }
    if (c < a) { t=c; c=a; a=t; }
    if (c < b) { t=b; b=c; c=t; }
    int mn = a < d ? a : d, mx = c > d ? c : d;
    return mx - mn;
}

static void lp4v_renderFast(RenderObject *ro, DirectX7 *g3d, Texture *tex) {
    LightedPolygon4V *self = (LightedPolygon4V*) ro;
    Polygon4V *p = (Polygon4V*) ro;
    if (tex->drawmode == 5) {
        sz2_g = 0xff - MathUtils_calcLight(ro->nx, ro->ny, ro->nz,
            DirectX7_lightdirx, DirectX7_lightdiry, DirectX7_lightdirz);
        if (sz2_g < 0) sz2_g = 0;
    }
    if (tex->drawmode == 1) Polygon4V_szCalcfogAdd(p);

    TexturingAffine_paint(g3d, tex,
        p->a, p->au, p->av, p->b, p->bu, p->bv, p->d, p->du, p->dv,
        DirectX7_fogc, tex->drawmode, sz2_g,
        self->la, self->lb, self->ld, self->la, self->lb, self->ld, self->la, self->lb, self->ld,
        ro->nx, ro->ny, ro->nz);
    TexturingAffine_paint(g3d, tex,
        p->b, p->bu, p->bv, p->c, p->cu, p->cv, p->d, p->du, p->dv,
        DirectX7_fogc, tex->drawmode, sz2_g,
        self->lb, self->lc, self->ld, self->lb, self->lc, self->ld, self->lb, self->lc, self->ld,
        ro->nx, ro->ny, ro->nz);
}

static void lp4v_render(RenderObject *ro, DirectX7 *g3d, Texture *tex) {
    LightedPolygon4V *self = (LightedPolygon4V*) ro;
    Polygon4V *p = (Polygon4V*) ro;
    if (Main_persQ == 0 && Main_mipMapping == 0) { lp4v_renderFast(ro, g3d, tex); return; }

    fog_g = tex->drawmode;
    sizex_g = size4(p->a->sx, p->b->sx, p->c->sx, p->d->sx);
    sizey_g = size4(p->a->sy, p->b->sy, p->c->sy, p->d->sy);

    int la2 = RenderObject_getLight(ro, p->a, self->la, g3d);
    int lb2 = RenderObject_getLight(ro, p->b, self->lb, g3d);
    int lc2 = RenderObject_getLight(ro, p->c, self->lc, g3d);
    int ld2 = RenderObject_getLight(ro, p->d, self->ld, g3d);

    if (fog_g == 5)
        sz2_g = 0xff - MathUtils_calcLight(ro->nx, ro->ny, ro->nz,
            DirectX7_lightdirx, DirectX7_lightdiry, DirectX7_lightdirz);
    else if (fog_g == 1) Polygon4V_szCalcfogAdd(p);

    q_g = (ro->ny > 4000 || ro->ny < -4000) ? 9999999 : Main_q;

    if (Main_mipMapping && tex->mip != NULL) {
        tex->rImg = tex->mip[0];
        if (tex->drawmode < 10 || tex->drawmode > 12) {
            addmip_g = q_g > 999 ? 2 : 3;
            sizeu_g = size4(p->au, p->bu, p->cu, p->du) * tex->rImg->w >> 8;
            sizev_g = size4(p->av, p->bv, p->cv, p->dv) * tex->rImg->h >> 8;
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

    int pers = (tex->perspectiveCorrection
                && (sizex_g > 70 - (Main_persQ < 3 ? 0 : 20) || sizey_g > 100 - (Main_persQ < 3 ? 0 : 30)))
               && Main_persQ != 0 || Main_persQ == 4;

    if (pers) {
        TexturingPers_paint(g3d, tex, p->a, p->au, p->av, p->b, p->bu, p->bv, p->d, p->du, p->dv,
            DirectX7_fogc, fog_g, sz2_g, q_g,
            la2, lb2, ld2, la2, lb2, ld2, la2, lb2, ld2, ro->nx, ro->ny, ro->nz);
        TexturingPers_paint(g3d, tex, p->b, p->bu, p->bv, p->c, p->cu, p->cv, p->d, p->du, p->dv,
            DirectX7_fogc, fog_g, sz2_g, q_g,
            lb2, lc2, ld2, lb2, lc2, ld2, lb2, lc2, ld2, ro->nx, ro->ny, ro->nz);
    } else {
        TexturingAffine_paint(g3d, tex, p->a, p->au, p->av, p->b, p->bu, p->bv, p->d, p->du, p->dv,
            DirectX7_fogc, fog_g, sz2_g,
            la2, lb2, ld2, la2, lb2, ld2, la2, lb2, ld2, ro->nx, ro->ny, ro->nz);
        TexturingAffine_paint(g3d, tex, p->b, p->bu, p->bv, p->c, p->cu, p->cv, p->d, p->du, p->dv,
            DirectX7_fogc, fog_g, sz2_g,
            lb2, lc2, ld2, lb2, lc2, ld2, lb2, lc2, ld2, ro->nx, ro->ny, ro->nz);
    }
}

static int lp4v_isVisible(RenderObject *ro, int x1, int y1, int x2, int y2) {
    return Polygon4V_vt()->isVisible(ro, x1, y1, x2, y2);
}
static void lp4v_destroy(RenderObject *ro) { LightedPolygon4V_free((LightedPolygon4V*) ro); }

static const RenderObjectVTable g_lp4v_vt = { lp4v_render, lp4v_renderFast, lp4v_isVisible, lp4v_destroy };
const RenderObjectVTable *LightedPolygon4V_vt(void) { return &g_lp4v_vt; }

LightedPolygon4V *LightedPolygon4V_new(Vertex *a, Vertex *b, Vertex *c, Vertex *d,
    int8_t au, int8_t av, int8_t bu, int8_t bv, int8_t cu, int8_t cv, int8_t du, int8_t dv) {
    LightedPolygon4V *lp = (LightedPolygon4V*) calloc(1, sizeof(LightedPolygon4V));
    Polygon4V *p4 = &lp->base;
    RenderObject_initWithNormals(&p4->base, &g_lp4v_vt, a, b, c);
    int oldnx=p4->base.nx, oldny=p4->base.ny, oldnz=p4->base.nz;
    RenderObject_calculateNormals(&p4->base, a, c, d);
    p4->base.nx = p4->base.nx/2 + oldnx/2;
    p4->base.ny = p4->base.ny/2 + oldny/2;
    p4->base.nz = p4->base.nz/2 + oldnz/2;
    p4->a=a; p4->au=au&0xff; p4->av=av&0xff;
    p4->b=b; p4->bu=bu&0xff; p4->bv=bv&0xff;
    p4->c=c; p4->cu=cu&0xff; p4->cv=cv&0xff;
    p4->d=d; p4->du=du&0xff; p4->dv=dv&0xff;
    lp->la = lp->lb = lp->lc = lp->ld = 255;
    return lp;
}

LightedPolygon4V *LightedPolygon4V_new_copy(const LightedPolygon4V *o) {
    LightedPolygon4V *lp = (LightedPolygon4V*) calloc(1, sizeof(LightedPolygon4V));
    lp->base = o->base;
    lp->base.base.vt = &g_lp4v_vt;
    lp->la = o->la; lp->lb = o->lb; lp->lc = o->lc; lp->ld = o->ld;
    return lp;
}

LightedPolygon4V *LightedPolygon4V_new_fromColor(const ColorLightedPolygon4V *cp) {
    LightedPolygon4V *lp = (LightedPolygon4V*) calloc(1, sizeof(LightedPolygon4V));
    lp->base = *(const Polygon4V*) &cp->base;
    lp->base.base.vt = &g_lp4v_vt;
    lp->la = (cp->ar + cp->ag + cp->ab) / 3;
    lp->lb = (cp->br + cp->bg + cp->bb) / 3;
    lp->lc = (cp->cr + cp->cg + cp->cb) / 3;
    lp->ld = (cp->dr + cp->dg + cp->db) / 3;
    return lp;
}

void LightedPolygon4V_free(LightedPolygon4V *self) { free(self); }
