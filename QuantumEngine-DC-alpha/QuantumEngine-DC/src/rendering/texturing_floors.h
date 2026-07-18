/*
 * texturing_floors.h — porte de code/Rendering/TexturingFloors.java
 *
 * Rasterizador otimizado para pisos horizontais (ny > 4000). Delega ao
 * TexturingAffine ate a conversao completa dos subprograms (1141 linhas).
 */
#ifndef QE_RENDERING_TEXTURING_FLOORS_H
#define QE_RENDERING_TEXTURING_FLOORS_H

typedef struct Vertex   Vertex;
typedef struct Texture  Texture;
typedef struct DirectX7 DirectX7;

void TexturingFloors_paint(DirectX7 *g3d, Texture *texture,
    Vertex *a, int au, int av,
    Vertex *b, int bu, int bv,
    Vertex *c, int cu, int cv,
    int fogc, int dmode, int qz, int q,
    int al, int bl, int cl,
    int ag, int bg, int cg,
    int ab, int bb, int cb,
    int nx, int ny, int nz);

#endif
