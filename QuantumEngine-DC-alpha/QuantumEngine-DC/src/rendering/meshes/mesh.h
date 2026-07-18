/*
 * mesh.h — porte fiel de code/Rendering/Meshes/Mesh.java
 */
#ifndef QE_RENDERING_MESHES_MESH_H
#define QE_RENDERING_MESHES_MESH_H

typedef struct Vertex        Vertex;
typedef struct RenderObject  RenderObject;
typedef struct MultyTexture  MultyTexture;
typedef struct Texture       Texture;
typedef struct DirectX7      DirectX7;
typedef struct Matrix        Matrix;
typedef struct Morphing      Morphing;
typedef struct Room          Room;

typedef struct Mesh {
    MultyTexture   *texture;
    Vertex        **vertices;   int vertices_n;
    RenderObject **polygons;    int polygons_n;
} Mesh;

Mesh *Mesh_new(void);
Mesh *Mesh_new_vp(Vertex **vertices, int nv, RenderObject **polygons, int np);
Mesh *Mesh_new_vpt(Vertex **vertices, int nv, RenderObject **polygons, int np, MultyTexture *tex);
void  Mesh_destroy(Mesh *self);

void  Mesh_optimize(Mesh *self);

void  Mesh_setTexture   (Mesh *self, Texture *texture);
void  Mesh_setTextureMT (Mesh *self, MultyTexture *texture);
void  Mesh_resetTexture (Mesh *self);

int   Mesh_maxX(const Mesh *self);
int   Mesh_maxY(const Mesh *self);
int   Mesh_maxZ(const Mesh *self);
int   Mesh_minX(const Mesh *self);
int   Mesh_minY(const Mesh *self);
int   Mesh_minZ(const Mesh *self);

MultyTexture *Mesh_getTexture(Mesh *self);
Vertex      **Mesh_vertices(Mesh *self, int *out_n);
RenderObject**Mesh_polygons(Mesh *self, int *out_n);
Texture     **Mesh_textures(Mesh *self, int *out_n);

void  Mesh_render      (Mesh *self, DirectX7 *g3d);
void  Mesh_renderRect  (Mesh *self, DirectX7 *g3d, int x1, int y1, int x2, int y2);
void  Mesh_resetBuffer (void);

void  Mesh_applySz(Mesh *self);
void  Mesh_increaseMeshSz(Mesh *self, int z);

void  Mesh_setAnimation(Mesh *self, Morphing *animation);

void  Mesh_recalculateNormals(Mesh *self, const Matrix *transform);
void  Mesh_rotateNormals     (Mesh *self, const Matrix *transform);
void  Mesh_updateLighting    (Mesh *self, const Matrix *transform, int smoothNormals, Room *room);

#endif
