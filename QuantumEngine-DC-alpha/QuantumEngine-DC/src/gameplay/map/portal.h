/*
 * portal.h — porte fiel de code/Gameplay/Map/Portal.java
 */
#ifndef QE_GAMEPLAY_MAP_PORTAL_H
#define QE_GAMEPLAY_MAP_PORTAL_H

#include <stdbool.h>
#include "../../rendering/vertex.h"

typedef struct DirectX7 DirectX7;
typedef struct Matrix   Matrix;

typedef struct Portal {
    Vertex vers[4];
    int    proj[8];     /* projected x,y for each vertex */
} Portal;

void Portal_init(Portal *self, int *verts);
void Portal_initFromVertices(Portal *self, Vertex *v0, Vertex *v1, Vertex *v2, Vertex *v3);

bool Portal_isFacing(Portal *self, Matrix *camMatrix);
void Portal_project(Portal *self, DirectX7 *g3d, Matrix *camMatrix);

int  Portal_getMinX(Portal *self);
int  Portal_getMaxX(Portal *self);
int  Portal_getMinY(Portal *self);
int  Portal_getMaxY(Portal *self);

/* Helper: Z-clip intersection */
void Portal_intersection(Vertex *out, Vertex *a, Vertex *b, int nearZ);

#endif
