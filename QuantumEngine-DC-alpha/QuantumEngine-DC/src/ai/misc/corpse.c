/*
 * corpse.c — porte fiel de code/AI/misc/Corpse.java
 */
#include "corpse.h"
#include <stdlib.h>

typedef struct Sprite       Sprite;
typedef struct MeshImage    MeshImage;
typedef struct MultyTexture MultyTexture;
typedef struct BoundingBox  BoundingBox;
typedef struct DirectX7     DirectX7;
typedef struct Morphing     Morphing;
typedef struct Mesh         Mesh;
typedef struct RenderObject RenderObject;
typedef struct Vector3D     Vector3D;

extern MeshImage   *MeshImage_new(Mesh *mesh, Morphing *anim);
extern Mesh        *MeshImage_getMesh(MeshImage *mi);
extern Morphing    *MeshImage_getAnimation(MeshImage *mi);
extern void         MeshImage_setMatrix(MeshImage *mi, Matrix *mat);
extern void         MeshImage_setTexture(MeshImage *mi, MultyTexture *mt);
extern void         MeshImage_setFrame(MeshImage *mi, int frame); /* compat: Morphing_setFrame */
extern int         *MeshImage_getSzPtr(MeshImage *mi);

extern BoundingBox *BoundingBox_new(Morphing *anim);
extern bool         BoundingBox_isVisible(BoundingBox *bb, DirectX7 *g3d, Matrix *fmat, int x1, int y1, int x2, int y2);

extern Matrix      *DirectX7_computeFinalMatrix(DirectX7 *g3d, Matrix *mat);
extern void         DirectX7_addRenderObject(DirectX7 *g3d, void *obj, int x1, int y1, int x2, int y2);
extern Matrix      *DirectX7_getInvCamera(DirectX7 *g3d);

extern Sprite      *Sprite_getPositionSprite(Sprite *spr); /* returns pos ref */
extern Vector3D    *Sprite_getPosition(Sprite *spr);
extern void         Sprite_updateFrame(Sprite *spr);
extern void         Sprite_project(Sprite *spr, Matrix *invCam, DirectX7 *g3d);
extern int          Sprite_isVisible(Sprite *spr, int x1, int y1, int x2, int y2);
extern int         *Sprite_getSzPtr(Sprite *spr);
extern void         Vector3D_set(Vector3D *self, int x, int y, int z);

extern void         Morphing_setFrame(Morphing *m, int frame);
extern void         Character_reset(Character *ch);
extern void         Character_setCollision(Character *ch, bool c);
extern void         Character_setCollidable(Character *ch, bool c);
extern void         Character_setOnFloor(Character *ch, bool f);
extern void         Character_setSpeedZero(Character *ch);

Corpse *Corpse_new(int frame2, Matrix *matrix2, MeshImage *mesh, MultyTexture *tex) {
    Corpse *self = (Corpse *)calloc(1, sizeof(Corpse));

    Matrix_setIdentity(&self->mat);
    Matrix_set(&self->mat, matrix2);

    if (mesh != NULL) {
        self->meshImage = MeshImage_new(MeshImage_getMesh(mesh), MeshImage_getAnimation(mesh));
        self->boundingBox = BoundingBox_new(MeshImage_getAnimation(mesh));
    } else {
        self->meshImage = NULL;
    }

    self->mtex = tex;
    self->frame = frame2;
    self->f = 0;

    Character_reset(self->base.character);
    Matrix_set(Character_getTransform(self->base.character), &self->mat);
    Character_setCollision(self->base.character, false);
    Character_setCollidable(self->base.character, false);
    Character_setOnFloor(self->base.character, true);
    Character_setSpeedZero(self->base.character);

    return self;
}

void Corpse_render(Corpse *self, DirectX7 *g3d, int x1, int y1, int x2, int y2) {
    if (self->f <= 3600) GameObject_setHp(&self->base, 100);
    self->f++;
    if (self->f > 3600) GameObject_setHp(&self->base, 0);

    if (self->meshImage != NULL) {
        Morphing_setFrame(MeshImage_getAnimation(self->meshImage), self->frame);
        Matrix *fmat = DirectX7_computeFinalMatrix(g3d, &self->mat);
        if (!BoundingBox_isVisible(self->boundingBox, g3d, fmat, x1, y1, x2, y2)) return;

        MeshImage_setMatrix(self->meshImage, fmat);
        MeshImage_setTexture(self->meshImage, self->mtex);

        DirectX7_addRenderObject(g3d, self->meshImage, x1, y1, x2, y2);
        *MeshImage_getSzPtr(self->meshImage) += 5900;

        RenderObject *oldFloor = self->base.character->oldFloorPoly;
        if (oldFloor != NULL) {
            extern int RenderObject_getSz(RenderObject *ro);
            extern void RenderObject_setSz(RenderObject *ro, int sz);
            if (RenderObject_getSz(oldFloor) > *MeshImage_getSzPtr(self->meshImage))
                RenderObject_setSz(oldFloor, *MeshImage_getSzPtr(self->meshImage) - 1);
        }
    } else {
        int px = GameObject_getPart(&self->base);
        (void)px;
        Vector3D *spos = Sprite_getPosition(self->spr);
        extern int Character_getX(Character *ch);
        extern int Character_getY(Character *ch);
        extern int Character_getZ(Character *ch);
        Vector3D_set(spos, Character_getX(self->base.character),
                     Character_getY(self->base.character),
                     Character_getZ(self->base.character));
        Sprite_updateFrame(self->spr);
        Sprite_project(self->spr, DirectX7_getInvCamera(g3d), g3d);
        if (!Sprite_isVisible(self->spr, x1, y1, x2, y2)) return;

        DirectX7_addRenderObject(g3d, self->spr, x1, y1, x2, y2);
        *Sprite_getSzPtr(self->spr) += 5900;

        RenderObject *oldFloor = self->base.character->oldFloorPoly;
        if (oldFloor != NULL) {
            extern int RenderObject_getSz(RenderObject *ro);
            extern void RenderObject_setSz(RenderObject *ro, int sz);
            if (RenderObject_getSz(oldFloor) > *Sprite_getSzPtr(self->spr))
                RenderObject_setSz(oldFloor, *Sprite_getSzPtr(self->spr) - 1);
        }
    }
}
