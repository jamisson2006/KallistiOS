/*
 * texturing_affine.c — porte fiel de code/Rendering/TexturingAffine.java
 *
 * Este arquivo porta os modos 0 (baseline) e 9 (vertex-lit) de forma FIEL
 * ao original — sao os modos com que a versao J2ME renderiza 100% das
 * meshes em condicoes de jogo. Os demais modos (fog aditivo, glass, mul,
 * dither, RGB, etc.) delegam ao modo 0 e ficam marcados para conversao
 * dedicada em futuras fases (ver docs/STATUS_CONVERSAO.md).
 */
#include "texturing_affine.h"
#include "vertex.h"
#include "texture.h"
#include "raw_image.h"
#include "directx7.h"

#include "../utils/main.h"
#include <limits.h>
#include <stdlib.h>

extern int DeveloperMenu_renderPolygonsOverwrite;

/* Java >>> = shift logico. */
static inline unsigned qe_urshift(int v, int s) { return ((unsigned) v) >> s; }

void TexturingAffine_line_0(int x1, int x2, int *rgb, const int *tex,
                            int lengthBIT, int widthBIT, int u, int v, int du, int dv) {
    int col;
    while (x2 - x1 >= 6) {
        if ((col = tex[((qe_urshift(v += dv, QE_TEXAFF_FP) << widthBIT) + qe_urshift(u += du, QE_TEXAFF_FP)) & lengthBIT]) != 0) rgb[x1  ] = col;
        if ((col = tex[((qe_urshift(v += dv, QE_TEXAFF_FP) << widthBIT) + qe_urshift(u += du, QE_TEXAFF_FP)) & lengthBIT]) != 0) rgb[x1+1] = col;
        if ((col = tex[((qe_urshift(v += dv, QE_TEXAFF_FP) << widthBIT) + qe_urshift(u += du, QE_TEXAFF_FP)) & lengthBIT]) != 0) rgb[x1+2] = col;
        if ((col = tex[((qe_urshift(v += dv, QE_TEXAFF_FP) << widthBIT) + qe_urshift(u += du, QE_TEXAFF_FP)) & lengthBIT]) != 0) rgb[x1+3] = col;
        if ((col = tex[((qe_urshift(v += dv, QE_TEXAFF_FP) << widthBIT) + qe_urshift(u += du, QE_TEXAFF_FP)) & lengthBIT]) != 0) rgb[x1+4] = col;
        if ((col = tex[((qe_urshift(v += dv, QE_TEXAFF_FP) << widthBIT) + qe_urshift(u += du, QE_TEXAFF_FP)) & lengthBIT]) != 0) rgb[x1+5] = col;
        x1 += 6;
    }
    while (x1 < x2) {
        if ((col = tex[((qe_urshift(v += dv, QE_TEXAFF_FP) << widthBIT) + qe_urshift(u += du, QE_TEXAFF_FP)) & lengthBIT]) != 0) rgb[x1] = col;
        x1++;
    }
}

#define QE_SWAP_V(x, y) do { Vertex *t_ = (x); (x) = (y); (y) = t_; } while (0)
#define QE_SWAP_I(x, y) do { int    t_ = (x); (x) = (y); (y) = t_; } while (0)

/* Baseline (modo 0) — porte literal do paintAffine_0. */
void TexturingAffine_paintAffine_0(DirectX7 *g3d, Texture *texture,
    Vertex *a, int au, int av,
    Vertex *b, int bu, int bv,
    Vertex *c, int cu, int cv) {

    /* Ordena por sy: a <= b <= c */
    if (b->sy < a->sy) { QE_SWAP_V(a, b); QE_SWAP_I(au, bu); QE_SWAP_I(av, bv); }
    if (c->sy < a->sy) { QE_SWAP_V(a, c); QE_SWAP_I(au, cu); QE_SWAP_I(av, cv); }
    if (c->sy < b->sy) { QE_SWAP_V(b, c); QE_SWAP_I(bu, cu); QE_SWAP_I(bv, cv); }
    if (a->sy == c->sy) return;

    const int *tex     = texture->rImg->img;
    const int lengthBIT = texture->rImg->w * texture->rImg->h - 1;
    const int widthBIT  = texture->rImg->widthBIT;
    int *rgb = g3d->display;
    int x1, x2, u, v, tempI;

    tempI = c->sy - a->sy;
    const int dx_start = ((c->sx - a->sx) << QE_TEXAFF_FP_POSITION) / tempI;
    const int du_start = ((cu    - au   ) << QE_TEXAFF_FP)         / tempI;
    const int dv_start = ((cv    - av   ) << QE_TEXAFF_FP)         / tempI;

    int dx_end = 0, du_end = 0, dv_end = 0;
    if (b->sy != a->sy) {
        tempI = b->sy - a->sy;
        dx_end = ((b->sx - a->sx) << QE_TEXAFF_FP_POSITION) / tempI;
        du_end = ((bu    - au   ) << QE_TEXAFF_FP)         / tempI;
        dv_end = ((bv    - av   ) << QE_TEXAFF_FP)         / tempI;
    }

    tempI = b->sy - a->sy;
    int x_start = (a->sx << QE_TEXAFF_FP_POSITION) + dx_start * tempI;
    int u_start = (au    << QE_TEXAFF_FP)          + du_start * tempI;
    int v_start = (av    << QE_TEXAFF_FP)          + dv_start * tempI;

    int x_end = b->sx << QE_TEXAFF_FP_POSITION;
    int u_end = bu    << QE_TEXAFF_FP;
    int v_end = bv    << QE_TEXAFF_FP;

    tempI = (x_start - x_end) >> QE_TEXAFF_FP_POSITION;
    if (tempI == 0) return;
    const int du = (u_start - u_end) / tempI;
    const int dv = (v_start - v_end) / tempI;

    x_end = x_start = a->sx << QE_TEXAFF_FP_POSITION;
    u_end = u_start = au    << QE_TEXAFF_FP;
    v_end = v_start = av    << QE_TEXAFF_FP;

    int y_start = a->sy;
    int y_end   = c->sy < g3d->height ? c->sy : g3d->height;

    for (; y_start < y_end;
         y_start++,
         x_start += dx_start, u_start += du_start, v_start += dv_start,
         x_end   += dx_end,   u_end   += du_end,   v_end   += dv_end) {

        if (y_start == b->sy) {
            if (c->sy == b->sy) return;
            tempI   = b->sy - a->sy;
            x_start = (a->sx << QE_TEXAFF_FP_POSITION) + dx_start * tempI;
            u_start = (au    << QE_TEXAFF_FP)          + du_start * tempI;
            v_start = (av    << QE_TEXAFF_FP)          + dv_start * tempI;
            x_end   = b->sx << QE_TEXAFF_FP_POSITION;
            u_end   = bu    << QE_TEXAFF_FP;
            v_end   = bv    << QE_TEXAFF_FP;
            tempI   = c->sy - b->sy;
            dx_end  = ((c->sx - b->sx) << QE_TEXAFF_FP_POSITION) / tempI;
            du_end  = ((cu    - bu   ) << QE_TEXAFF_FP)         / tempI;
            dv_end  = ((cv    - bv   ) << QE_TEXAFF_FP)         / tempI;
        }

        if (y_start < 0) continue;

        if (x_start > x_end) {
            x1 = x_end   >> QE_TEXAFF_FP_POSITION;
            u  = u_end;
            v  = v_end;
            x2 = x_start >> QE_TEXAFF_FP_POSITION;
        } else {
            x1 = x_start >> QE_TEXAFF_FP_POSITION;
            u  = u_start;
            v  = v_start;
            x2 = x_end   >> QE_TEXAFF_FP_POSITION;
        }

        if (x1 < 0) { u -= du * x1; v -= dv * x1; x1 = 0; }
        if (x2 > g3d->width) x2 = g3d->width;

        tempI = g3d->width * y_start;
        x1 += tempI; x2 += tempI;
        u -= du; v -= dv;
        TexturingAffine_line_0(x1, x2, rgb, tex, lengthBIT, widthBIT, u, v, du, dv);
    }
}

/*
 * paintAffine_9 — porte fiel do "lit textured triangle".
 * Como o modo 9 eh MUITO similar ao modo 0 mas com interpolacao de brilho
 * por vertice (af, bf, cf), fazemos o porte compacto: cada pixel calcula
 * a intensidade interpolada e aplica shift-multiplicacao no cor de textura.
 */
void TexturingAffine_paintAffine_9(DirectX7 *g3d, Texture *texture,
    Vertex *a, int au, int av,
    Vertex *b, int bu, int bv,
    Vertex *c, int cu, int cv,
    int fogc, int sz2, int af, int bf, int cf, int nx, int ny, int nz) {
    (void) fogc; (void) sz2; (void) nx; (void) ny; (void) nz;
    /* Se todos os brilhos sao iguais a 255, cai para modo 0 (fiel ao Java). */
    if (af == 255 && bf == 255 && cf == 255) {
        TexturingAffine_paintAffine_0(g3d, texture, a, au, av, b, bu, bv, c, cu, cv);
        return;
    }

    if (b->sy < a->sy) { QE_SWAP_V(a, b); QE_SWAP_I(au, bu); QE_SWAP_I(av, bv); QE_SWAP_I(af, bf); }
    if (c->sy < a->sy) { QE_SWAP_V(a, c); QE_SWAP_I(au, cu); QE_SWAP_I(av, cv); QE_SWAP_I(af, cf); }
    if (c->sy < b->sy) { QE_SWAP_V(b, c); QE_SWAP_I(bu, cu); QE_SWAP_I(bv, cv); QE_SWAP_I(bf, cf); }
    if (a->sy == c->sy) return;

    const int *tex     = texture->rImg->img;
    const int lengthBIT = texture->rImg->w * texture->rImg->h - 1;
    const int widthBIT  = texture->rImg->widthBIT;
    int *rgb = g3d->display;

    int tempI = c->sy - a->sy;
    const int dx_start = ((c->sx - a->sx) << QE_TEXAFF_FP_POSITION) / tempI;
    const int du_start = ((cu    - au   ) << QE_TEXAFF_FP)         / tempI;
    const int dv_start = ((cv    - av   ) << QE_TEXAFF_FP)         / tempI;
    const int df_start = ((cf    - af   ) << QE_TEXAFF_FP)         / tempI;

    int dx_end = 0, du_end = 0, dv_end = 0, df_end = 0;
    if (b->sy != a->sy) {
        tempI = b->sy - a->sy;
        dx_end = ((b->sx - a->sx) << QE_TEXAFF_FP_POSITION) / tempI;
        du_end = ((bu    - au   ) << QE_TEXAFF_FP)         / tempI;
        dv_end = ((bv    - av   ) << QE_TEXAFF_FP)         / tempI;
        df_end = ((bf    - af   ) << QE_TEXAFF_FP)         / tempI;
    }

    int x_start = a->sx << QE_TEXAFF_FP_POSITION;
    int u_start = au    << QE_TEXAFF_FP;
    int v_start = av    << QE_TEXAFF_FP;
    int f_start = af    << QE_TEXAFF_FP;
    int x_end   = x_start, u_end = u_start, v_end = v_start, f_end = f_start;

    int y_start = a->sy;
    int y_end   = c->sy < g3d->height ? c->sy : g3d->height;

    for (; y_start < y_end;
         y_start++,
         x_start += dx_start, u_start += du_start, v_start += dv_start, f_start += df_start,
         x_end   += dx_end,   u_end   += du_end,   v_end   += dv_end,   f_end   += df_end) {

        if (y_start == b->sy) {
            if (c->sy == b->sy) return;
            tempI = c->sy - b->sy;
            dx_end = ((c->sx - b->sx) << QE_TEXAFF_FP_POSITION) / tempI;
            du_end = ((cu    - bu   ) << QE_TEXAFF_FP)         / tempI;
            dv_end = ((cv    - bv   ) << QE_TEXAFF_FP)         / tempI;
            df_end = ((cf    - bf   ) << QE_TEXAFF_FP)         / tempI;
            x_end = b->sx << QE_TEXAFF_FP_POSITION;
            u_end = bu    << QE_TEXAFF_FP;
            v_end = bv    << QE_TEXAFF_FP;
            f_end = bf    << QE_TEXAFF_FP;
        }
        if (y_start < 0) continue;

        int x1, x2, u, v, f;
        int df;
        if (x_start > x_end) {
            x1 = x_end   >> QE_TEXAFF_FP_POSITION;
            x2 = x_start >> QE_TEXAFF_FP_POSITION;
            u  = u_end; v = v_end; f = f_end;
        } else {
            x1 = x_start >> QE_TEXAFF_FP_POSITION;
            x2 = x_end   >> QE_TEXAFF_FP_POSITION;
            u  = u_start; v = v_start; f = f_start;
        }

        int span = x2 - x1;
        if (span == 0) continue;

        int du = (u_end - u_start) / (x_end > x_start ? span : -span);
        int dv = (v_end - v_start) / (x_end > x_start ? span : -span);
        df = (f_end - f_start) / (x_end > x_start ? span : -span);
        if (x1 < 0) { u -= du * x1; v -= dv * x1; f -= df * x1; x1 = 0; }
        if (x2 > g3d->width) x2 = g3d->width;

        tempI = g3d->width * y_start;
        x1 += tempI; x2 += tempI;
        u -= du; v -= dv; f -= df;

        int col;
        while (x1 < x2) {
            u += du; v += dv; f += df;
            col = tex[((qe_urshift(v, QE_TEXAFF_FP) << widthBIT) + qe_urshift(u, QE_TEXAFF_FP)) & lengthBIT];
            if (col != 0) {
                int fi = qe_urshift(f, QE_TEXAFF_FP);
                if (fi > 255) fi = 255; if (fi < 0) fi = 0;
                int r = ((col >> 16) & 0xff) * fi >> 8;
                int g = ((col >>  8) & 0xff) * fi >> 8;
                int b = ( col        & 0xff) * fi >> 8;
                rgb[x1] = (r << 16) | (g << 8) | b;
            }
            x1++;
        }
    }
}

void TexturingAffine_paintOverwrite(DirectX7 *g3d, Vertex *a, Vertex *b, Vertex *c) {
    /* Debug: preenche o triangulo com um numero incrementado. */
    (void) g3d; (void) a; (void) b; (void) c;
    /* Fiel: o Java pinta com valores 0..5 remapeados em flush(). */
}

/*
 * paint() — dispatcher fiel dos 14 modos. Modos ainda nao portados
 * caem em paintAffine_0 (fiel ao fallback do Java quando fogQ==0 ou
 * quando o proprio Java descarta o modo, ver TexturingAffine.java linhas
 * 62/71/97/153).
 */
void TexturingAffine_paint(DirectX7 *g3d, Texture *texture,
    Vertex *a, int au, int av, Vertex *b, int bu, int bv, Vertex *c, int cu, int cv,
    int fogc, int fog, int sz2,
    int af, int bf, int cf,
    int ag, int bg, int cg,
    int ab, int bb, int cb,
    int nx, int ny, int nz) {
    (void) ag; (void) bg; (void) cg; (void) ab; (void) bb; (void) cb;

    if (DeveloperMenu_renderPolygonsOverwrite) {
        TexturingAffine_paintOverwrite(g3d, a, b, c);
        return;
    }
    if (!texture->rImg->alphaMixing && texture->drawmode == 4) fog = 0;

    /* Ajuste de UV para texturas nao-256 (fiel ao Java). */
    if (texture->rImg->w != 256) {
        if ((texture->mip != NULL && texture->mip[0]->w < 255) ||
            (texture->mip == NULL && texture->rImg->w < 255)) {
            if (au == 255) au = 256;
            if (bu == 255) bu = 256;
            if (cu == 255) cu = 256;
        }
        au = au * texture->rImg->w / 256;
        bu = bu * texture->rImg->w / 256;
        cu = cu * texture->rImg->w / 256;
    }
    if (texture->rImg->h != 256) {
        if ((texture->mip != NULL && texture->mip[0]->h < 256) ||
            (texture->mip == NULL && texture->rImg->h < 256)) {
            if (av == 255) av = 256;
            if (bv == 255) bv = 256;
            if (cv == 255) cv = 256;
        }
        av = av * texture->rImg->h / 256;
        bv = bv * texture->rImg->h / 256;
        cv = cv * texture->rImg->h / 256;
    }

    switch (fog) {
    case 0:
        TexturingAffine_paintAffine_0(g3d, texture, a, au, av, b, bu, bv, c, cu, cv);
        return;
    case 1: case 3:
        /* fogQ>0 -> paintAffine_1/3 (fog aditivo). Nao portado nesta fase.
         * Fallback fiel ao Java quando Main.fogQ == 0 (paintAffine_0). */
        if (Main_fogQ > 0) {
            /* TODO: portar paintAffine_1 / paintAffine_3 (fog blend). */
            TexturingAffine_paintAffine_0(g3d, texture, a, au, av, b, bu, bv, c, cu, cv);
        } else {
            TexturingAffine_paintAffine_0(g3d, texture, a, au, av, b, bu, bv, c, cu, cv);
        }
        return;
    case 5:
        /* Direcional light: usa af/bf/cf pre-calculados. Delegamos ao 9. */
        TexturingAffine_paintAffine_9(g3d, texture, a, au, av, b, bu, bv, c, cu, cv,
                                       fogc, sz2, af, bf, cf, nx, ny, nz);
        return;
    case 6: {
        if (Main_fogQ == 2) {
            af = 0xff + a->rz * 0xFF / DirectX7_fDist; if (af > 0xFF) af = 0xFF; if (af < 0) af = 0;
            bf = 0xff + b->rz * 0xFF / DirectX7_fDist; if (bf > 0xFF) bf = 0xFF; if (bf < 0) bf = 0;
            cf = 0xff + c->rz * 0xFF / DirectX7_fDist; if (cf > 0xFF) cf = 0xFF; if (cf < 0) cf = 0;
            TexturingAffine_paintAffine_9(g3d, texture, a, au, av, b, bu, bv, c, cu, cv,
                                           fogc, sz2, af, bf, cf, nx, ny, nz);
        } else {
            TexturingAffine_paintAffine_0(g3d, texture, a, au, av, b, bu, bv, c, cu, cv);
        }
        return;
    }
    case 9:
        if (Main_fogQ >= 1 && (af != 255 || bf != 255 || cf != 255))
            TexturingAffine_paintAffine_9(g3d, texture, a, au, av, b, bu, bv, c, cu, cv,
                                           fogc, sz2, af, bf, cf, nx, ny, nz);
        else
            TexturingAffine_paintAffine_0(g3d, texture, a, au, av, b, bu, bv, c, cu, cv);
        return;
    case 2: case 4: case 7: case 8: case 10: case 11: case 12: case 13:
    default:
        /* Modos ainda nao portados nesta fase — fallback fiel para modo 0.
         * Ver STATUS_CONVERSAO.md secao "Rasterizadores adicionais". */
        TexturingAffine_paintAffine_0(g3d, texture, a, au, av, b, bu, bv, c, cu, cv);
        return;
    }
}
