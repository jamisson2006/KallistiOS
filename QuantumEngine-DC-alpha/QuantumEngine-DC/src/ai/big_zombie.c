/*
 * big_zombie.c — porte fiel de code/AI/BigZombie.java
 */
#include "big_zombie.h"
#include "../math/matrix.h"
#include <stdlib.h>
#include <math.h>

typedef struct MeshImage    MeshImage;
typedef struct BoundingBox  BoundingBox;
typedef struct MultyTexture MultyTexture;
typedef struct Scene        Scene;
typedef struct House        House;
typedef struct Portal       Portal;
typedef struct DirectX7     DirectX7;
typedef struct Matrix       Matrix;

extern MeshImage   *MeshImage_new(void *mesh, void *anim);
extern void        *MeshImage_getMesh(MeshImage *mi);
extern void        *MeshImage_getAnimation(MeshImage *mi);
extern void         MeshImage_setMatrix(MeshImage *mi, Matrix *mat);
extern void         MeshImage_setTexture(MeshImage *mi, MultyTexture *mt);
extern void         MeshImage_setFrame(MeshImage *mi, int frame);
extern int         *MeshImage_getSzPtr(MeshImage *mi);

extern BoundingBox *BoundingBox_new(void *anim);
extern bool         BoundingBox_isVisible(BoundingBox *bb, DirectX7 *g3d, Matrix *fmat,
                                          int x1, int y1, int x2, int y2);

extern Matrix      *DirectX7_computeFinalMatrix(DirectX7 *g3d, Matrix *mat);
extern void         DirectX7_addRenderObject(DirectX7 *g3d, void *obj, int x1, int y1, int x2, int y2);

extern Matrix      *Character_getTransform(Character *ch);
extern int          Character_getRadius(Character *ch);
extern void         Character_moveZ(Character *ch, int amount);
extern bool         Character_isCollision(Character *ch);
extern void         Character_jump2(Character *ch, int height, float speed);
extern int64_t      Character_distance(Character *a, Character *b);

extern void         GameObject_setHp(GameObject *self, int hp);
extern void         GameObject_setCharacterSize(GameObject *self, int height);
extern int          GameObject_getFrameInter(GameObject *self);
extern int          GameObject_getFrameInterDiv(GameObject *self);
extern bool         GameObject_isDead(GameObject *self);
extern int          GameObject_getPart(GameObject *self);

extern House       *Scene_getHouse(Scene *scene);
extern void       **House_getObjects(House *house);
extern int          House_getObjectsCount(House *house);
extern bool         House_isNear(House *house, int p1, int p2);
extern int          Scene_getNext(Scene *scene, int from, int to);

extern int          QFPS_frames;

extern RenderObject *Character_getOldFloorPoly(Character *ch);
extern void          RenderObject_setSz(RenderObject *ro, int sz);
extern int           RenderObject_getSz(RenderObject *ro);
typedef struct RenderObject RenderObject;

int8_t       BigZombie_fallDeath = 1;
bool         BigZombie_bloodHas = true;
int          BigZombie_max_hp = 400;
int          BigZombie_model_height = 2000;
BoundingBox *BigZombie_boundingBox = NULL;
int         *BigZombie_attackTo = NULL;
int          BigZombie_attackTo_n = 0;
int          BigZombie_enemyReaction = 2;
int          BigZombie_animSpeed = 135;
int          BigZombie_walkSpeed = 135;
int          BigZombie_moneyOnDeath = 30;
int          BigZombie_attackState = 2;
float        BigZombie_jumpSpeed = 1.2f;
int          BigZombie_attackTimer = 14;
int          BigZombie_attackAnimSpeed = 270;
float        BigZombie_jumpSpeed2 = 1.5f;
int          BigZombie_reactTimer = 8;
int          BigZombie_AI = 1;
MultyTexture *BigZombie_texture = NULL;
int          BigZombie_jumpHeight2 = 202;
int          BigZombie_jumpHeight = 202;
int          BigZombie_attackDamage = 7;
float        BigZombie_attackRadius = 1.2f;
MeshImage   *BigZombie_model = NULL;

BigZombie *BigZombie_new(Vector3D *pos) {
    BigZombie *self = (BigZombie *)calloc(1, sizeof(BigZombie));
    Bot_init(&self->base);

    if (BigZombie_model != NULL) {
        self->meshImage = MeshImage_new(MeshImage_getMesh(BigZombie_model),
                                        MeshImage_getAnimation(BigZombie_model));
        BigZombie_boundingBox = BoundingBox_new(MeshImage_getAnimation(BigZombie_model));
    }

    self->base.base.base.name = "BIG_ZOMBIE";
    Matrix *t = Character_getTransform(self->base.base.character);
    Matrix_setIdentity(t);
    BigZombie_set(self, pos);
    self->base.fraction = 2;
    self->base.deathFall = BigZombie_fallDeath;
    self->base.hasBlood = BigZombie_bloodHas;
    return self;
}

void BigZombie_set(BigZombie *self, Vector3D *pos) {
    Bot_set(&self->base, pos);
    GameObject_setHp(&self->base.base, BigZombie_max_hp);
    GameObject_setCharacterSize(&self->base.base, BigZombie_model_height);
    self->dir.x = pos->x; self->dir.y = pos->y; self->dir.z = pos->z + 50;
    Bot_lookAt(&self->base, pos->x, pos->z + 50);
    self->notCol = false;
    self->state = -1;
}

void BigZombie_destroy(BigZombie *self) {
    Bot_destroy(&self->base);
    self->enemy = NULL;
    self->meshImage = NULL;
}

void BigZombie_render(BigZombie *self, DirectX7 *g3d, int x1, int y1, int x2, int y2) {
    if (!self->base.base.base.visible) return;

    Matrix *mat = Character_getTransform(self->base.base.character);
    Matrix *finalMatrix = DirectX7_computeFinalMatrix(g3d, mat);
    if (!BoundingBox_isVisible(BigZombie_boundingBox, g3d, finalMatrix, x1, y1, x2, y2)) return;

    if (mat->m11 == 16384) {
        if (self->state == 1) MeshImage_setFrame(self->meshImage,
            GameObject_getFrameInter(&self->base.base) * BigZombie_animSpeed / 100);
        if (self->state == 2) MeshImage_setFrame(self->meshImage,
            GameObject_getFrameInter(&self->base.base) * BigZombie_attackAnimSpeed / 100);
    }

    MeshImage_setMatrix(self->meshImage, finalMatrix);
    MeshImage_setTexture(self->meshImage, BigZombie_texture);

    DirectX7_addRenderObject(g3d, self->meshImage, x1, y1, x2, y2);
    *MeshImage_getSzPtr(self->meshImage) += Character_getRadius(self->base.base.character);

    RenderObject *oldFloor = Character_getOldFloorPoly(self->base.base.character);
    if (oldFloor != NULL && RenderObject_getSz(oldFloor) > *MeshImage_getSzPtr(self->meshImage))
        RenderObject_setSz(oldFloor, *MeshImage_getSzPtr(self->meshImage) - 1);

    Bot_renderBlood(&self->base, g3d, 1500);
}

void BigZombie_action(BigZombie *self, Scene *scene) {
    if (abs(GameObject_getFrameInterDiv(&self->base.base)) % BigZombie_reactTimer == 1) {
        House *house = Scene_getHouse(scene);
        void **objects = House_getObjects(house);
        int objects_n = House_getObjectsCount(house);

        if ((self->enemy != NULL && GameObject_isDead(self->enemy)) ||
            (self->enemy != NULL && QFPS_frames == 0)) {
            self->enemy = NULL;
        }

        self->enemy = Bot_findBot(&self->base, objects, objects_n,
                                  (Bot *)self, BigZombie_attackTo, BigZombie_attackTo_n);

        if (self->enemy != NULL) {
            Vector3D *walkTo = &self->dir;
            if (Bot_isTargetVisibleRaycast(&self->base, house, self->enemy) &&
                House_isNear(house, GameObject_getPart(&self->base.base),
                             GameObject_getPart(self->enemy))) {
                Matrix *em = Character_getTransform(self->enemy->character);
                walkTo->x = em->m03; walkTo->y = em->m13; walkTo->z = em->m23;
                self->notCol = true;
            } else {
                int nextPart = Scene_getNext(scene, GameObject_getPart(&self->base.base),
                                             GameObject_getPart(self->enemy));
                Portal *portal = Bot_commonPortal(house, GameObject_getPart(&self->base.base), nextPart);
                if (portal != NULL) Bot_computeCentre(portal, walkTo);
                self->notCol = false;
            }

            if (self->dir.x != 0 && self->dir.z != 0) {
                Bot_lookAt(&self->base, self->dir.x, self->dir.z);
            }

            int64_t distance = Character_distance(self->base.base.character, self->enemy->character);
            int64_t atkRange = Bot_sqr(Character_getRadius(self->base.base.character) +
                               Character_getRadius(self->enemy->character));

            if (self->notCol && distance <= (int64_t)(atkRange * BigZombie_attackRadius)) {
                self->state = BigZombie_enemyReaction;
            } else {
                if (Character_isCollision(self->base.base.character) && !self->notCol)
                    Character_jump2(self->base.base.character, BigZombie_jumpHeight, BigZombie_jumpSpeed);

                if (distance > Bot_sqr(Character_getRadius(self->base.base.character) * 11))
                    Character_jump2(self->base.base.character, BigZombie_jumpHeight2, BigZombie_jumpSpeed2);

                self->state = BigZombie_AI;
            }
        } else {
            self->state = -1;
        }
    }

    if (self->state == 1) Character_moveZ(self->base.base.character, BigZombie_walkSpeed);

    if (self->state == BigZombie_attackState &&
        GameObject_getFrameInterDiv(&self->base.base) % BigZombie_attackTimer == 0 && self->notCol)
        GameObject_damage(self->enemy, BigZombie_attackDamage);
}
