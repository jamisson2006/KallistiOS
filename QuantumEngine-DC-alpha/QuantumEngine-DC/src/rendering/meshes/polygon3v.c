/*
 * polygon3v.c — porte fiel de code/Rendering/Meshes/Polygon3V.java
 */
#include "polygon3v.h"
#include "../vertex.h"
#include "../directx7.h"
#include "../texture.h"
#include "../raw_image.h"
#include "../texturing_affine.h"
#include "../texturing_pers.h"

#include "../../math/math_utils.h"
#include "../../utils/main.h"

#include <stdlib.h>

static int   q_g, sz2_g;
static int   sizex_g, sizey_g, sizeu_g, sizev_g;
static int   addmip_g, fog_g;

static int size3(int a, int b, int c) {
    int t;
    if (b < a) { t=a; a=b; b=t; }
    if (c < a) { t=c; c=a; a=t; }
    if (c < b) { t=b; b=c; c=t; }
    return c - a;
}

void Polygon3V_szCalcfogAdd(Polygon3V *self) {
    RenderObject *ro = &self->base;
    int sz = self->a->rz;
    if (self->c->rz < sz) sz = self->c->rz;
    if (self->b->rz < sz) sz = self->b->rz;
    ro->sz = sz;

    int sz2 = -sz / 255;
    if (DirectX7_fDist / 255 != 0) sz2 = -sz / (DirectX7_fDist / 255);
    if (sz2 > 255) sz2 = 255;
    if (sz2 < 0)   sz2 = 0;
    sz2 = 255 - sz2;
    int cr = (DirectX7_fogc >> 16) & 0xff;
    int cg = (DirectX7_fogc >> 8)  & 0xff;
    int cb =  DirectX7_fogc        & 0xff;
    cr -= sz2; if (cr < 0) cr = 0;
    cg -= sz2; if (cg < 0) cg = 0;
    cb -= sz2; if (cb < 0) cb = 0;
    sz2_g = (cr << 16) | (cg << 8) | cb;
}

static int p3v_isVisible(RenderObject *ro, int x1, int y1, int x2, int y2) {
    Polygon3V *self = (Polygon3V*) ro;
    Vertex *a = self->a, *b = self->b, *c = self->c;
    if (a->sx < x1 && b->sx < x1 && c->sx < x1) return 0;
    if (a->sx > x2 && b->sx > x2 && c->sx > x2) return 0;
    if (a->sy < y1 && b->sy < y1 && c->sy < y1) return 0;
    if (a->sy > y2 && b->sy > y2 && c->sy > y2) return 0;

    if ((a->sx - b->sx) * (b->sy - c->sy) <= (a->sy - b->sy) * (b->sx - c->sx)) return 0;

    int sz = a->rz;
    if (c->rz < sz) sz = c->rz;
    if (b->rz < sz) sz = b->rz;
    if (sz > 0) return 0;
    if (-sz > DirectX7_drDist) return 0;

    sz = ((a->rz + b->rz + c->rz)) * 4 / 3;
    if (ro->ny > 4000 || ro->ny < -4000) sz += Main_floorOffsetSZ;
    ro->sz = sz;
    return 1;
}

static void p3v_renderFast(RenderObject *ro, DirectX7 *g3d, Texture *tex) {
    Polygon3V *self = (Polygon3V*) ro;
    if (tex->drawmode == 5) {
        sz2_g = 0xff - MathUtils_calcLight(ro->nx, ro->ny, ro->nz,
            DirectX7_lightdirx, DirectX7_lightdiry, DirectX7_lightdirz);
    } else if (tex->drawmode == 1) {
        Polygon3V_szCalcfogAdd(self);
    }
    TexturingAffine_paint(g3d, tex,
        self->a, self->au, self->av,
        self->b, self->bu, self->bv,
        self->c, self->cu, self->cv,
        DirectX7_fogc, tex->drawmode, sz2_g,
        255,255,255, 255,255,255, 255,255,255,
        ro->nx, ro->ny, ro->nz);
}

static void p3v_render(RenderObject *ro, DirectX7 *g3d, Texture *tex) {
    Polygon3V *self = (Polygon3V*) ro;
    if (Main_persQ == 0 && Main_mipMapping == 0) { p3v_renderFast(ro, g3d, tex); return; }

    fog_g = tex->drawmode;
    int pers = Main_persQ > 0;

    if ((Main_mipMapping && tex->mip != NULL) || pers) {
        sizex_g = size3(self->a->sx, self->b->sx, self->c->sx);
        sizey_g = size3(self->a->sy, self->b->sy, self->c->sy);
    }

    if (fog_g == 5) {
        sz2_g = 0xff - MathUtils_calcLight(ro->nx, ro->ny, ro->nz,
            DirectX7_lightdirx, DirectX7_lightdiry, DirectX7_lightdirz);
    } else if (fog_g == 1) {
        Polygon3V_szCalcfogAdd(self);
    }

    q_g = (ro->ny > 4000 || ro->ny < -4000) ? 9999999 : Main_q;

    if (Main_mipMapping && tex->mip != NULL) {
        tex->rImg = tex->mip[0];
        if (tex->drawmode < 10 || tex->drawmode > 12) {
            addmip_g = q_g > 999 ? 2 : 3;
            sizeu_g = size3(self->au, self->bu, self->cu) * tex->rImg->w >> 8;
            sizev_g = size3(self->av, self->bv, self->cv) * tex->rImg->h >> 8;
            if (((sizex_g + sizey_g) >> 1) * addmip_g < sizeu_g + sizev_g) {
                tex->rImg = tex->mip[1];
                if (tex->mip != NULL) {
                    sizeu_g >>= 1; sizev_g >>= 1;
                    if (((sizex_g + sizey_g) >> 1) * addmip_g < sizeu_g + sizev_g) {
                        tex->rImg = tex->mip[2];
                    }
                }
            }
        } else {
            if (sizex_g < tex->rImg->w || sizey_g < tex->rImg->w) {
                tex->rImg = tex->mip[1];
                if (tex->mip != NULL && (sizex_g < tex->rImg->w || sizey_g < tex->rImg->w)) {
                    tex->rImg = tex->mip[2];
                }
            }
        }
    }

    if (pers) pers = ((sizex_g > (Main_persQ < 3 ? 30 : 15) || sizey_g > (Main_persQ < 3 ? 30 : 15))
                     && tex->perspectiveCorrection) || Main_persQ == 4;

    if (pers) {
        TexturingPers_paint(g3d, tex,
            self->a, self->au, self->av,
            self->b, self->bu, self->bv,
            self->c, self->cu, self->cv,
            DirectX7_fogc, fog_g, sz2_g, q_g,
            255,255,255, 255,255,255, 255,255,255,
            ro->nx, ro->ny, ro->nz);
    } else {
        TexturingAffine_paint(g3d, tex,
            self->a, self->au, self->av,
            self->b, self->bu, self->bv,
            self->c, self->cu, self->cv,
            DirectX7_fogc, fog_g, sz2_g,
            255,255,255, 255,255,255, 255,255,255,
            ro->nx, ro->ny, ro->nz);
    }
}

static void p3v_destroy(RenderObject *ro) { Polygon3V_free((Polygon3V*) ro); }

static const RenderObjectVTable g_p3v_vt = {
    p3v_render, p3v_renderFast, p3v_isVisible, p3v_destroy
};

const RenderObjectVTable *Polygon3V_vt(void) { return &g_p3v_vt; }

Polygon3V *Polygon3V_new(Vertex *a, Vertex *b, Vertex *c,
                          int8_t au, int8_t av, int8_t bu, int8_t bv, int8_t cu, int8_t cv) {
    Polygon3V *p = (Polygon3V*) calloc(1, sizeof(Polygon3V));
    RenderObject_initWithNormals(&p->base, &g_p3v_vt, a, b, c);
    p->a = a; p->au = au & 0xff; p->av = av & 0xff;
    p->b = b; p->bu = bu & 0xff; p->bv = bv & 0xff;
    p->c = c; p->cu = cu & 0xff; p->cv = cv & 0xff;
    p->tex = 0;
    return p;
}

Polygon3V *Polygon3V_new_copy(const Polygon3V *o) {
    Polygon3V *p = (Polygon3V*) calloc(1, sizeof(Polygon3V));
    p->base.vt = &g_p3v_vt;
    p->base.nx = o->base.nx; p->base.ny = o->base.ny; p->base.nz = o->base.nz;
    p->a = o->a; p->au = o->au; p->av = o->av;
    p->b = o->b; p->bu = o->bu; p->bv = o->bv;
    p->c = o->c; p->cu = o->cu; p->cv = o->cv;
    p->tex = o->tex;
    return p;
}
void Polygon3V_free(Polygon3V *self) { free(self); }
