/*
 * mesh_image.c — porte fiel de code/Rendering/Meshes/MeshImage.java
 */
#include "mesh_image.h"
#include "mesh.h"
#include "morphing.h"
#include "../vertex.h"
#include "../multytexture.h"
#include "../directx7.h"
#include "../renderobject.h"
#include "../../math/matrix.h"

#include <stdlib.h>

static int max2(int x, int y) { return x > y ? x : y; }

static void mi_render(RenderObject *ro, DirectX7 *g3d, Texture *texture) {
    (void) texture;
    MeshImage *self = (MeshImage*) ro;
    if (self->tex) MultyTexture_updateAnimation(self->tex);
    if (self->animation) {
        Morphing_setFrameNI(self->animation, self->frame);
        Mesh_setAnimation(self->mesh, self->animation);
    }
    DirectX7_transformAndProjectVertices(g3d, self->mesh, self->matrix);
    if (self->tex) Mesh_setTextureMT(self->mesh, self->tex);
    Mesh_renderRect(self->mesh, g3d, self->x1, self->y1, self->x2, self->y2);
}

static int mi_isVisible(RenderObject *ro, int x1, int y1, int x2, int y2) {
    MeshImage *self = (MeshImage*) ro;
    if (ro->sz - self->meshRadius > 0) return 0;
    if (-ro->sz / 4 - self->meshRadius > DirectX7_drDist) return 0;
    self->x1 = x1; self->y1 = y1; self->x2 = x2; self->y2 = y2;
    return 1;
}

static void mi_destroy(RenderObject *ro) { MeshImage_free((MeshImage*) ro); }

static const RenderObjectVTable g_mi_vt = { mi_render, mi_render, mi_isVisible, mi_destroy };
const RenderObjectVTable *MeshImage_vt(void) { return &g_mi_vt; }

MeshImage *MeshImage_new_m(Mesh *mesh) {
    MeshImage *m = (MeshImage*) calloc(1, sizeof(MeshImage));
    RenderObject_init(&m->base, &g_mi_vt);
    m->matrix = Matrix_new();
    m->mesh   = mesh;

    long sx = 0, sy = 0, sz = 0;
    int nv; Vertex **vs = Mesh_vertices(mesh, &nv);
    for (int i = 0; i < nv; i++) { sx += vs[i]->x; sy += vs[i]->y; sz += vs[i]->z; }
    if (nv > 0) { sx /= nv; sy /= nv; sz /= nv; }
    m->centre = Vertex_new_xyz((int)sx, (int)sy, (int)sz);
    m->meshRadius = max2(Mesh_maxX(mesh) - Mesh_minX(mesh), Mesh_maxZ(mesh) - Mesh_minZ(mesh));
    return m;
}

MeshImage *MeshImage_new(Mesh *mesh, Morphing *animation) {
    MeshImage *m = MeshImage_new_m(mesh);
    m->animation = animation;
    return m;
}

void MeshImage_free(MeshImage *self) {
    if (!self) return;
    Matrix_free(self->matrix);
    Vertex_free(self->centre);
    free(self);
}

void      MeshImage_setMesh    (MeshImage *self, Mesh *mesh)       { self->mesh = mesh; }
Mesh     *MeshImage_getMesh    (const MeshImage *self)              { return self->mesh; }
void      MeshImage_setAnimation(MeshImage *self, Morphing *a)      { self->animation = a; }
Morphing *MeshImage_getAnimation(const MeshImage *self)             { return self->animation; }
void      MeshImage_setTexture (MeshImage *self, MultyTexture *tex) { self->tex = tex; }
void      MeshImage_setAddMiddleZ(MeshImage *self, int addMiddleZ)  { self->addMiddleZ = addMiddleZ; }
void      MeshImage_setFrame   (MeshImage *self, int f)             { self->frame = f; }

void MeshImage_setMatrix(MeshImage *self, const Matrix *matrix) {
    Matrix_set_m(self->matrix, matrix);
    Vertex_transform(self->centre, matrix);
    self->base.sz = self->centre->rz * 4;
}
