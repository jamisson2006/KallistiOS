/*
 * portal.c — porte fiel de code/Gameplay/Map/Portal.java
 */
#include "portal.h"
#include "../../math/matrix.h"
#include <stdlib.h>
#include <string.h>

/* Dependencias externas */
extern int  DirectX7_getScreenWidth(DirectX7 *g3d);
extern int  DirectX7_getScreenHeight(DirectX7 *g3d);
extern int  DirectX7_getNearZ(DirectX7 *g3d);

void Portal_init(Portal *self, int *verts) {
    memset(self, 0, sizeof(Portal));
    for (int i = 0; i < 4; i++) {
        Vertex_set(&self->vers[i], verts[i * 3], verts[i * 3 + 1], verts[i * 3 + 2]);
    }
}

void Portal_initFromVertices(Portal *self, Vertex *v0, Vertex *v1, Vertex *v2, Vertex *v3) {
    memset(self->proj, 0, sizeof(self->proj));
    Vertex_setV(&self->vers[0], v0);
    Vertex_setV(&self->vers[1], v1);
    Vertex_setV(&self->vers[2], v2);
    Vertex_setV(&self->vers[3], v3);
}

bool Portal_isFacing(Portal *self, Matrix *camMatrix) {
    /* Compute portal normal via cross product of edges */
    int e1x = self->vers[1].x - self->vers[0].x;
    int e1y = self->vers[1].y - self->vers[0].y;
    int e1z = self->vers[1].z - self->vers[0].z;

    int e2x = self->vers[2].x - self->vers[0].x;
    int e2y = self->vers[2].y - self->vers[0].y;
    int e2z = self->vers[2].z - self->vers[0].z;

    int nx = e1y * e2z - e1z * e2y;
    int ny = e1z * e2x - e1x * e2z;
    int nz = e1x * e2y - e1y * e2x;

    /* Vector from portal to camera */
    int cx = camMatrix->m03 - self->vers[0].x;
    int cy = camMatrix->m13 - self->vers[0].y;
    int cz = camMatrix->m23 - self->vers[0].z;

    /* Dot product: if positive, camera is on front side */
    long dot = (long)nx * cx + (long)ny * cy + (long)nz * cz;
    return dot > 0;
}

void Portal_intersection(Vertex *out, Vertex *a, Vertex *b, int nearZ) {
    int dz = b->z - a->z;
    if (dz == 0) {
        Vertex_setV(out, a);
        return;
    }
    int t = ((nearZ - a->z) << 14) / dz;
    out->x = a->x + ((b->x - a->x) * t >> 14);
    out->y = a->y + ((b->y - a->y) * t >> 14);
    out->z = nearZ;
}

void Portal_project(Portal *self, DirectX7 *g3d, Matrix *camMatrix) {
    int nearZ = DirectX7_getNearZ(g3d);
    int screenW = DirectX7_getScreenWidth(g3d);
    int screenH = DirectX7_getScreenHeight(g3d);
    int halfW = screenW >> 1;
    int halfH = screenH >> 1;

    /* Transform vertices to camera space and project */
    Vertex transformed[4];
    for (int i = 0; i < 4; i++) {
        transformed[i] = self->vers[i];
        Vertex_transform(&transformed[i], camMatrix);
    }

    /* Project each vertex, handling Z clipping */
    for (int i = 0; i < 4; i++) {
        Vertex *cur = &transformed[i];
        Vertex *next = &transformed[(i + 1) % 4];

        if (cur->z < nearZ) {
            /* Clip: find intersection with near plane using adjacent vertex */
            Vertex clipped;
            if (next->z >= nearZ) {
                Portal_intersection(&clipped, cur, next, nearZ);
                cur->x = clipped.x;
                cur->y = clipped.y;
                cur->z = clipped.z;
            } else {
                /* Both behind - push to screen edges */
                self->proj[i * 2] = (cur->x < 0) ? 0 : screenW;
                self->proj[i * 2 + 1] = (cur->y < 0) ? 0 : screenH;
                continue;
            }
        }

        /* Perspective projection */
        if (cur->z > 0) {
            self->proj[i * 2]     = halfW + (cur->x * halfW) / cur->z;
            self->proj[i * 2 + 1] = halfH - (cur->y * halfH) / cur->z;
        } else {
            self->proj[i * 2]     = halfW;
            self->proj[i * 2 + 1] = halfH;
        }
    }
}

int Portal_getMinX(Portal *self) {
    int min = self->proj[0];
    for (int i = 1; i < 4; i++) {
        if (self->proj[i * 2] < min) min = self->proj[i * 2];
    }
    return min;
}

int Portal_getMaxX(Portal *self) {
    int max = self->proj[0];
    for (int i = 1; i < 4; i++) {
        if (self->proj[i * 2] > max) max = self->proj[i * 2];
    }
    return max;
}

int Portal_getMinY(Portal *self) {
    int min = self->proj[1];
    for (int i = 1; i < 4; i++) {
        if (self->proj[i * 2 + 1] < min) min = self->proj[i * 2 + 1];
    }
    return min;
}

int Portal_getMaxY(Portal *self) {
    int max = self->proj[1];
    for (int i = 1; i < 4; i++) {
        if (self->proj[i * 2 + 1] > max) max = self->proj[i * 2 + 1];
    }
    return max;
}
