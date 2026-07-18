/*
 * bounding_box.h — porte fiel de code/Rendering/Meshes/BoundingBox.java
 * @author DDDENISSS
 */
#ifndef QE_RENDERING_MESHES_BOUNDING_BOX_H
#define QE_RENDERING_MESHES_BOUNDING_BOX_H

typedef struct Mesh     Mesh;
typedef struct Morphing Morphing;
typedef struct Matrix   Matrix;
typedef struct DirectX7 DirectX7;

typedef struct BoundingBox {
    int minx, miny, maxx, maxy, minz, maxz;
} BoundingBox;

BoundingBox *BoundingBox_new_mesh(Mesh *mesh);
BoundingBox *BoundingBox_new_morphing(Morphing *anim);
void         BoundingBox_free(BoundingBox *self);

void         BoundingBox_set(BoundingBox *self, int minX, int minY, int minZ,
                                                 int maxX, int maxY, int maxZ);

int          BoundingBox_isVisible (BoundingBox *self, DirectX7 *g3d, int x1, int y1, int x2, int y2);
int          BoundingBox_isVisibleM(BoundingBox *self, DirectX7 *g3d, const Matrix *matrix, int x1, int y1, int x2, int y2);

void         BoundingBox_reSort(BoundingBox *self, const Matrix *matrix);
int  BoundingBox_getMinX(void);
int  BoundingBox_getMinY(void);
int  BoundingBox_getMinZ(void);
int  BoundingBox_getMaxX(void);
int  BoundingBox_getMaxY(void);
int  BoundingBox_getMaxZ(void);

#endif
