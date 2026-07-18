/*
 * byte_raw_image.h — porte fiel de code/Rendering/ByteRawImage.java
 */
#ifndef QE_RENDERING_BYTE_RAW_IMAGE_H
#define QE_RENDERING_BYTE_RAW_IMAGE_H

#include <stdint.h>

typedef struct ByteRawImage {
    uint8_t *img;
    int      w, h;
    int8_t   scale;   /* default 2 */
} ByteRawImage;

ByteRawImage *ByteRawImage_new(uint8_t *rgb, int w, int h);
void          ByteRawImage_free(ByteRawImage *self);

#endif
