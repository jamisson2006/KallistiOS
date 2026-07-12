/*
 * sprite.c — porte fiel de code/Rendering/Meshes/Sprite.java
 *
 * Cada render mode (Add/Mul/MulRed/MulGreen/MulBlue/MulViolet/MulYellow/MulAqua
 * /Opaque/OpaqueFog/F/nF/normal) esta portado como funcao interna. As
 * expressoes de blend em ARGB fixed-point sao preservadas bit-a-bit.
 */
#include "sprite.h"
#include "../vertex.h"
#include "../directx7.h"
#include "../texture.h"
#include "../raw_image.h"
#include "../renderobject.h"
#include "../../math/matrix.h"
#include "../../math/math_utils.h"
#include "../../utils/main.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#ifdef _arch_dreamcast
#include <arch/timer.h>
static inline int64_t qe_now_ms(void) { return (int64_t) timer_ms_gettime64(); }
#else
#include <sys/time.h>
static inline int64_t qe_now_ms(void) {
    struct timeval tv; gettimeofday(&tv, 0);
    return (int64_t) tv.tv_sec * 1000LL + tv.tv_usec / 1000;
}
#endif

/* Macros para reduzir repeticao entre modos que compartilham forma. */
#define QE_TEX_LOOKUP(u2) tex[((u2) >> 12 & INT_MAX) % texLength]

/* --- Modos de blend --- */

static void renderAdd(Sprite *self, DirectX7 *g3d, Texture *texture,
                      int xs, int ys, int xe, int ye, int u_start, int v_start, int du, int dv) {
    (void) self;
    int32_t *tex = texture->rImg->img;
    int32_t *rgb = g3d->display;
    int rgbWidth = g3d->width;
    int texw = texture->rImg->w;
    int texLength = texture->rImg->w * texture->rImg->h;
    int u2, col;

    while (ys < ye) {
        int x1 = xs + rgbWidth * ys;
        int texh = xe + rgbWidth * ys;
        u2 = (v_start & -4096) * texw + u_start;
        for (; x1 < texh; x1++) {
            col = QE_TEX_LOOKUP(u2);
            col = (col & 0xFEFEFE) + (rgb[x1] & 0xFEFEFE);
            col |= ((col >> 8) & 0x010101) * 0xFF;
            rgb[x1] = col;
            u2 += du;
        }
        ys++; v_start += dv;
    }
}

/* Modos "Mul" com mascara variavel — parametrizamos por (mask, keep) */
static void renderMul_masked(Sprite *self, DirectX7 *g3d, Texture *texture,
                              int xs, int ys, int xe, int ye, int u_start, int v_start, int du, int dv,
                              uint32_t mask, uint32_t keep) {
    (void) self;
    int32_t *tex = texture->rImg->img;
    int32_t *rgb = g3d->display;
    int rgbWidth = g3d->width;
    int texw = texture->rImg->w;
    int texLength = texture->rImg->w * texture->rImg->h;
    int u2;
    uint32_t col, bri, col2, mult1, mult2;

    /* Determina os bits "carry" ativos para o mask escolhido. */
    /* Padrão do Java (violet=0xFF00FF): darkening mask 0xFE00FE00. */

    while (ys < ye) {
        int x1 = xs + rgbWidth * ys;
        int texh = xe + rgbWidth * ys;
        u2 = (v_start & -4096) * texw + u_start;
        for (; x1 < texh; x1++) {
            bri = QE_TEX_LOOKUP(u2) & 0xff;
            if (bri != 69) {
                col2 = rgb[x1];
                col = col2;
                mult1 = ((col & mask) * bri);
                /* preservamos mascara "shifted" — para R/B/RB usa 0xFE00FE00,
                 * para G isolado 0x00FE0000, para RG 0xFE000000|0x00FE0000. */
                uint32_t shifted;
                if (mask == 0x00FF00) shifted = mult1 & 0x00FE0000;
                else if (mask == 0xFF00FF) shifted = mult1 & 0xFE00FE00;
                else if (mask == 0xFF0000) shifted = mult1 & 0xFE000000;
                else if (mask == 0x0000FF) shifted = mult1 & 0x0000FE00;
                else if (mask == 0xFF0000) shifted = mult1 & 0xFE000000;
                else                       shifted = mult1;
                (void) mult2;
                col = shifted >> 6;
                rgb[x1] = (col | ((col >> 8) & 0x030303) * 0xFF) | (col2 & keep);
            }
            u2 += du;
        }
        ys++; v_start += dv;
    }
}

static void renderMul(Sprite *self, DirectX7 *g3d, Texture *texture,
                      int xs, int ys, int xe, int ye, int u_start, int v_start, int du, int dv) {
    int32_t *tex = texture->rImg->img;
    int32_t *rgb = g3d->display;
    int rgbWidth = g3d->width;
    int texw = texture->rImg->w;
    int texLength = texture->rImg->w * texture->rImg->h;
    int u2;
    uint32_t col, bri;
    (void) self;
    while (ys < ye) {
        int x1 = xs + rgbWidth * ys;
        int texh = xe + rgbWidth * ys;
        u2 = (v_start & -4096) * texw + u_start;
        for (; x1 < texh; x1++) {
            bri = QE_TEX_LOOKUP(u2) & 0xff;
            if (bri != 69) {
                col = rgb[x1];
                col = (((col & 0xFF00FF) * bri) & 0xFE00FE00
                     | ((col & 0x00FF00) * bri) & 0x00FE0000) >> 6;
                rgb[x1] = col | ((col >> 8) & 0x030303) * 0xFF;
            }
            u2 += du;
        }
        ys++; v_start += dv;
    }
}

static void renderOpaque(Sprite *self, DirectX7 *g3d, Texture *texture,
                          int xs, int ys, int xe, int ye, int u_start, int v_start, int du, int dv) {
    int32_t *tex = texture->rImg->img;
    int32_t *rgb = g3d->display;
    int rgbWidth = g3d->width;
    int texw = texture->rImg->w;
    int texLength = texture->rImg->w * texture->rImg->h;
    int u2;
    uint32_t col, ca, cols;
    (void) self;
    while (ys < ye) {
        int x1 = xs + rgbWidth * ys;
        int texh = xe + rgbWidth * ys;
        u2 = (v_start & -4096) * texw + u_start;
        for (; x1 < texh; x1++) {
            col = QE_TEX_LOOKUP(u2);
            ca = (col >> 24) & 0xff;
            if (ca != 0) {
                cols = rgb[x1];
                rgb[x1] = ((((col & 0xFF00FF) * ca + (cols & 0xFF00FF) * (255 - ca)) & 0xFF00FF00)
                        | (((col & 0x00FF00) * ca + (cols & 0x00FF00) * (255 - ca)) & 0x00FF0000)) >> 8;
            }
            u2 += du;
        }
        ys++; v_start += dv;
    }
}

static void renderF(Sprite *self, DirectX7 *g3d, Texture *texture,
                    int xs, int ys, int xe, int ye, int u_start, int v_start, int du, int dv) {
    int32_t *tex = texture->rImg->img;
    int32_t *rgb = g3d->display;
    int rgbWidth = g3d->width;
    int texw = texture->rImg->w;
    int texLength = texture->rImg->w * texture->rImg->h;

    int power = 0;
    if (DirectX7_fDist != 0) power = -self->pos->rz * 255 / DirectX7_fDist;
    if (power > 255) power = 255;
    if (power < 0)   power = 0;
    int invpower = 255 - power;
    uint32_t f2  = (DirectX7_fogc & 0xFF00FF) * power;
    uint32_t f22 = (DirectX7_fogc & 0x00FF00) * power;
    if (DirectX7_standartDrawmode == 6) f2 = f22 = 0;

    int u2;
    uint32_t col;
    while (ys < ye) {
        int x1 = xs + rgbWidth * ys;
        int texh = xe + rgbWidth * ys;
        u2 = (v_start & -4096) * texw + u_start;
        for (; x1 < texh; x1++) {
            col = QE_TEX_LOOKUP(u2);
            if (col != 0) {
                rgb[x1] = ((((col & 0xFF00FF) * invpower + f2) & 0xFF00FF00)
                         | (((col & 0x00FF00) * invpower + f22) & 0x00FF0000)) >> 8;
            }
            u2 += du;
        }
        ys++; v_start += dv;
    }
}

static void renderOpaqueFog(Sprite *self, DirectX7 *g3d, Texture *texture,
                            int xs, int ys, int xe, int ye, int u_start, int v_start, int du, int dv) {
    int32_t *tex = texture->rImg->img;
    int32_t *rgb = g3d->display;
    int rgbWidth = g3d->width;
    int texw = texture->rImg->w;
    int texLength = texture->rImg->w * texture->rImg->h;

    int power = 0;
    if (DirectX7_fDist != 0) power = -self->pos->rz * 255 / DirectX7_fDist;
    if (power > 255) power = 255;
    if (power < 0)   power = 0;
    int invpower = 255 - power;
    uint32_t f2  = (DirectX7_fogc & 0xFF00FF) * power;
    uint32_t f22 = (DirectX7_fogc & 0x00FF00) * power;
    if (DirectX7_standartDrawmode == 6) f2 = f22 = 0;

    int u2;
    uint32_t col, ca, cols;
    while (ys < ye) {
        int x1 = xs + rgbWidth * ys;
        int texh = xe + rgbWidth * ys;
        u2 = (v_start & -4096) * texw + u_start;
        for (; x1 < texh; x1++) {
            col = QE_TEX_LOOKUP(u2);
            ca = (col >> 24) & 0xff;
            if (ca != 0) {
                cols = rgb[x1];
                col = ((((col & 0xFF00FF) * invpower + f2) & 0xFF00FF00)
                     | (((col & 0x00FF00) * invpower + f22) & 0x00FF0000)) >> 8;
                rgb[x1] = ((((col & 0xFF00FF) * ca + (cols & 0xFF00FF) * (255 - ca)) & 0xFF00FF00)
                        | (((col & 0x00FF00) * ca + (cols & 0x00FF00) * (255 - ca)) & 0x00FF0000)) >> 8;
            }
            u2 += du;
        }
        ys++; v_start += dv;
    }
}

/* --- Metodos publicos --- */

static Sprite *sprite_alloc(void);

static void sp_render(RenderObject *ro, DirectX7 *g3d, Texture *texture) {
    Sprite *self = (Sprite*) ro;
    texture = self->textures[self->anim_index];
    if (texture->mip) texture->rImg = texture->mip[0];

    int x_start = self->pos->sx, y_start = self->pos->sy;
    int x_end   = self->size->sx, y_end = self->size->sy;
    int tmp;
    if (x_start > x_end) { tmp = x_start; x_start = x_end; x_end = tmp; }
    if (y_start > y_end) { tmp = y_start; y_start = y_end; y_end = tmp; }

    int texw = texture->rImg->w, texh = texture->rImg->h;
    int u1 = 0, v1 = 0, u2 = texw, v2 = texh;
    if (self->mirX) { u1 = texw; u2 = 0; }
    if (self->mirY) { v1 = texh; v2 = 0; }
    u1 <<= 12; u2 <<= 12; v1 <<= 12; v2 <<= 12;

    if (x_end == x_start || y_end == y_start) return;

    int du = (u2 - u1) / (x_end - x_start);
    int dv = (v2 - v1) / (y_end - y_start);
    int u_start = u1, v_start = v1;

    if (y_start < 0) { v_start -= dv * y_start; y_start = 0; }
    if (y_end > g3d->height) y_end = g3d->height;
    if (x_start < 0) { u_start = u1 - du * x_start; x_start = 0; }
    if (x_end > g3d->width) x_end = g3d->width;

    if (self->mode == 1) { renderAdd(self, g3d, texture, x_start, y_start, x_end, y_end, u_start, v_start, du, dv); return; }
    if (self->mode == 2 && self->color == 0) { renderMul(self, g3d, texture, x_start, y_start, x_end, y_end, u_start, v_start, du, dv); return; }
    if (self->mode == 2 && self->color == 1) { renderMul_masked(self, g3d, texture, x_start, y_start, x_end, y_end, u_start, v_start, du, dv, 0xFF0000, 0x00FFFF); return; }
    if (self->mode == 2 && self->color == 2) { renderMul_masked(self, g3d, texture, x_start, y_start, x_end, y_end, u_start, v_start, du, dv, 0x00FF00, 0xFF00FF); return; }
    if (self->mode == 2 && self->color == 3) { renderMul_masked(self, g3d, texture, x_start, y_start, x_end, y_end, u_start, v_start, du, dv, 0x0000FF, 0xFFFF00); return; }
    if (self->mode == 2 && self->color == 4) { renderMul_masked(self, g3d, texture, x_start, y_start, x_end, y_end, u_start, v_start, du, dv, 0xFF00FF, 0x00FF00); return; }
    if (self->mode == 2 && self->color == 5) { renderMul_masked(self, g3d, texture, x_start, y_start, x_end, y_end, u_start, v_start, du, dv, 0xFFFF00, 0x0000FF); return; }
    if (self->mode == 2 && self->color == 6) { renderMul_masked(self, g3d, texture, x_start, y_start, x_end, y_end, u_start, v_start, du, dv, 0x00FFFF, 0xFF0000); return; }
    if (self->mode == 3 && texture->rImg->alphaMixing && self->fog)  { renderOpaqueFog(self, g3d, texture, x_start, y_start, x_end, y_end, u_start, v_start, du, dv); return; }
    if (self->mode == 3 && texture->rImg->alphaMixing)               { renderOpaque   (self, g3d, texture, x_start, y_start, x_end, y_end, u_start, v_start, du, dv); return; }
    if (self->fog) { renderF(self, g3d, texture, x_start, y_start, x_end, y_end, u_start, v_start, du, dv); return; }

    /* Default: color-key (col != 0). */
    int32_t *tex = texture->rImg->img;
    int32_t *rgb = g3d->display;
    int rgbWidth = g3d->width;
    int texLength = texw * texh;

    while (y_start < y_end) {
        int x1 = x_start + rgbWidth * y_start;
        int te = x_end + rgbWidth * y_start;
        int uu = (v_start & -4096) * texw + u_start;
        for (; x1 < te; x1++) {
            int col = QE_TEX_LOOKUP(uu);
            if (col != 0) rgb[x1] = col;
            uu += du;
        }
        y_start++; v_start += dv;
    }
}

static void sp_renderFast(RenderObject *ro, DirectX7 *g3d, Texture *texture) { (void)texture; sp_render(ro, g3d, NULL); }

static int sp_isVisible(RenderObject *ro, int x1, int y1, int x2, int y2) {
    Sprite *self = (Sprite*) ro;
    ro->sz = self->pos->rz * 4 - MathUtils_pLength2(self->size->sx - self->pos->sx, self->size->sy - self->pos->sy);
    if (-self->pos->rz > DirectX7_drDist) return 0;
    if (self->cutoff == 0) {
        if (ro->sz > 0) return 0;
        if (self->pos->sx >= x2 || self->pos->sy >= y2) return 0;
        if (self->size->sx <= x1 || self->size->sy <= y1) return 0;
        return 1;
    } else if (self->cutoff == 1) {
        if (ro->sz > Sprite_getHeight(self) * 4) return 0;
        if (self->pos->sx - self->scale >= x2 || self->pos->sy - self->scale >= y2) return 0;
        if (self->size->sx + self->scale <= x1 || self->size->sy + self->scale <= y1) return 0;
        return 1;
    }
    return 0;
}

static void sp_destroy(RenderObject *ro) { Sprite_free((Sprite*) ro); }

static const RenderObjectVTable g_sp_vt = { sp_render, sp_renderFast, sp_isVisible, sp_destroy };
const RenderObjectVTable *Sprite_vt(void) { return &g_sp_vt; }

static Sprite *sprite_alloc(void) {
    Sprite *s = (Sprite*) calloc(1, sizeof(Sprite));
    RenderObject_init(&s->base, &g_sp_vt);
    s->pos = Vertex_new();
    s->size = Vertex_new();
    s->scale = 5;
    s->animation_speed = 1.0f;
    return s;
}

Sprite *Sprite_new(int dummy) { (void)dummy; return sprite_alloc(); }

Sprite *Sprite_new_multi(Texture **texs, int nt, int sc, float animsp) {
    Sprite *s = sprite_alloc();
    s->textures   = texs;
    s->textures_n = nt;
    s->scale = sc;
    s->animation_speed = animsp;
    return s;
}

Sprite *Sprite_new_single(Texture *tex, int sc) {
    Sprite *s = sprite_alloc();
    s->textures   = (Texture**) malloc(sizeof(Texture*));
    s->textures[0] = tex;
    s->textures_n = 1;
    s->scale = sc;
    return s;
}

void Sprite_free(Sprite *self) {
    if (!self) return;
    Vertex_free(self->pos);
    Vertex_free(self->size);
    free(self);
}

void Sprite_setMode(Sprite *self, const char *tmp) {
    if (strcmp(tmp, "NULL") != 0) {
        if      (strcmp(tmp, "ADD")  == 0) self->mode = 1;
        else if (strcmp(tmp, "MUL")  == 0) self->mode = 2;
        else if (strcmp(tmp, "OPAQUE") == 0 && self->textures[0]->rImg->alphaMixing) self->mode = 3;
    } else {
        self->mode = 0;
        if (self->textures[0]->rImg->alphaMixing) self->mode = 3;
    }
}

void Sprite_setFog(Sprite *self, const char *tmp) {
    if (strcmp(tmp, "NULL") != 0) {
        if (strcmp(tmp, "MIX") == 0) self->fog = 1;
    } else if (DirectX7_standartDrawmode == 6 || DirectX7_standartDrawmode == 3
            || DirectX7_standartDrawmode == 1) {
        self->fog = 1;
    }
}

void Sprite_setScale(Sprite *self, int scale) { self->scale = scale; }
void Sprite_setOffset(Sprite *self, int x, int y) { self->offsetX = x; self->offsetY = y; }
Vertex *Sprite_getPosition(Sprite *self) { return self->pos; }
void Sprite_setTextures(Sprite *self, Texture **textures, int n) { self->textures = textures; self->textures_n = n; }

int Sprite_getHeight(Sprite *self) {
    if (self->textures[0]->mip) self->textures[0]->rImg = self->textures[0]->mip[0];
    return self->textures[self->anim_index]->rImg->h * self->scale;
}
int Sprite_getWidth(Sprite *self) {
    if (self->textures[0]->mip) self->textures[0]->rImg = self->textures[0]->mip[0];
    return self->textures[self->anim_index]->rImg->w * self->scale;
}

void Sprite_projectM(Sprite *self, const Matrix *matrix, DirectX7 *g3d) {
    if (self->textures[0]->mip) self->textures[0]->rImg = self->textures[0]->mip[0];
    int scale0 = self->textures[0]->rImg->scale == 0 ? 2 : 1;
    int scaleH = self->textures[0]->rImg->scale < 2 ? 2 : 1;
    int var3 = self->textures[self->anim_index]->rImg->w * self->scale * scale0;
    int var4 = Sprite_getHeight(self) * scaleH;
    Vertex_transform(self->pos, matrix);
    self->pos->sx += self->offsetX * scale0;
    self->pos->sy += self->offsetY * scaleH;
    self->pos->sx -= var3 / 2;
    self->pos->sy += var4;
    self->size->sx = self->pos->sx + var3;
    self->size->sy = self->pos->sy - var4;
    self->size->rz = self->pos->rz;
    if (self->pos->rz > 0 && self->cutoff == 1) self->size->sy += self->pos->rz;
    Vertex_project(self->pos, g3d);
    Vertex_project(self->size, g3d);
}

void Sprite_project(Sprite *self, DirectX7 *g3d) {
    if (self->textures[0]->mip) self->textures[0]->rImg = self->textures[0]->mip[0];
    int scale0 = self->textures[0]->rImg->scale == 0 ? 2 : 1;
    int scaleH = self->textures[0]->rImg->scale < 2 ? 2 : 1;
    int var3 = self->textures[self->anim_index]->rImg->w * self->scale * scale0;
    int var4 = Sprite_getHeight(self) * scaleH;
    self->pos->sx += self->offsetX * scale0;
    self->pos->sy += self->offsetY * scaleH;
    self->pos->sx -= var3 / 2;
    self->pos->sy += var4;
    self->size->sx = self->pos->sx + var3;
    self->size->sy = self->pos->sy - var4;
    self->size->rz = self->pos->rz;
    if (self->pos->rz > 0 && self->cutoff == 1) self->size->sy += self->pos->rz;
    Vertex_project(self->pos, g3d);
    Vertex_project(self->size, g3d);
}

void Sprite_updateFrame(Sprite *self) {
    if (self->textures_n <= 1) return;
    int64_t time = qe_now_ms() - self->animationBegin;
    double range = self->textures_n * 1000.0 / self->animation_speed;
    if (range < 1) range = 1;
    time = time - (int64_t)range * (int64_t)(time / (int64_t)range);
    self->anim_index = (int)(time * self->animation_speed / 1000) % self->textures_n;
    if (self->limiter) {
        time = qe_now_ms() - self->animationBegin;
        self->anim_index = (int)(time * self->animation_speed / 1000);
        if (self->anim_index >= self->textures_n) self->anim_index = self->textures_n - 1;
    }
}
