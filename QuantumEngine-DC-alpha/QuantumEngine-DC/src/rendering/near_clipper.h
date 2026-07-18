/*
 * near_clipper.h — porte fiel de code/Rendering/NearClipper.java
 * @author Roman Lahin
 */
#ifndef QE_RENDERING_NEAR_CLIPPER_H
#define QE_RENDERING_NEAR_CLIPPER_H

typedef struct Vertex   Vertex;
typedef struct DirectX7 DirectX7;

typedef struct RenderVertex {
    int x, y, z;
    int r, g, b;
    int u, v;
} RenderVertex;

/* Estado global (Java: static) */
extern RenderVertex *NearClipper_verts[9];
extern int           NearClipper_vertexCount;

void NearClipper_init(void); /* aloca os 9 verts estaticos uma vez */

void NearClipper_set(Vertex *a, int au, int av,
                     Vertex *b, int bu, int bv,
                     Vertex *c, int cu, int cv,
                     int ar, int br, int cr,
                     int ag, int bg, int cg,
                     int ab, int bb, int cb);

int  NearClipper_clip(int clipZ);
void NearClipper_project(DirectX7 *g3d);

void RenderVertex_set(RenderVertex *self, int x, int y, int z, int r, int g, int b, int u, int v);

#endif
