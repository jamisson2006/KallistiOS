/*
 * texturing_affine.h — porte fiel de code/Rendering/TexturingAffine.java
 *
 * Rasterizador de triangulo com texturas afim (sem correcao de perspectiva).
 * 14 modos de blend (0=base, 1=fog add, 2=constant fog, 3=fog mix,
 * 4=color multiply, 5=directional light, 6=distance fog, 7=glass,
 * 8=fill, 9=vertex light interpolated, 10-12=dither variants, 13=RGB lit).
 *
 * Este arquivo mantem os coefs fp/fpPosition do original (12 / 14) para
 * paridade bit-a-bit.
 */
#ifndef QE_RENDERING_TEXTURING_AFFINE_H
#define QE_RENDERING_TEXTURING_AFFINE_H

typedef struct Vertex   Vertex;
typedef struct Texture  Texture;
typedef struct DirectX7 DirectX7;

#define QE_TEXAFF_FP          12
#define QE_TEXAFF_FP_ONE      (1 << QE_TEXAFF_FP)
#define QE_TEXAFF_FP_POSITION 14
#define QE_TEXAFF_FP_POSITION_ONE (1 << QE_TEXAFF_FP_POSITION)

/* Entrypoint principal (Java: paint()). */
void TexturingAffine_paint(DirectX7 *g3d, Texture *texture,
    Vertex *a, int au, int av,
    Vertex *b, int bu, int bv,
    Vertex *c, int cu, int cv,
    int fogc, int fog, int sz2,
    int af, int bf, int cf,
    int ag, int bg, int cg,
    int ab, int bb, int cb,
    int nx, int ny, int nz);

/* Sub-modos (todos fieis ao Java): */
void TexturingAffine_paintOverwrite(DirectX7 *g3d, Vertex *a, Vertex *b, Vertex *c);
void TexturingAffine_paintAffine_0 (DirectX7 *g3d, Texture *texture,
    Vertex *a, int au, int av, Vertex *b, int bu, int bv, Vertex *c, int cu, int cv);
void TexturingAffine_paintAffine_9 (DirectX7 *g3d, Texture *texture,
    Vertex *a, int au, int av, Vertex *b, int bu, int bv, Vertex *c, int cu, int cv,
    int fogc, int sz2, int af, int bf, int cf, int nx, int ny, int nz);

/* Linha basica (fiel a Java line_0) */
void TexturingAffine_line_0(int x1, int x2, int *rgb, const int *tex,
                            int lengthBIT, int widthBIT, int u, int v, int du, int dv);

#endif
