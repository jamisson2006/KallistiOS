/*
 * mesh_image.h — porte fiel de code/Rendering/Meshes/MeshImage.java
 */
#ifndef QE_RENDERING_MESHES_MESH_IMAGE_H
#define QE_RENDERING_MESHES_MESH_IMAGE_H

#include "../renderobject.h"

typedef struct Mesh         Mesh;
typedef struct Morphing     Morphing;
typedef struct MultyTexture MultyTexture;
typedef struct Matrix       Matrix;
typedef struct Vertex       Vertex;

typedef struct MeshImage {
    RenderObject  base;
    Matrix       *matrix;
    int           addMiddleZ;
    Vertex       *centre;
    Mesh         *mesh;
    Morphing     *animation;
    MultyTexture *tex;
    int           meshRadius;
    int           x1, y1, x2, y2;
    int           frame;
} MeshImage;

MeshImage *MeshImage_new(Mesh *mesh, Morphing *animation);
MeshImage *MeshImage_new_m(Mesh *mesh);
void       MeshImage_free(MeshImage *self);

void       MeshImage_setMesh    (MeshImage *self, Mesh *mesh);
Mesh      *MeshImage_getMesh    (const MeshImage *self);
void       MeshImage_setAnimation(MeshImage *self, Morphing *animation);
Morphing  *MeshImage_getAnimation(const MeshImage *self);
void       MeshImage_setTexture (MeshImage *self, MultyTexture *tex);
void       MeshImage_setAddMiddleZ(MeshImage *self, int addMiddleZ);
void       MeshImage_setMatrix  (MeshImage *self, const Matrix *matrix);
void       MeshImage_setFrame   (MeshImage *self, int f);

const RenderObjectVTable *MeshImage_vt(void);

#endif
