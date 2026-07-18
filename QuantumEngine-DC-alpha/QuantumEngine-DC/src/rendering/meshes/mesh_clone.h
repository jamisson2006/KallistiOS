/*
 * mesh_clone.h — porte fiel de code/Rendering/Meshes/MeshClone.java
 */
#ifndef QE_RENDERING_MESHES_MESH_CLONE_H
#define QE_RENDERING_MESHES_MESH_CLONE_H

typedef struct Mesh         Mesh;
typedef struct MultyTexture MultyTexture;

typedef struct MeshClone {
    int          *vertexCoords; int vertexCoords_n;
    int          *poly3vData;   int poly3vData_n;
    int          *poly4vData;   int poly4vData_n;
    MultyTexture *texture;
    int           lighting;
} MeshClone;

MeshClone *MeshClone_new(void);
MeshClone *MeshClone_new_fromMesh(Mesh *mesh);
void       MeshClone_destroy(MeshClone *self);

Mesh      *MeshClone_copy(const MeshClone *self);

#endif
