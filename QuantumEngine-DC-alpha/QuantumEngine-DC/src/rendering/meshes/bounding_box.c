/*
 * bounding_box.c — porte fiel de code/Rendering/Meshes/BoundingBox.java
 */
#include "bounding_box.h"
#include "mesh.h"
#include "morphing.h"
#include "../vertex.h"
#include "../directx7.h"
#include "../../math/matrix.h"

#include <limits.h>
#include <stdlib.h>

static Vertex g_vertices[8];
static Vertex *g_ptrs[8];
static int    g_init = 0;
static int    minpx, minpy, maxpx, maxpy, minpz, maxpz;

static void ensure_init(void) {
    if (g_init) return;
    for (int i = 0; i < 8; i++) g_ptrs[i] = &g_vertices[i];
    g_init = 1;
}

BoundingBox *BoundingBox_new_mesh(Mesh *mesh) {
    BoundingBox *b = (BoundingBox*) calloc(1, sizeof(BoundingBox));
    b->minx = Mesh_minX(mesh); b->miny = Mesh_minY(mesh); b->minz = Mesh_minZ(mesh);
    b->maxx = Mesh_maxX(mesh); b->maxy = Mesh_maxY(mesh); b->maxz = Mesh_maxZ(mesh);
    return b;
}

BoundingBox *BoundingBox_new_morphing(Morphing *anim) {
    BoundingBox *b = (BoundingBox*) calloc(1, sizeof(BoundingBox));
    b->minx = b->miny = b->minz = INT_MAX;
    b->maxx = b->maxy = b->maxz = INT_MIN;

    extern int16_t **Morphing_getVertices(Morphing *m, int *rows, int *cols);
    int nrows, ncols;
    int16_t **vertices = Morphing_getVertices(anim, &nrows, &ncols);
    for (int j = 0; j < nrows; j++) {
        int16_t *vers = vertices[j];
        int nv = ncols / 3;
        for (int i = 0; i < nv; i++) {
            int x = vers[i*3], y = vers[i*3+1], z = vers[i*3+2];
            if (x < b->minx) b->minx = x;
            if (y < b->miny) b->miny = y;
            if (z < b->minz) b->minz = z;
            if (x > b->maxx) b->maxx = x;
            if (y > b->maxy) b->maxy = y;
            if (z > b->maxz) b->maxz = z;
        }
    }
    return b;
}

void BoundingBox_free(BoundingBox *self) { free(self); }

void BoundingBox_set(BoundingBox *self, int minX, int minY, int minZ, int maxX, int maxY, int maxZ) {
    (void) self;
    ensure_init();
    Vertex_set(g_ptrs[0], minX, minY, minZ);
    Vertex_set(g_ptrs[1], maxX, minY, minZ);
    Vertex_set(g_ptrs[2], minX, minY, maxZ);
    Vertex_set(g_ptrs[3], maxX, minY, maxZ);
    Vertex_set(g_ptrs[4], minX, maxY, minZ);
    Vertex_set(g_ptrs[5], maxX, maxY, minZ);
    Vertex_set(g_ptrs[6], minX, maxY, maxZ);
    Vertex_set(g_ptrs[7], maxX, maxY, maxZ);
}

int BoundingBox_isVisible(BoundingBox *self, DirectX7 *g3d, int x1, int y1, int x2, int y2) {
    return BoundingBox_isVisibleM(self, g3d, g3d->invCamera, x1, y1, x2, y2);
}

int BoundingBox_isVisibleM(BoundingBox *self, DirectX7 *g3d, const Matrix *matrix, int x1, int y1, int x2, int y2) {
    BoundingBox_set(self, self->minx, self->miny, self->minz, self->maxx, self->maxy, self->maxz);
    DirectX7_transformAndProjectVerticesArr(g3d, g_ptrs, 8, matrix);

    Vertex *v = g_ptrs[0];
    int mnx=v->sx, mxx=v->sx, mny=v->sy, mxy=v->sy, mnz=v->rz, mxz=v->rz;
    for (int i = 1; i < 8; i++) {
        v = g_ptrs[i];
        if (v->sx < mnx) mnx = v->sx;
        if (v->sy < mny) mny = v->sy;
        if (v->rz < mnz) mnz = v->rz;
        if (v->rz > mxz) mxz = v->rz;
        if (v->sx > mxx) mxx = v->sx;
        if (v->sy > mxy) mxy = v->sy;
    }
    return !(mxx < x1 || mnx > x2 || mxy < y1 || mny > y2 || mnz >= 0 || -mxz > DirectX7_drDist);
}

void BoundingBox_reSort(BoundingBox *self, const Matrix *matrix) {
    BoundingBox_set(self, self->minx, self->miny, self->minz, self->maxx, self->maxy, self->maxz);
    DirectX7_transformSave(g_ptrs, 8, matrix);

    Vertex *v = g_ptrs[0];
    minpx = maxpx = v->x;
    minpy = maxpy = v->y;
    minpz = maxpz = v->z;
    for (int i = 1; i < 8; i++) {
        v = g_ptrs[i];
        if (v->x < minpx) minpx = v->x;
        if (v->y < minpy) minpy = v->y;
        if (v->z < minpz) minpz = v->z;
        if (v->x > maxpx) maxpx = v->x;
        if (v->y > maxpy) maxpy = v->y;
        if (v->z > maxpz) maxpz = v->z;
    }
}

int BoundingBox_getMinX(void) { return minpx; }
int BoundingBox_getMinY(void) { return minpy; }
int BoundingBox_getMinZ(void) { return minpz; }
int BoundingBox_getMaxX(void) { return maxpx; }
int BoundingBox_getMaxY(void) { return maxpy; }
int BoundingBox_getMaxZ(void) { return maxpz; }
