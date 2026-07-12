/*
 * raw_image.h — porte fiel de code/Rendering/RawImage.java
 */
#ifndef QE_RENDERING_RAW_IMAGE_H
#define QE_RENDERING_RAW_IMAGE_H

#include <stdint.h>

typedef struct RawImage {
    int32_t *img;
    int      w, h;
    int8_t   scale;      /* default 2 */
    int      isPalette;
    int      widthBIT;
    int      widthBITmode10;
    int      W_UNIT;
    int      alphaMixing;
} RawImage;

RawImage *RawImage_new(int32_t *rgb, int w, int h);
void      RawImage_free(RawImage *self);

/* Carrega imagem (PNG via stb_image no port DC, ou .qct proprietario). */
RawImage *RawImage_createRawImage(const char *file);
RawImage *RawImage_createMipRawImage(RawImage *base);

/* accessors compat com Asset (utils/asset.c) */
int      RawImage_scale     (const RawImage *self);
int      RawImage_isPalette (const RawImage *self);
void    *RawImage_img       (const RawImage *self);      /* void* = int32_t* */
void     RawImage_setImg    (RawImage *self, void *img);
int      RawImage_w         (const RawImage *self);
int      RawImage_h         (const RawImage *self);
void     RawImage_setW      (RawImage *self, int w);
void     RawImage_setH      (RawImage *self, int h);
void     RawImage_setScale  (RawImage *self, int scale);
int     *RawImage_getImg    (const RawImage *self);

#endif
