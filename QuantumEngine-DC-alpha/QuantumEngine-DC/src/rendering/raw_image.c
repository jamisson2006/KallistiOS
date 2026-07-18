/*
 * raw_image.c — porte fiel de code/Rendering/RawImage.java
 *
 * PNG carregado via stb_image (vendor). Formato .qct proprietario portado
 * fielmente (DXT-like customizado do Quantum). Nao portamos a classe
 * TextureCompressor pois o Java tambem so a usa em builds especificos
 * (comentario "TextureCompressor.compress" esta comentado no original).
 */
#include "raw_image.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../utils/image_resize.h"

extern int Main_pixelsQ;
extern int Asset_desizeSomething(void);

/* stb_image (vendor/) */
#define STB_IMAGE_IMPLEMENTATION_ONCE
#ifdef STB_IMAGE_IMPLEMENTATION_ONCE
/* deixamos a impl a cargo do modulo render/raw_image antigo; aqui declaramos: */
extern unsigned char *stbi_load(const char *filename, int *x, int *y, int *comp, int req_comp);
extern void           stbi_image_free(void *retval_from_stbi_load);
#endif

/* static error singleton */
static RawImage *g_error = NULL;

static RawImage *qe_error_image(void) {
    if (g_error) return g_error;
    int32_t *px = (int32_t*) malloc(sizeof(int32_t) * 4);
    px[0] = px[1] = px[2] = px[3] = 0xffff0000;
    g_error = RawImage_new(px, 2, 2);
    return g_error;
}

static int widthToBIT(int w) {
    for (int i = 0; i < 32; i++)
        if ((w >> i) == 1 && (1 << i) == w) return i;
    return 0;
}
static int wUnitGen(int w) {
    if (w <=  256) return 1 << 30;
    if (w <=  512) return 1 << 29;
    if (w <= 1024) return 1 << 28;
    if (w <= 2048) return 1 << 27;
    if (w <= 4096) return 1 << 26;
    if (w <= 8192) return 1 << 25;
    return 1 << 24;
}
static int prepareAlpha(int32_t *pix, int len) {
    int alphaMixing = 0;
    for (int i = 0; i < len; i++) {
        int alpha = (int)((uint32_t) pix[i] >> 24) & 0xff;
        if (alpha > 0 && alpha < 255) alphaMixing = 1;
        if (alpha == 0) pix[i] = 0;
    }
    return alphaMixing;
}

RawImage *RawImage_new(int32_t *rgb, int w, int h) {
    RawImage *r = (RawImage*) calloc(1, sizeof(RawImage));
    r->img = rgb; r->w = w; r->h = h;
    r->scale = 2;
    r->widthBIT       = widthToBIT(w);
    r->widthBITmode10 = widthToBIT(w * w);
    r->W_UNIT         = wUnitGen(w > h ? w : h);
    r->alphaMixing    = prepareAlpha(rgb, w * h);
    return r;
}
void RawImage_free(RawImage *self) {
    if (!self) return;
    free(self->img);
    free(self);
}

/* PNG via stb_image */
static RawImage *imageToRawImage(const char *file, int size) {
    char path[512];
    if (file[0] == '/') snprintf(path, sizeof path, "/rd%s", file);
    else                snprintf(path, sizeof path, "/rd/%s", file);

    int w, h, ch;
    unsigned char *px = stbi_load(path, &w, &h, &ch, 4);
    if (!px) {
        fprintf(stderr, "ERROR create image %s\n", file);
        return qe_error_image();
    }

    /* Converte RGBA -> ARGB (Java-style 0xAARRGGBB) */
    int32_t *img = (int32_t*) malloc(sizeof(int32_t) * w * h);
    for (int i = 0; i < w * h; i++) {
        unsigned char R = px[i*4+0], G = px[i*4+1], B = px[i*4+2], A = px[i*4+3];
        img[i] = ((int32_t)A << 24) | ((int32_t)R << 16) | ((int32_t)G << 8) | B;
    }
    stbi_image_free(px);

    int scale = size;
    if (scale > Main_pixelsQ) scale = Main_pixelsQ;
    if (Main_pixelsQ == 0 && w > 1 && h > 1) {
        int32_t *img2 = ImageResize_cubic2XDesize(img, w, h);
        free(img); img = img2;
        w /= 2; h /= 2; scale = 0;
    } else if (Main_pixelsQ != 2 && h > 1) {
        int32_t *img2 = ImageResize_cubic2XVertDesize(img, w, h);
        free(img); img = img2;
        h /= 2; scale = 1;
    }

    RawImage *ri = RawImage_new(img, w, h);
    ri->scale = (int8_t) scale;
    return ri;
}

/* QCT (proprietary) — big-endian data stream */
static RawImage *qctToRawImage(const char *file) {
    char path[512];
    if (file[0] == '/') snprintf(path, sizeof path, "/rd%s", file);
    else                snprintf(path, sizeof path, "/rd/%s", file);

    FILE *fp = fopen(path, "rb");
    if (!fp) { fprintf(stderr, "ERROR in qctToRawImage: %s\n", file); return qe_error_image(); }

    unsigned char hdr[128];
    if (fread(hdr, 1, 3, fp) != 3) { fclose(fp); return qe_error_image(); }
    int headerSize = fgetc(fp) << 8; headerSize |= fgetc(fp);
    fgetc(fp); /* Format version */
    int w = 0; w = fgetc(fp) << 8; w |= fgetc(fp);
    int h = 0; h = fgetc(fp) << 8; h |= fgetc(fp);
    int colorCount = 1 << (fgetc(fp) + 1);
    int alphaType  = fgetc(fp);
    headerSize -= 7;

    int lastColorReplace = 0;
    if (alphaType >= 1) { lastColorReplace = fgetc(fp); headerSize--; }
    (void) lastColorReplace;

    if (headerSize > 0) fseek(fp, headerSize, SEEK_CUR);

    int32_t *img    = (int32_t*) calloc(w * h, sizeof(int32_t));
    int32_t *colors = (int32_t*) calloc(colorCount, sizeof(int32_t));

    for (int i = 0; i < colorCount; i++) {
        int r = fgetc(fp), g = fgetc(fp), b = fgetc(fp);
        colors[i] = 0xff000000 | (r << 16) | (g << 8) | b;
    }

    int32_t tmpColors[4];

    for (int y = 0; y < h / 2; y++) {
        int yp = y * w << 1;
        for (int x = 0; x < w / 2; x++) {
            int xp = yp + (x << 1);
            int c0id = fgetc(fp), c3id = fgetc(fp);
            int hasAlpha = c0id > c3id;
            tmpColors[0] = colors[c0id]; tmpColors[3] = colors[c3id];
            int rb0 = tmpColors[0] & 0xff00ff;
            int rb3 = tmpColors[3] & 0xff00ff;
            int g0  = tmpColors[0] & 0x00ff00;
            int g3  = tmpColors[3] & 0x00ff00;

            if (hasAlpha) {
                tmpColors[2] = 0;
                tmpColors[1] = (
                    (((rb0 + rb3) & 0x1fe01de) |
                     ((g0  + g3 ) & 0x001fe00)) >> 1) | 0xff000000;
            } else {
                tmpColors[1] = (
                    (((rb0*171 + rb3*85) & 0xff00ff00) |
                     ((g0 *171 + g3 *85) & 0x00ff0000)) >> 8) | 0xff000000;
                tmpColors[2] = (
                    (((rb0*85 + rb3*171) & 0xff00ff00) |
                     ((g0 *85 + g3 *171) & 0x00ff0000)) >> 8) | 0xff000000;
            }

            int ids = (int8_t) fgetc(fp);
            img[xp    ]     = tmpColors[ ids       & 3];
            img[xp + 1]     = tmpColors[((uint32_t)ids >> 2) & 3];
            img[xp + w]     = tmpColors[((uint32_t)ids >> 4) & 3];
            img[xp + w + 1] = tmpColors[((uint32_t)ids >> 6) & 3];
        }
    }

    if (alphaType == 2) {
        for (int i = 0; i < w * h; i++) {
            if (((uint32_t)img[i] >> 24) == 0) continue;

            int solid = fgetc(fp), transparent = fgetc(fp);
            i += solid;
            if (transparent == 0) { i--; continue; }
            int c = 0;
            for (; i < w * h; i++) {
                if (((uint32_t)img[i] >> 24) == 0) continue;
                img[i] = (fgetc(fp) << 24) | (img[i] & 0xffffff);
                c++;
                if (c == transparent) break;
            }
        }
    }

    if (Main_pixelsQ == 0) { int32_t *n = ImageResize_cubic2XDesize    (img, w, h); free(img); img = n; w/=2; h/=2; }
    if (Main_pixelsQ == 1) { int32_t *n = ImageResize_cubic2XVertDesize(img, w, h); free(img); img = n; h/=2; }

    RawImage *ri = RawImage_new(img, w, h);
    ri->scale = (int8_t) Main_pixelsQ;
    free(colors);
    fclose(fp);
    return ri;
}

RawImage *RawImage_createRawImage(const char *file) {
    /* Determina formato pela extensao (Java: substring(indexOf('.')+1). */
    const char *dot = strrchr(file, '.');
    if (dot) {
        char fmt[16];
        int i = 0;
        for (dot++; *dot && i + 1 < (int) sizeof fmt; dot++, i++) fmt[i] = (char) tolower((unsigned char) *dot);
        fmt[i] = 0;
        if (strcmp(fmt, "qct") == 0) return qctToRawImage(file);
    }
    return imageToRawImage(file, 2);
}

RawImage *RawImage_createMipRawImage(RawImage *base) {
    if (base->w < 2 || base->h < 2) return NULL;
    int32_t *img = ImageResize_mipMap(base->img, base->w, base->h);
    if (!img) return NULL;
    return RawImage_new(img, base->w / 2, base->h / 2);
}

/* accessors */
int   RawImage_scale     (const RawImage *self) { return self->scale; }
int   RawImage_isPalette (const RawImage *self) { return self->isPalette; }
void *RawImage_img       (const RawImage *self) { return self->img; }
void  RawImage_setImg    (RawImage *self, void *img) { self->img = (int32_t*) img; }
int   RawImage_w         (const RawImage *self) { return self->w; }
int   RawImage_h         (const RawImage *self) { return self->h; }
void  RawImage_setW      (RawImage *self, int w) { self->w = w; }
void  RawImage_setH      (RawImage *self, int h) { self->h = h; }
void  RawImage_setScale  (RawImage *self, int s) { self->scale = (int8_t) s; }
int  *RawImage_getImg    (const RawImage *self) { return self->img; }
