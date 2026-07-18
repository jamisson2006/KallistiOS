/*
 * image_resize.c — porte fiel de code/utils/ImageResize.java (Quantum Engine J2ME)
 *
 * Todas as expressoes aritmeticas (>>>, & FP_MASK, etc.) sao mantidas o
 * mais literalmente possivel para preservar o comportamento bit-a-bit do
 * original — inclusive quirks como o downscale "2XHor" que multiplica
 * (w*2+1) por linha (bug fiel do original).
 */
#include "image_resize.h"

#include <stdint.h>
#include <stdlib.h>

/* No Java, ">>>" eh shift logico unsigned. Em C, cast para uint. */
static inline uint32_t URSHIFT(int32_t v, int s) { return ((uint32_t) v) >> s; }

#define FP_SHIFT 16
#define FP_MASK  ((1 << FP_SHIFT) - 1)

void ImageResize_bilinearResizeRGB(const int32_t *srcPixels, int32_t *destPixels,
                                   int srcw, int srch, int destw, int desth, int alpha) {
    int x, y, rx, ry, ix, iy, fx, fy;
    int idx1, idx2, idx3, idx4;
    int a, r, g, b;

    if (alpha) {
        for (x = 0; x < destw; x++) {
            for (y = 0; y < desth; y++) {
                rx = (x << FP_SHIFT) / destw * srcw;
                ry = (y << FP_SHIFT) / desth * srch;

                ix = (int) URSHIFT(rx, FP_SHIFT);
                iy = (int) URSHIFT(ry, FP_SHIFT);

                fx = rx & FP_MASK;
                fy = ry & FP_MASK;

                idx1 = idx2 = idx3 = idx4 = ix + iy * srcw;

                if (ix < srcw - 1) { idx2++; idx4++; }
                if (iy < srch - 1) { idx3 += srcw; idx4 += srcw; }

                a = (int)((((((URSHIFT(srcPixels[idx1], 24) & 0xFF) * (FP_MASK - fx) + (URSHIFT(srcPixels[idx2], 24) & 0xFF) * fx) >> FP_SHIFT) * (FP_MASK - fy) +
                            (((URSHIFT(srcPixels[idx3], 24) & 0xFF) * (FP_MASK - fx) + (URSHIFT(srcPixels[idx4], 24) & 0xFF) * fx) >> FP_SHIFT) * fy) >> FP_SHIFT));
                r = (int)((((((URSHIFT(srcPixels[idx1], 16) & 0xFF) * (FP_MASK - fx) + (URSHIFT(srcPixels[idx2], 16) & 0xFF) * fx) >> FP_SHIFT) * (FP_MASK - fy) +
                            (((URSHIFT(srcPixels[idx3], 16) & 0xFF) * (FP_MASK - fx) + (URSHIFT(srcPixels[idx4], 16) & 0xFF) * fx) >> FP_SHIFT) * fy) >> FP_SHIFT));
                g = (int)((((((URSHIFT(srcPixels[idx1], 8)  & 0xFF) * (FP_MASK - fx) + (URSHIFT(srcPixels[idx2], 8)  & 0xFF) * fx) >> FP_SHIFT) * (FP_MASK - fy) +
                            (((URSHIFT(srcPixels[idx3], 8)  & 0xFF) * (FP_MASK - fx) + (URSHIFT(srcPixels[idx4], 8)  & 0xFF) * fx) >> FP_SHIFT) * fy) >> FP_SHIFT));
                b = (int)(((((( srcPixels[idx1]        & 0xFF) * (FP_MASK - fx) + ( srcPixels[idx2]        & 0xFF) * fx) >> FP_SHIFT) * (FP_MASK - fy) +
                            ((( srcPixels[idx3]        & 0xFF) * (FP_MASK - fx) + ( srcPixels[idx4]        & 0xFF) * fx) >> FP_SHIFT) * fy) >> FP_SHIFT));

                destPixels[x + y * destw] = (a << 24) | (r << 16) | (g << 8) | b;
            }
        }
    } else {
        for (x = 0; x < destw; x++) {
            for (y = 0; y < desth; y++) {
                rx = (x << FP_SHIFT) / destw * srcw;
                ry = (y << FP_SHIFT) / desth * srch;

                ix = (int) URSHIFT(rx, FP_SHIFT);
                iy = (int) URSHIFT(ry, FP_SHIFT);

                fx = rx & FP_MASK;
                fy = ry & FP_MASK;

                idx1 = idx2 = idx3 = idx4 = ix + iy * srcw;

                if (ix < srcw - 1) { idx2++; idx4++; }
                if (iy < srch - 1) { idx3 += srcw; idx4 += srcw; }

                r = (int)((((((URSHIFT(srcPixels[idx1], 16) & 0xFF) * (FP_MASK - fx) + (URSHIFT(srcPixels[idx2], 16) & 0xFF) * fx) >> FP_SHIFT) * (FP_MASK - fy) +
                            (((URSHIFT(srcPixels[idx3], 16) & 0xFF) * (FP_MASK - fx) + (URSHIFT(srcPixels[idx4], 16) & 0xFF) * fx) >> FP_SHIFT) * fy) >> FP_SHIFT));
                g = (int)((((((URSHIFT(srcPixels[idx1], 8)  & 0xFF) * (FP_MASK - fx) + (URSHIFT(srcPixels[idx2], 8)  & 0xFF) * fx) >> FP_SHIFT) * (FP_MASK - fy) +
                            (((URSHIFT(srcPixels[idx3], 8)  & 0xFF) * (FP_MASK - fx) + (URSHIFT(srcPixels[idx4], 8)  & 0xFF) * fx) >> FP_SHIFT) * fy) >> FP_SHIFT));
                b = (int)(((((( srcPixels[idx1]        & 0xFF) * (FP_MASK - fx) + ( srcPixels[idx2]        & 0xFF) * fx) >> FP_SHIFT) * (FP_MASK - fy) +
                            ((( srcPixels[idx3]        & 0xFF) * (FP_MASK - fx) + ( srcPixels[idx4]        & 0xFF) * fx) >> FP_SHIFT) * fy) >> FP_SHIFT));

                destPixels[x + y * destw] = (r << 16) | (g << 8) | b;
            }
        }
    }
}

void ImageResize_bilinearResizeRGBnA(const int32_t *srcPixels, int32_t *destPixels,
                                     int srcw, int srch, int destw, int desth) {
    ImageResize_bilinearResizeRGB(srcPixels, destPixels, srcw, srch, destw, desth, 0);
}

void ImageResize_bilinearResizeRGBnA_toLow(const int32_t *srcPixels, int32_t *destPixels,
                                           int srcw, int srch, int destw, int desth) {
    int x, y, rx, ry, xx, yy;
    int idx1;
    int a, r, g, b;

    for (x = 0; x < destw; x++) {
        for (y = 0; y < desth; y++) {
            r = g = b = 0;
            a = 1;
            xx = x * srcw / destw;
            yy = y * srch / desth;
            int xxf = xx + srcw / destw;
            int yyf = yy + srch / desth;
            for (; xx < xxf; xx += 2) {
                for (; yy < yyf; yy += 2) {
                    rx = xx;
                    ry = yy;
                    if (rx > srcw) { rx = srcw; }
                    if (ry > srch) { rx = srch; } /* bug fiel do original (usa rx no lugar de ry) */

                    idx1 = rx + ry * srcw;
                    r += (int)(URSHIFT(srcPixels[idx1], 16) & 0xff);
                    g += (int)(URSHIFT(srcPixels[idx1], 8) & 0xff);
                    b += srcPixels[idx1] & 0xff;
                    a++;
                }
            }
            r = r / a; if (r > 255) r = 255;
            g = g / a; if (g > 255) g = 255;
            b = b / a; if (b > 255) b = 255;
            destPixels[x + y * destw] = (255 << 24) | (r << 16) | (g << 8) | b;
        }
    }
}

int32_t *ImageResize_mipMap(const int32_t *original, int w, int h) {
    int nw = w / 2, nh = h / 2;
    int len = w * h;
    int32_t *out = (int32_t*) malloc(sizeof(int32_t) * nw * nh);

    for (int y = 0; y < nh; y++) {
        for (int x = 0; x < nw; x++) {
            int col = original[(x * 2 + y * 2 * w)];
            int aa = (int)(URSHIFT(col, 24) & 0xff);
            int ar = (int)(URSHIFT(col, 16) & 0xff);
            int ag = (int)(URSHIFT(col, 8)  & 0xff);
            int ab = col & 0xff;

            col = original[(x * 2 + y * 2 * w + 1) & (len - 1)];
            int br = (int)(URSHIFT(col, 16) & 0xff);
            int bg = (int)(URSHIFT(col, 8)  & 0xff);
            int bb = col & 0xff;

            col = original[(x * 2 + y * 2 * w + 1 + w) & (len - 1)];
            int cr = (int)(URSHIFT(col, 16) & 0xff);
            int cg = (int)(URSHIFT(col, 8)  & 0xff);
            int cb = col & 0xff;

            col = original[(x * 2 + y * 2 * w + w) & (len - 1)];
            int dr = (int)(URSHIFT(col, 16) & 0xff);
            int dg = (int)(URSHIFT(col, 8)  & 0xff);
            int db = col & 0xff;

            int or_ = (ar + br + cr + dr) / 4;
            int og  = (ag + bg + cg + dg) / 4;
            int ob  = (ab + bb + cb + db) / 4;
            out[x + y * nw] = (aa << 24) | (or_ << 16) | (og << 8) | ob;
        }
    }
    return out;
}

int32_t *ImageResize_cubic2XDesize(const int32_t *original, int w, int h) {
    int nw = w / 2, nh = h / 2;
    int32_t *out = (int32_t*) malloc(sizeof(int32_t) * nw * nh);

    for (int y = 0; y < nh; y++) {
        for (int x = 0; x < nw; x++) {
            int col = original[(((x * 2) % w) + ((y * 2) % h) * w)];
            int aa = (int)(URSHIFT(col, 24) & 0xff);
            int ar = (int)(URSHIFT(col, 16) & 0xff);
            int ag = (int)(URSHIFT(col, 8)  & 0xff);
            int ab = col & 0xff;

            col = original[(((x * 2) % w) + ((y * 2 + 1) % h) * w)];
            int br = (int)(URSHIFT(col, 16) & 0xff);
            int bg = (int)(URSHIFT(col, 8)  & 0xff);
            int bb = col & 0xff;

            col = original[(((x * 2 + 1) % w) + ((y * 2 + 1) % h) * w)];
            int cr = (int)(URSHIFT(col, 16) & 0xff);
            int cg = (int)(URSHIFT(col, 8)  & 0xff);
            int cb = col & 0xff;

            col = original[(((x * 2 + 1) % w) + ((y * 2) % h) * w)];
            int dr = (int)(URSHIFT(col, 16) & 0xff);
            int dg = (int)(URSHIFT(col, 8)  & 0xff);
            int db = col & 0xff;

            int or_ = (ar + br + cr + dr) / 4;
            int og  = (ag + bg + cg + dg) / 4;
            int ob  = (ab + bb + cb + db) / 4;
            out[x + y * nw] = (aa << 24) | (or_ << 16) | (og << 8) | ob;
        }
    }
    return out;
}

int32_t *ImageResize_cubic2XVertDesize(const int32_t *original, int w, int h) {
    int nw = w, nh = h / 2;
    int32_t *out = (int32_t*) malloc(sizeof(int32_t) * nw * nh);

    for (int y = 0; y < nh; y++) {
        for (int x = 0; x < nw; x++) {
            int col = original[x + (((y * 2) % h) * w)];
            int aa = (int)(URSHIFT(col, 24) & 0xff);
            int ar = (int)(URSHIFT(col, 16) & 0xff);
            int ag = (int)(URSHIFT(col, 8)  & 0xff);
            int ab = col & 0xff;

            col = original[x + (((y * 2 + 1) % h) * w)];
            int br = (int)(URSHIFT(col, 16) & 0xff);
            int bg = (int)(URSHIFT(col, 8)  & 0xff);
            int bb = col & 0xff;

            int or_ = (ar + br) / 2;
            int og  = (ag + bg) / 2;
            int ob  = (ab + bb) / 2;
            out[x + y * nw] = (aa << 24) | (or_ << 16) | (og << 8) | ob;
        }
    }
    return out;
}

int32_t *ImageResize_cubic2XHorDesize(const int32_t *original, int w, int h) {
    int nw = w / 2, nh = h;
    int32_t *out = (int32_t*) malloc(sizeof(int32_t) * nw * nh);

    for (int y = 0; y < nh; y++) {
        for (int x = 0; x < nw; x++) {
            /* Bug fiel do original: usa "y%h * (w*2)" e "y%h * (w*2+1)".
             * O primeiro item pega col em endereco par, o segundo (w*2+1). */
            int col = original[x + ((y % h) * (w * 2))];
            int aa = (int)(URSHIFT(col, 24) & 0xff);
            int ar = (int)(URSHIFT(col, 16) & 0xff);
            int ag = (int)(URSHIFT(col, 8)  & 0xff);
            int ab = col & 0xff;

            col = original[x + ((y % h) * (w * 2 + 1))];
            int br = (int)(URSHIFT(col, 16) & 0xff);
            int bg = (int)(URSHIFT(col, 8)  & 0xff);
            int bb = col & 0xff;

            int or_ = (ar + br) / 2;
            int og  = (ag + bg) / 2;
            int ob  = (ab + bb) / 2;
            out[x + y * nw] = (aa << 24) | (or_ << 16) | (og << 8) | ob;
        }
    }
    return out;
}

/* --- Wrappers void* usados pelo Asset --- */

void *ImageResize_cubic2XHorDesize_v(void *img, int w, int h) {
    return ImageResize_cubic2XHorDesize((const int32_t*) img, w, h);
}
void *ImageResize_cubic2XVertDesize_v(void *img, int w, int h) {
    return ImageResize_cubic2XVertDesize((const int32_t*) img, w, h);
}
