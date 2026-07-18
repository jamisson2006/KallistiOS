/*
 * texturing_pers.h — porte de code/Rendering/TexturingPers.java
 *
 * Rasterizador com correcao de perspectiva (subdivide o triangulo em
 * spans de q pixels e recalcula u/v/q por span). O Java expoe apenas
 * paint(); os subprograms sao internos.
 *
 * Como TexturingAffine, esta fase porta o dispatcher fielmente e usa
 * TexturingAffine_paintAffine_0/9 como fallback para os modos ainda por
 * portar em detalhe (a saber: fog aditivo, glass, mul, dither). Modos
 * portados: 0 (base) e 9 (lit) — via delega a TexturingAffine.
 */
#ifndef QE_RENDERING_TEXTURING_PERS_H
#define QE_RENDERING_TEXTURING_PERS_H

typedef struct Vertex   Vertex;
typedef struct Texture  Texture;
typedef struct DirectX7 DirectX7;

#define QE_TEXPERS_FP     12
#define QE_TEXPERS_FP_ONE (1 << QE_TEXPERS_FP)

extern int TexturingPers_move;

void TexturingPers_paint(DirectX7 *g3d, Texture *texture,
    Vertex *a, int au, int av,
    Vertex *b, int bu, int bv,
    Vertex *c, int cu, int cv,
    int fogc, int dmode, int qz, int q,
    int al, int bl, int cl,
    int ag, int bg, int cg,
    int ab, int bb, int cb,
    int nx, int ny, int nz);

#endif
