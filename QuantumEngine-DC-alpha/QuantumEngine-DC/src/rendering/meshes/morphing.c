/*
 * morphing.c — porte fiel de code/Rendering/Meshes/Morphing.java
 */
#include "morphing.h"
#include "mesh.h"
#include "../vertex.h"

#include <stdlib.h>
#include <string.h>

int16_t **Morphing_create(Mesh **meshes, int n_meshes, int start, int end, int *out_rows, int *out_cols) {
    (void) n_meshes;
    int rows = end - start;
    int16_t **anim = (int16_t**) calloc(rows > 0 ? rows : 1, sizeof(int16_t*));

    int cols = 0;
    int c = 0;
    for (int i = start; i < end; i++) {
        Mesh *mesh = meshes[i];
        int nv;
        Vertex **verts = Mesh_vertices(mesh, &nv);
        cols = nv * 3;
        int16_t *m = (int16_t*) malloc(sizeof(int16_t) * cols);
        for (int x = 0; x < nv; x++) {
            m[x*3    ] = (int16_t) verts[x]->x;
            m[x*3 + 1] = (int16_t) verts[x]->y;
            m[x*3 + 2] = (int16_t) verts[x]->z;
        }
        anim[c++] = m;
    }
    *out_rows = rows;
    *out_cols = cols;
    return anim;
}

Morphing *Morphing_new(int16_t **anim, int rows, int cols, Mesh *mesh) {
    Morphing *m = (Morphing*) calloc(1, sizeof(Morphing));
    m->animation      = anim;
    m->animation_rows = rows;
    m->animation_cols = cols;
    m->mesh           = mesh;
    m->morphEnabled   = 1;
    return m;
}

void Morphing_destroy(Morphing *self) {
    if (!self) return;
    if (self->animation) {
        for (int i = 0; i < self->animation_rows; i++) free(self->animation[i]);
        free(self->animation);
    }
    free(self);
}

int16_t **Morphing_getVertices(Morphing *self, int *rows, int *cols) {
    *rows = self->animation_rows; *cols = self->animation_cols;
    return self->animation;
}
int Morphing_getMaxFrame(const Morphing *self) { return QE_MORPHING_FP_ONE * self->animation_rows; }
int Morphing_getFrame   (const Morphing *self) { return self->frame; }

void Morphing_setFrameNI(Morphing *self, int frame) {
    int maxFrame = Morphing_getMaxFrame(self);
    while (frame < 0) frame += maxFrame;
    if (maxFrame != 0) frame %= maxFrame;
    else frame = 0;
    self->frame = frame;
}
void Morphing_setFrame(Morphing *self, int frame) {
    Morphing_setFrameNI(self, frame);
    int nv; Vertex **verts = Mesh_vertices(self->mesh, &nv);
    Morphing_interpolation(self, verts, nv);
}

void Morphing_interpolation(Morphing *self, Vertex **versRes, int n) {
    int aFrame = self->frame / QE_MORPHING_FP_ONE;
    int af1 = 0, af2 = 0;
    if (self->animation_rows != 0) {
        af1 = aFrame % self->animation_rows;
        af2 = (aFrame + 1) % self->animation_rows;
    }
    int16_t *versA = self->animation[af1];
    int16_t *versB = self->animation[af2];

    int kinv = self->frame % QE_MORPHING_FP_ONE;
    int k = QE_MORPHING_FP_ONE - kinv;
    int morphEnabled = self->morphEnabled;

    for (int i = 0; i < n; i++) {
        int nn = i * 3;
        int ax = versA[nn], ay = versA[nn+1], az = versA[nn+2];
        if (morphEnabled) {
            int bx = versB[nn], by = versB[nn+1], bz = versB[nn+2];
            Vertex_set(versRes[i],
                (ax * k >> QE_MORPHING_FP) + (bx * kinv >> QE_MORPHING_FP),
                (ay * k >> QE_MORPHING_FP) + (by * kinv >> QE_MORPHING_FP),
                (az * k >> QE_MORPHING_FP) + (bz * kinv >> QE_MORPHING_FP));
        } else {
            Vertex_set(versRes[i], ax, ay, az);
        }
    }
}

Mesh     *Morphing_getMesh(const Morphing *self) { return self->mesh; }
int16_t **Morphing_getAnimation(Morphing *self)  { return self->animation; }
