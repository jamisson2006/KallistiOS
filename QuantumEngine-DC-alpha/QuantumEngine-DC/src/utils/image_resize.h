/*
 * image_resize.h — porte fiel de code/utils/ImageResize.java (Quantum Engine J2ME)
 * @author DDDEN!SSS
 *
 * Todas as operacoes trabalham sobre buffers ARGB int32_t (mesma
 * representacao do Image.getRGB Java). Retornos "novos" sao heap
 * (chamador free()).
 *
 * No Java, "Image" era javax.microedition.lcdui.Image. Aqui trabalhamos
 * apenas com buffers de pixels — o modulo de Image (RawImage etc.) fica
 * separado e chama estas funcoes.
 */
#ifndef QE_UTILS_IMAGE_RESIZE_H
#define QE_UTILS_IMAGE_RESIZE_H

#include <stdint.h>

/* Redimensiona ARGB. destPixels deve ter destw*desth ints.
 * alpha=0: apenas RGB. */
void ImageResize_bilinearResizeRGB(const int32_t *srcPixels, int32_t *destPixels,
                                    int srcw, int srch, int destw, int desth, int alpha);

/* Variantes fieis: nA = no alpha; toLow = versao "raw" simples do original. */
void ImageResize_bilinearResizeRGBnA(const int32_t *srcPixels, int32_t *destPixels,
                                      int srcw, int srch, int destw, int desth);
void ImageResize_bilinearResizeRGBnA_toLow(const int32_t *srcPixels, int32_t *destPixels,
                                            int srcw, int srch, int destw, int desth);

/* mipMap: reduz por 2 em ambos os eixos com media 2x2 (heap alocado). */
int32_t *ImageResize_mipMap(const int32_t *original, int w, int h);
int32_t *ImageResize_cubic2XDesize(const int32_t *original, int w, int h);
int32_t *ImageResize_cubic2XVertDesize(const int32_t *original, int w, int h);
int32_t *ImageResize_cubic2XHorDesize(const int32_t *original, int w, int h);

/* Wrappers usados por Asset.desizeSomething via void*: */
void *ImageResize_cubic2XHorDesize_v(void *img, int w, int h);
void *ImageResize_cubic2XVertDesize_v(void *img, int w, int h);

#endif
