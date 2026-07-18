/*
 * texturing_experiments.h — porte fiel de code/Rendering/TexturingExperiments.java
 * @author Roman Lahin
 *
 * NOTA: no original Java essa classe eh um work-in-progress (loop vazio),
 * portanto o port preserva o mesmo estado — nao introduzimos logica que
 * nao existe. */
#ifndef QE_RENDERING_TEXTURING_EXPERIMENTS_H
#define QE_RENDERING_TEXTURING_EXPERIMENTS_H

typedef struct DirectX7 DirectX7;
typedef struct Texture  Texture;
typedef struct Vertex   Vertex;

void TexturingExperiments_renderQuad(DirectX7 *g3d, Texture *tex,
    Vertex *a, int au, int av,
    Vertex *b, int bu, int bv,
    Vertex *c, int cu, int cv,
    Vertex *d, int du, int dv);

void TexturingExperiments_renderPolygon(DirectX7 *g3d, Texture *tex,
    Vertex **verts, int nverts, int *uTex, int *vTex);

#endif
