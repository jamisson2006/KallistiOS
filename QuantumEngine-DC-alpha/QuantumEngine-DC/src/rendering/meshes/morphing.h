/*
 * morphing.h — porte fiel de code/Rendering/Meshes/Morphing.java
 */
#ifndef QE_RENDERING_MESHES_MORPHING_H
#define QE_RENDERING_MESHES_MORPHING_H

#include <stdint.h>

#define QE_MORPHING_FP  10
#define QE_MORPHING_FP_ONE (1 << QE_MORPHING_FP)

typedef struct Mesh   Mesh;
typedef struct Vertex Vertex;

typedef struct Morphing {
    int16_t **animation;    /* array de arrays: [frame][vertex*3] */
    int       animation_rows, animation_cols; /* frames, vertsflat */
    Mesh     *mesh;
    int       frame;
    int       morphEnabled;
} Morphing;

/* Cria a matriz de animacao a partir de um array de meshes. */
int16_t **Morphing_create(Mesh **meshes, int n_meshes, int start, int end, int *out_rows, int *out_cols);

Morphing *Morphing_new(int16_t **anim, int rows, int cols, Mesh *mesh);
void      Morphing_destroy(Morphing *self);

int16_t **Morphing_getVertices(Morphing *self, int *rows, int *cols);
int       Morphing_getMaxFrame(const Morphing *self);
int       Morphing_getFrame   (const Morphing *self);

void      Morphing_setFrameNI(Morphing *self, int frame);
void      Morphing_setFrame  (Morphing *self, int frame);

void      Morphing_interpolation(Morphing *self, Vertex **versRes, int n);

Mesh     *Morphing_getMesh(const Morphing *self);
int16_t **Morphing_getAnimation(Morphing *self);

#endif
