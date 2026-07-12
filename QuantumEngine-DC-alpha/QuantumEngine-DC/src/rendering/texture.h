/*
 * texture.h — porte fiel de code/Rendering/Texture.java
 */
#ifndef QE_RENDERING_TEXTURE_H
#define QE_RENDERING_TEXTURE_H

#include <stdint.h>
#include "byte_raw_image.h"

typedef struct RawImage RawImage;

typedef struct Texture {
    int       perspectiveCorrection;
    RawImage *rImg;
    ByteRawImage *brImg;
    RawImage **mip;         int mip_n;
    RawImage **brMip;       int brMip_n;
    int8_t    drawmode;
    int       addsz;
    int       castShadow;   /* default 1 */
    int       collision;    /* default 1 */
    struct Texture **animMIP; int animMIP_n;
    float     animation_speed;
} Texture;

Texture *Texture_new(void);
Texture *Texture_new_full(RawImage *ri, int perspectiveCorrection, ByteRawImage *bri);
void     Texture_free(Texture *self);

void  Texture_setTexture(Texture *self, RawImage *ri, int perspectiveCorrection, ByteRawImage *bri);

int  *Texture_getPixels(const Texture *self);
int8_t Texture_getDrawMode(const Texture *self);
void  Texture_setDrawMode(Texture *self, int8_t dr);
void  Texture_setPerspectiveCorrection(Texture *self, int on);

Texture *Texture_createTexture(const char *file);

void  Texture_updateAnimation(Texture *self);

/* Usado por Asset e MultyTexture: */
void  Texture_setMip(Texture *self, RawImage **mip, int n);

/* Array de texturas (usado por Mesh) — no port cada Mesh mantem seu proprio;
 * este acessor esta em Mesh. */

#endif
