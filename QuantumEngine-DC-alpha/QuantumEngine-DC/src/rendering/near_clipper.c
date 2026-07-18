/*
 * near_clipper.c — porte fiel de code/Rendering/NearClipper.java
 */
#include "near_clipper.h"
#include "vertex.h"
#include "directx7.h"

#include <stdlib.h>

RenderVertex *NearClipper_verts[9];
int           NearClipper_vertexCount = 3;
static int    g_init = 0;

void NearClipper_init(void) {
    if (g_init) return;
    for (int i = 0; i < 9; i++)
        NearClipper_verts[i] = (RenderVertex*) calloc(1, sizeof(RenderVertex));
    g_init = 1;
}

void RenderVertex_set(RenderVertex *self, int x, int y, int z, int r, int g, int b, int u, int v) {
    self->x = x; self->y = y; self->z = z;
    self->r = r; self->g = g; self->b = b;
    self->u = u; self->v = v;
}

static void insertVertex(int index, int x, int y, int z, int r, int g, int b, int u, int v) {
    RenderVertex *newVertex = NearClipper_verts[8];
    RenderVertex_set(newVertex, x, y, z, r, g, b, u, v);
    for (int i = 8; i > index; i--) NearClipper_verts[i] = NearClipper_verts[i - 1];
    NearClipper_verts[index] = newVertex;
    NearClipper_vertexCount++;
}

static void deleteVertex(int index) {
    RenderVertex *deleted = NearClipper_verts[index];
    for (int i = index; i < 8; i++) NearClipper_verts[i] = NearClipper_verts[i + 1];
    NearClipper_verts[8] = deleted;
    NearClipper_vertexCount--;
}

void NearClipper_set(Vertex *a, int au, int av,
                     Vertex *b, int bu, int bv,
                     Vertex *c, int cu, int cv,
                     int ar, int br, int cr,
                     int ag, int bg, int cg,
                     int ab, int bb, int cb) {
    NearClipper_init();
    RenderVertex_set(NearClipper_verts[0], a->sx, a->sy, a->rz, ar, ag, ab, au, av);
    RenderVertex_set(NearClipper_verts[1], b->sx, b->sy, b->rz, br, bg, bb, bu, bv);
    RenderVertex_set(NearClipper_verts[2], c->sx, c->sy, c->rz, cr, cg, cb, cu, cv);
    NearClipper_vertexCount = 3;
}

int NearClipper_clip(int clipZ) {
    int isCompletelyHidden = 1;

    for (int i = 0; i < NearClipper_vertexCount; i++) {
        int next = (i + 1) % NearClipper_vertexCount;
        RenderVertex *v1 = NearClipper_verts[i];
        RenderVertex *v2 = NearClipper_verts[next];
        if (v1->z < clipZ) isCompletelyHidden = 0;

        if (v1->z > v2->z) { RenderVertex *tmp = v1; v1 = v2; v2 = tmp; }

        if (v1->z < clipZ && v2->z > clipZ) {
            int mul = clipZ - v1->z;
            int div = v2->z - v1->z;
            insertVertex(next,
                v1->x + (mul * (v2->x - v1->x) / div), v1->y + (mul * (v2->y - v1->y) / div), clipZ,
                v1->r + (mul * (v2->r - v1->r) / div), v1->g + (mul * (v2->g - v1->g) / div), v1->b + (mul * (v2->b - v1->b) / div),
                v1->u + (mul * (v2->u - v1->u) / div), v1->v + (mul * (v2->v - v1->v) / div));
            i++;
        }
    }

    if (isCompletelyHidden) return 0;

    for (int i = NearClipper_vertexCount - 1; i >= 0; i--) {
        if (NearClipper_verts[i]->z > clipZ) deleteVertex(i);
    }
    return NearClipper_vertexCount >= 3;
}

void NearClipper_project(DirectX7 *g3d) {
    for (int i = 0; i < NearClipper_vertexCount; i++) {
        RenderVertex *ver = NearClipper_verts[i];
        int sx = ver->x, sy = -ver->y, rz = ver->z;
        if (rz <= 0) {
            sx = sx * g3d->distX / (rz + g3d->distX);
            sy = sy * g3d->distY / (rz + g3d->distY);
        }
        ver->x = sx + g3d->centreX;
        ver->y = sy + g3d->centreY;
    }
}
