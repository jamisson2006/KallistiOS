/*
 * polygon4v.c — porte fiel de code/Rendering/Meshes/Polygon4V.java
 */
#include "polygon4v.h"
#include "../vertex.h"
#include "../directx7.h"
#include "../texture.h"
#include "../raw_image.h"
#include "../texturing_affine.h"
#include "../texturing_pers.h"

#include "../../math/math_utils.h"
#include "../../utils/main.h"

#include <stdlib.h>

static int q_g, sz2_g;
static int sizex_g, sizey_g, sizeu_g, sizev_g;
static int addmip_g, fog_g;

static int size4(int a, int b, int c, int d) {
    int t;
    if (b < a) { t=a; a=b; b=t; }
    if (c < a) { t=c; c=a; a=t; }
    if (c < b) { t=b; b=c; c=t; }
    int mn = a < d ? a : d;
    int mx = c > d ? c : d;
    return mx - mn;
}
static int min3(int a, int b, int c) {
    if (a == b || b == c) return a < c ? a : c;
    if (a == c) return a < b ? a : b;
    if (a < b && a < c) return a;
    if (b < a && b < c) return b;
    return c;
}
static int min4(int a, int b, int c, int d) {
    int t;
    if (a == b || b == d || b == c || d == c) return min3(a, c, d);
    else if (a == c) return min3(a, b, d);
    else if (a == d) return min3(a, b, c);
    if (b < a) { t=a; a=b; b=t; }
    if (c < a) { t=c; c=a; a=t; }
    if (c < b) { t=b; b=c; c=t; }
    return a < d ? a : d;
}

void Polygon4V_szCalcfogAdd(Polygon4V *self) {
    RenderObject *ro = &self->base;
    int sz = self->a->rz;
    if (self->d->rz < sz) sz = self->d->rz;
    if (self->c->rz < sz) sz = self->c->rz;
    if (self->b->rz < sz) sz = self->b->rz;
    ro->sz = sz;

    int sz2 = -sz / 255;
    if (DirectX7_fDist / 255 != 0) sz2 = -sz / (DirectX7_fDist / 255);
    if (sz2 > 255) sz2 = 255;
    if (sz2 < 0)   sz2 = 0;
    sz2 = 255 - sz2;
    int cr = (DirectX7_fogc >> 16) & 0xff, cg = (DirectX7_fogc >> 8) & 0xff, cb = DirectX7_fogc & 0xff;
    cr -= sz2; if (cr < 0) cr = 0;
    cg -= sz2; if (cg < 0) cg = 0;
    cb -= sz2; if (cb < 0) cb = 0;
    sz2_g = (cr << 16) | (cg << 8) | cb;
}

static int p4v_isVisible(RenderObject *ro, int x1, int y1, int x2, int y2) {
    Polygon4V *self = (Polygon4V*) ro;
    Vertex *a=self->a, *b=self->b, *c=self->c, *d=self->d;
    if (a->sx<x1 && b->sx<x1 && c->sx<x1 && d->sx<x1) return 0;
    if (a->sx>x2 && b->sx>x2 && c->sx>x2 && d->sx>x2) return 0;
    if (a->sy<y1 && b->sy<y1 && c->sy<y1 && d->sy<y1) return 0;
    if (a->sy>y2 && b->sy>y2 && c->sy>y2 && d->sy>y2) return 0;

    if ((a->sx - b->sx) * (b->sy - d->sy) <= (a->sy - b->sy) * (b->sx - d->sx) &&
        (c->sx - d->sx) * (d->sy - b->sy) <= (c->sy - d->sy) * (d->sx - b->sx)) return 0;

    int sz = a->rz;
    if (d->rz < sz) sz = d->rz;
    if (c->rz < sz) sz = c->rz;
    if (b->rz < sz) sz = b->rz;
    if (sz > 0) return 0;
    if (-sz > DirectX7_drDist) return 0;

    sz = a->rz + b->rz + c->rz + d->rz;
    if (ro->ny > 4000 || ro->ny < -4000) sz += Main_floorOffsetSZ;
    ro->sz = sz;
    return 1;
}

static void p4v_renderFast(RenderObject *ro, DirectX7 *g3d, Texture *tex) {
    Polygon4V *s = (Polygon4V*) ro;
    if (tex->drawmode == 5)
        sz2_g = 0xff - MathUtils_calcLight(ro->nx, ro->ny, ro->nz,
            DirectX7_lightdirx, DirectX7_lightdiry, DirectX7_lightdirz);
    if (tex->drawmode == 1) Polygon4V_szCalcfogAdd(s);

    TexturingAffine_paint(g3d, tex, s->a, s->au, s->av, s->b, s->bu, s->bv, s->d, s->du, s->dv,
        DirectX7_fogc, tex->drawmode, sz2_g, 255,255,255, 255,255,255, 255,255,255, ro->nx, ro->ny, ro->nz);
    TexturingAffine_paint(g3d, tex, s->b, s->bu, s->bv, s->c, s->cu, s->cv, s->d, s->du, s->dv,
        DirectX7_fogc, tex->drawmode, sz2_g, 255,255,255, 255,255,255, 255,255,255, ro->nx, ro->ny, ro->nz);
}

static void p4v_render(RenderObject *ro, DirectX7 *g3d, Texture *tex) {
    Polygon4V *s = (Polygon4V*) ro;
    if (Main_persQ == 0 && Main_mipMapping == 0) { p4v_renderFast(ro, g3d, tex); return; }

    fog_g = tex->drawmode;
    int pers = Main_persQ > 0;

    if ((Main_mipMapping && tex->mip != NULL) || pers) {
        sizex_g = size4(s->a->sx, s->b->sx, s->c->sx, s->d->sx);
        sizey_g = size4(s->a->sy, s->b->sy, s->c->sy, s->d->sy);
    }

    if (fog_g == 5)
        sz2_g = 0xff - MathUtils_calcLight(ro->nx, ro->ny, ro->nz,
            DirectX7_lightdirx, DirectX7_lightdiry, DirectX7_lightdirz);
    else if (fog_g == 1) Polygon4V_szCalcfogAdd(s);

    q_g = (ro->ny > 4000 || ro->ny < -4000) ? 9999999 : Main_q;

    if (Main_mipMapping && tex->mip != NULL) {
        tex->rImg = tex->mip[0];
        if (tex->drawmode < 10 || tex->drawmode > 12) {
            addmip_g = q_g > 999 ? 2 : 3;
            sizeu_g = size4(s->au, s->bu, s->cu, s->du) * tex->rImg->w >> 8;
            sizev_g = size4(s->av, s->bv, s->cv, s->dv) * tex->rImg->h >> 8;
            if (((sizex_g + sizey_g) >> 1) * addmip_g < sizeu_g + sizev_g) {
                tex->rImg = tex->mip[1];
                if (tex->mip != NULL) {
                    sizeu_g >>= 1; sizev_g >>= 1;
                    if (((sizex_g + sizey_g) >> 1) * addmip_g < sizeu_g + sizev_g) {
                        tex->rImg = tex->mip[2];
                    }
                }
            }
        } else if (sizex_g < tex->rImg->w || sizey_g < tex->rImg->w) {
            tex->rImg = tex->mip[1];
            if (tex->mip != NULL && (sizex_g < tex->rImg->w || sizey_g < tex->rImg->w))
                tex->rImg = tex->mip[2];
        }
    }

    if (pers)
        pers = ((sizex_g > (Main_persQ < 3 ? 70 : 50) || sizey_g > (Main_persQ < 3 ? 100 : 70))
                && tex->perspectiveCorrection) || Main_persQ == 4;

    int base = (fog_g != 3 && fog_g != 6) || Main_fogQ == 0;
    if (!base)
        base = (min4(s->a->rz, s->b->rz, s->c->rz, s->d->rz) == (s->a->rz < s->c->rz ? s->a->rz : s->c->rz));

    Vertex *a=s->a,*b=s->b,*c=s->c,*d=s->d;
    if (base) {
        if (pers) {
            TexturingPers_paint(g3d, tex, a, s->au, s->av, b, s->bu, s->bv, d, s->du, s->dv,
                DirectX7_fogc, fog_g, sz2_g, q_g, 255,255,255, 255,255,255, 255,255,255, ro->nx, ro->ny, ro->nz);
            TexturingPers_paint(g3d, tex, b, s->bu, s->bv, c, s->cu, s->cv, d, s->du, s->dv,
                DirectX7_fogc, fog_g, sz2_g, q_g, 255,255,255, 255,255,255, 255,255,255, ro->nx, ro->ny, ro->nz);
        } else {
            TexturingAffine_paint(g3d, tex, a, s->au, s->av, b, s->bu, s->bv, d, s->du, s->dv,
                DirectX7_fogc, fog_g, sz2_g, 255,255,255, 255,255,255, 255,255,255, ro->nx, ro->ny, ro->nz);
            TexturingAffine_paint(g3d, tex, b, s->bu, s->bv, c, s->cu, s->cv, d, s->du, s->dv,
                DirectX7_fogc, fog_g, sz2_g, 255,255,255, 255,255,255, 255,255,255, ro->nx, ro->ny, ro->nz);
        }
    } else {
        if (pers) {
            TexturingPers_paint(g3d, tex, a, s->au, s->av, b, s->bu, s->bv, c, s->cu, s->cv,
                DirectX7_fogc, fog_g, sz2_g, q_g, 255,255,255, 255,255,255, 255,255,255, ro->nx, ro->ny, ro->nz);
            TexturingPers_paint(g3d, tex, a, s->au, s->av, c, s->cu, s->cv, d, s->du, s->dv,
                DirectX7_fogc, fog_g, sz2_g, q_g, 255,255,255, 255,255,255, 255,255,255, ro->nx, ro->ny, ro->nz);
        } else {
            TexturingAffine_paint(g3d, tex, a, s->au, s->av, b, s->bu, s->bv, c, s->cu, s->cv,
                DirectX7_fogc, fog_g, sz2_g, 255,255,255, 255,255,255, 255,255,255, ro->nx, ro->ny, ro->nz);
            TexturingAffine_paint(g3d, tex, a, s->au, s->av, c, s->cu, s->cv, d, s->du, s->dv,
                DirectX7_fogc, fog_g, sz2_g, 255,255,255, 255,255,255, 255,255,255, ro->nx, ro->ny, ro->nz);
        }
    }
}

static void p4v_destroy(RenderObject *ro) { Polygon4V_free((Polygon4V*) ro); }

static const RenderObjectVTable g_p4v_vt = {
    p4v_render, p4v_renderFast, p4v_isVisible, p4v_destroy
};
const RenderObjectVTable *Polygon4V_vt(void) { return &g_p4v_vt; }

Polygon4V *Polygon4V_new(Vertex *a, Vertex *b, Vertex *c, Vertex *d,
                          int8_t au, int8_t av, int8_t bu, int8_t bv,
                          int8_t cu, int8_t cv, int8_t du, int8_t dv) {
    Polygon4V *p = (Polygon4V*) calloc(1, sizeof(Polygon4V));
    RenderObject_initWithNormals(&p->base, &g_p4v_vt, a, b, c);
    int oldnx = p->base.nx, oldny = p->base.ny, oldnz = p->base.nz;
    RenderObject_calculateNormals(&p->base, a, c, d);
    p->base.nx = p->base.nx / 2 + oldnx / 2;
    p->base.ny = p->base.ny / 2 + oldny / 2;
    p->base.nz = p->base.nz / 2 + oldnz / 2;

    p->a = a; p->au = au & 0xff; p->av = av & 0xff;
    p->b = b; p->bu = bu & 0xff; p->bv = bv & 0xff;
    p->c = c; p->cu = cu & 0xff; p->cv = cv & 0xff;
    p->d = d; p->du = du & 0xff; p->dv = dv & 0xff;
    return p;
}

Polygon4V *Polygon4V_new_copy(const Polygon4V *o) {
    Polygon4V *p = (Polygon4V*) calloc(1, sizeof(Polygon4V));
    p->base.vt = &g_p4v_vt;
    p->base.nx = o->base.nx; p->base.ny = o->base.ny; p->base.nz = o->base.nz;
    p->a=o->a; p->au=o->au; p->av=o->av;
    p->b=o->b; p->bu=o->bu; p->bv=o->bv;
    p->c=o->c; p->cu=o->cu; p->cv=o->cv;
    p->d=o->d; p->du=o->du; p->dv=o->dv;
    p->tex=o->tex;
    return p;
}
void Polygon4V_free(Polygon4V *self) { free(self); }
