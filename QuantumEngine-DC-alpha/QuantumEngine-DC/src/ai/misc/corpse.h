/*
 * corpse.h — porte fiel de code/AI/misc/Corpse.java
 */
#ifndef QE_AI_MISC_CORPSE_H
#define QE_AI_MISC_CORPSE_H

#include "../../gameplay/objects/game_object.h"
#include "../../math/matrix.h"

typedef struct MeshImage    MeshImage;
typedef struct MultyTexture MultyTexture;
typedef struct BoundingBox  BoundingBox;
typedef struct Sprite       Sprite;
typedef struct DirectX7     DirectX7;

typedef struct Corpse {
    GameObject    base;
    MultyTexture *mtex;
    int           f;
    int           frame;
    Matrix        mat;
    MeshImage    *meshImage;
    BoundingBox  *boundingBox;
    Sprite       *spr;
} Corpse;

Corpse *Corpse_new(int frame, Matrix *matrix, MeshImage *mesh, MultyTexture *tex);
void    Corpse_render(Corpse *self, DirectX7 *g3d, int x1, int y1, int x2, int y2);

#endif
