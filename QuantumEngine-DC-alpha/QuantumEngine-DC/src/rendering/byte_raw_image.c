#include "byte_raw_image.h"
#include <stdlib.h>

ByteRawImage *ByteRawImage_new(uint8_t *rgb, int w, int h) {
    ByteRawImage *b = (ByteRawImage*) calloc(1, sizeof(ByteRawImage));
    b->img = rgb; b->w = w; b->h = h; b->scale = 2;
    return b;
}
void ByteRawImage_free(ByteRawImage *self) { free(self); }
