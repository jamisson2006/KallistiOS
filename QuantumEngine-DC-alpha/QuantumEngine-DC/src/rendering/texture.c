/*
 * texture.c — porte fiel de code/Rendering/Texture.java
 */
#include "texture.h"
#include "raw_image.h"
#include "directx7.h"

#include <stdlib.h>
#include <string.h>

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

/* external: Asset.getRawImage — do utils */
extern RawImage *Asset_getRawImage(const char *file);
extern int      *RawImage_getImg(const RawImage *self);

Texture *Texture_new(void) {
    Texture *t = (Texture*) calloc(1, sizeof(Texture));
    t->drawmode        = DirectX7_standartDrawmode;
    t->castShadow      = 1;
    t->collision       = 1;
    t->animation_speed = 1.0f;
    return t;
}

Texture *Texture_new_full(RawImage *ri, int perspectiveCorrection, ByteRawImage *bri) {
    Texture *t = Texture_new();
    Texture_setTexture(t, ri, perspectiveCorrection, bri);
    return t;
}

void Texture_free(Texture *self) { free(self); }

void Texture_setTexture(Texture *self, RawImage *ri, int perspectiveCorrection, ByteRawImage *bri) {
    self->perspectiveCorrection = perspectiveCorrection;
    self->rImg = ri;
    self->drawmode = DirectX7_standartDrawmode;
    self->brImg = bri;
}

int   *Texture_getPixels(const Texture *self)      { return RawImage_getImg(self->rImg); }
int8_t Texture_getDrawMode(const Texture *self)    { return self->drawmode; }
void   Texture_setDrawMode(Texture *self, int8_t dr){ self->drawmode = dr; }
void   Texture_setPerspectiveCorrection(Texture *self, int on) { self->perspectiveCorrection = on; }

Texture *Texture_createTexture(const char *file) {
    RawImage *ri = Asset_getRawImage(file);
    return Texture_new_full(ri, 0, NULL);
}

void Texture_updateAnimation(Texture *self) {
    if (self->animMIP == NULL) return;
    int64_t time = qe_now_ms();
    double range = self->animMIP_n * 1000.0 / self->animation_speed;
    if (range < 1) range = 1;
    time = (int64_t)(time - (int64_t)(range) * (int64_t)(time / (int64_t)range));
    int anim_index = (int)(time * self->animation_speed / 1000) % self->animMIP_n;

    self->rImg  = self->animMIP[anim_index]->rImg;
    self->mip   = self->animMIP[anim_index]->mip;
    self->mip_n = self->animMIP[anim_index]->mip_n;
    self->brMip = self->animMIP[anim_index]->brMip;
    self->brMip_n = self->animMIP[anim_index]->brMip_n;
}

void Texture_setMip(Texture *self, RawImage **mip, int n) {
    self->mip = mip; self->mip_n = n;
}
