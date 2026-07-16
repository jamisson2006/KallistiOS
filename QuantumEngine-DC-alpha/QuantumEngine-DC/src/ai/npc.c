/*
 * npc.c — porte fiel de code/AI/NPC.java
 */
#include "npc.h"
#include "player.h"
#include "../math/matrix.h"
#include <stdlib.h>
#include <math.h>

typedef struct MeshImage    MeshImage;
typedef struct BoundingBox  BoundingBox;
typedef struct MultyTexture MultyTexture;
typedef struct Sprite       Sprite;
typedef struct Vertex       Vertex;
typedef struct Scene        Scene;
typedef struct House        House;
typedef struct Portal       Portal;
typedef struct DirectX7     DirectX7;
typedef struct Matrix       Matrix;
typedef struct RenderObject RenderObject;
typedef struct Texture      Texture;
typedef struct RImg         RImg;

extern MeshImage   *MeshImage_new(void *mesh, void *anim);
extern void        *MeshImage_getMesh(MeshImage *mi);
extern void        *MeshImage_getAnimation(MeshImage *mi);
extern void         MeshImage_setMatrix(MeshImage *mi, Matrix *mat);
extern void         MeshImage_setTexture(MeshImage *mi, MultyTexture *mt);
extern void         MeshImage_setFrame(MeshImage *mi, int frame);
extern int         *MeshImage_getSzPtr(MeshImage *mi);

extern int          Mesh_maxY(void *mesh);
extern int          Mesh_minY(void *mesh);

extern BoundingBox *BoundingBox_new(void *anim);
extern bool         BoundingBox_isVisible(BoundingBox *bb, DirectX7 *g3d, Matrix *fmat,
                                          int x1, int y1, int x2, int y2);

extern Matrix      *DirectX7_computeFinalMatrix(DirectX7 *g3d, Matrix *mat);
extern Matrix      *DirectX7_getInvCamera(DirectX7 *g3d);
extern Matrix      *DirectX7_getCamera(DirectX7 *g3d);
extern void         DirectX7_addRenderObject(DirectX7 *g3d, void *obj, int x1, int y1, int x2, int y2);

extern Matrix      *Character_getTransform(Character *ch);
extern int          Character_getRadius(Character *ch);
extern int          Character_getHeight(Character *ch);
extern void         Character_moveZ(Character *ch, int amount);
extern bool         Character_isCollision(Character *ch);
extern void         Character_jump2(Character *ch, int height, float speed);
extern int64_t      Character_distance(Character *a, Character *b);

extern void         GameObject_setHp(GameObject *self, int hp);
extern int          GameObject_getHp(GameObject *self);
extern void         GameObject_setCharacterSize(GameObject *self, int height);
extern int          GameObject_getFrameInter(GameObject *self);
extern int          GameObject_getFrameInterDiv(GameObject *self);
extern bool         GameObject_isDead(GameObject *self);
extern int          GameObject_getPart(GameObject *self);
extern int          GameObject_getPosX(GameObject *self);
extern int          GameObject_getPosY(GameObject *self);
extern int          GameObject_getPosZ(GameObject *self);

extern House       *Scene_getHouse(Scene *scene);
extern void       **House_getObjects(House *house);
extern int          House_getObjectsCount(House *house);
extern bool         House_isNear(House *house, int p1, int p2);
extern int          Scene_getNext(Scene *scene, int from, int to);
extern Player      *Scene_findPlayer(Scene *scene);
extern void         Scene_runScript(Scene *scene, void *args);

extern int64_t      qe_current_ms(void);

extern Vector3D    *Sprite_getPosition(Sprite *spr);
extern void         Sprite_updateFrame(Sprite *spr);
extern void         Sprite_project(Sprite *spr, Matrix *invCam, DirectX7 *g3d);
extern int          Sprite_isVisible(Sprite *spr, int x1, int y1, int x2, int y2);
extern int         *Sprite_getSzPtr(Sprite *spr);
extern int          Sprite_getHeight(Sprite *spr);
extern void         Sprite_setMirX(Sprite *spr, bool mir);
extern int64_t      Sprite_getAnimationBegin(Sprite *spr);
extern void         Sprite_setAnimationBegin(Sprite *spr, int64_t t);
extern Texture    **Sprite_getTextures(Sprite *spr, int *out_n);
extern int          Sprite_getAnimSpeed(Sprite *spr);
extern Vector3D    *Sprite_getPos(Sprite *spr);

extern void         Vertex_transform(Vertex *v, const Matrix *mat);
extern int          Vertex_getSx(Vertex *v);
extern int          Vertex_getSy(Vertex *v);
extern int          Vertex_getRz(Vertex *v);

extern int          MathUtils_fixDegree(int d);
extern int          MathUtils_getAnglez(int x1, int z1, int x2, int z2);
extern int          Matrix_getRotZ(Matrix *m);

extern RenderObject *Character_getOldFloorPoly(Character *ch);
extern int           RenderObject_getSz(RenderObject *ro);
extern void          RenderObject_setSz(RenderObject *ro, int sz);

extern int           Texture_getScale(Texture *tex);

static Matrix s_tmpMatrix;

NPC *NPC_new(Vector3D *pos, MeshImage *mi, int hp, MultyTexture *mts) {
    NPC *self = (NPC *)calloc(1, sizeof(NPC));
    Bot_init(&self->base);

    self->animspeed = 140;
    self->attackanimspeed = 700;
    self->npc_damage = 1;
    self->attackradius = 1.2f;
    self->jumpheight = 140;
    self->jumpspeed = 1.2f;
    self->speed = 140;
    self->reacttimer = 8;
    self->attacktimer = 8;

    self->attackState = NPC_ATTACK_STATE;
    self->whenEnemyIsFar = NPC_WALK_STATE;
    self->whenEnemyIsNear = NPC_ATTACK_STATE;

    self->moneyOnDeath = 0;
    self->fragsOnDeath = 0;
    self->fragsOnAnyDeath = 0;
    self->damageSleepTime = 0;
    self->lastDamage = 0;
    self->maxEnemyDistance = -1;
    self->attackOnDamageOnlyPlayer = false;
    self->spawnerId = -1;

    self->maxHp = hp;
    self->meshImage = mi;
    if (mi != NULL) {
        self->model_height = Mesh_maxY(MeshImage_getMesh(mi)) - Mesh_minY(MeshImage_getMesh(mi));
        self->boundingBox = BoundingBox_new(MeshImage_getAnimation(mi));
    }
    self->mt = mts;
    self->base.fraction = 3;

    self->toAttack_n = 2;
    self->toAttack = (int *)malloc(2 * sizeof(int));
    self->toAttack[0] = 1; self->toAttack[1] = 2;

    self->toFollow_n = 1;
    self->toFollow = (int *)malloc(1 * sizeof(int));
    self->toFollow[0] = 0;

    self->base.base.base.name = "NPC";
    self->base.visiblityCheck = true;

    NPC_init(self, pos);
    return self;
}

void NPC_init(NPC *self, Vector3D *pos) {
    if (self->meshImage != NULL) MeshImage_setFrame(self->meshImage, 0);
    Bot_set(&self->base, pos);
    GameObject_setHp(&self->base.base, self->maxHp);

    self->dir.x = pos->x + 50; self->dir.y = pos->y; self->dir.z = pos->z + 50;
    Bot_lookAt(&self->base, pos->x + 50, pos->z + 50);

    NPC_setSprite(self, self->stayFront, self->staySide, self->stayBack);
    if (self->stayFront != NULL) {
        int scale = Texture_getScale(Sprite_getTextures(self->stayFront, NULL)[0]);
        self->model_height = Sprite_getHeight(self->stayFront) * ((scale < 2) ? 2 : 1);
    }

    GameObject_setCharacterSize(&self->base.base, self->model_height);

    if (self->unicalEnemies != NULL) {
        self->unicalEnemies_n = 0;
    }
    self->lastDamage = 0;
    self->state = -1;
    self->enemy = NULL;
    self->follower = NULL;
    self->currentSprite = NULL;
    self->walkingToEnemy = false;
}

void NPC_initStaySprite(NPC *self) {
    NPC_setSprite(self, self->stayFront, self->staySide, self->stayBack);
    if (self->stayFront != NULL) {
        int scale = Texture_getScale(Sprite_getTextures(self->stayFront, NULL)[0]);
        self->model_height = Sprite_getHeight(self->stayFront) * ((scale < 2) ? 2 : 1);
    }
}

void NPC_destroy(NPC *self) {
    Bot_destroy(&self->base);
    self->enemy = NULL;
    self->follower = NULL;
    self->meshImage = NULL;
    self->boundingBox = NULL;
}

bool NPC_checkVisibility(NPC *self, DirectX7 *g3d, int x, int y, int z,
                         int x1, int y1, int x2, int y2) {
    if (self->boundingBox != NULL) {
        Matrix_setIdentity(&s_tmpMatrix);
        Matrix_setPosition(&s_tmpMatrix, x, y, z);
        Matrix *fmat = DirectX7_computeFinalMatrix(g3d, &s_tmpMatrix);
        return BoundingBox_isVisible(self->boundingBox, g3d, fmat, x1, y1, x2, y2);
    }

    if (self->stayFront != NULL) {
        Vector3D *spos = Sprite_getPosition(self->stayFront);
        spos->x = x; spos->y = y; spos->z = z;
        Sprite_project(self->stayFront, DirectX7_getInvCamera(g3d), g3d);
        return Sprite_isVisible(self->stayFront, x1, y1, x2, y2);
    }

    return false;
}

void NPC_render(NPC *self, DirectX7 *g3d, int x1, int y1, int x2, int y2) {
    if (!self->base.base.base.visible) return;

    if (self->meshImage != NULL) {
        Matrix *mat = Character_getTransform(self->base.base.character);
        Matrix *fmat = DirectX7_computeFinalMatrix(g3d, mat);
        if (self->boundingBox != NULL && !BoundingBox_isVisible(self->boundingBox, g3d, fmat, x1, y1, x2, y2))
            return;

        if (mat->m11 == 16384) {
            if (self->state == NPC_WALK_STATE) MeshImage_setFrame(self->meshImage,
                GameObject_getFrameInter(&self->base.base) * self->animspeed / 100);
            if (self->state == NPC_ATTACK_STATE) MeshImage_setFrame(self->meshImage,
                GameObject_getFrameInter(&self->base.base) * self->attackanimspeed / 100);
        }

        MeshImage_setMatrix(self->meshImage, fmat);
        MeshImage_setTexture(self->meshImage, self->mt);
        DirectX7_addRenderObject(g3d, self->meshImage, x1, y1, x2, y2);
        *MeshImage_getSzPtr(self->meshImage) += Character_getRadius(self->base.base.character);

        RenderObject *oldFloor = Character_getOldFloorPoly(self->base.base.character);
        if (oldFloor != NULL && RenderObject_getSz(oldFloor) > *MeshImage_getSzPtr(self->meshImage))
            RenderObject_setSz(oldFloor, *MeshImage_getSzPtr(self->meshImage) - 1);
    } else {
        NPC_setSpriteDir(self, g3d);

        if (self->currentSprite != NULL) {
            Vector3D *spos = Sprite_getPosition(self->currentSprite);
            spos->x = GameObject_getPosX(&self->base.base);
            spos->y = GameObject_getPosY(&self->base.base);
            spos->z = GameObject_getPosZ(&self->base.base);

            Sprite_updateFrame(self->currentSprite);
            Sprite_project(self->currentSprite, DirectX7_getInvCamera(g3d), g3d);

            if (!Sprite_isVisible(self->currentSprite, x1, y1, x2, y2)) return;

            DirectX7_addRenderObject(g3d, self->currentSprite, x1, y1, x2, y2);
            *Sprite_getSzPtr(self->currentSprite) += Character_getRadius(self->base.base.character) * 2;

            RenderObject *oldFloor = Character_getOldFloorPoly(self->base.base.character);
            if (oldFloor != NULL && RenderObject_getSz(oldFloor) > *Sprite_getSzPtr(self->currentSprite))
                RenderObject_setSz(oldFloor, *Sprite_getSzPtr(self->currentSprite) - 1);
        }
    }

    if (self->muzzleFlash != NULL && qe_current_ms() - self->lastAttack <= self->muzzleFlashTimer) {
        Vertex_transform(self->muzzleFlashPos, &self->base.base.character->transform);
        Vector3D *mpos = Sprite_getPos(self->muzzleFlash);
        mpos->x = Vertex_getSx(self->muzzleFlashPos);
        mpos->y = Vertex_getSy(self->muzzleFlashPos) - Sprite_getHeight(self->muzzleFlash) / 2;
        mpos->z = Vertex_getRz(self->muzzleFlashPos);
        Sprite_project(self->muzzleFlash, DirectX7_getInvCamera(g3d), g3d);
        DirectX7_addRenderObject(g3d, self->muzzleFlash, x1, y1, x2, y2);
        *Sprite_getSzPtr(self->muzzleFlash) += Character_getRadius(self->base.base.character);
    }
    Bot_renderBlood(&self->base, g3d, 1500);
}

void NPC_setSpriteDir(NPC *self, DirectX7 *g3d) {
    Sprite *nextSpr = NULL;
    int degree = 0;

    Matrix *camera = DirectX7_getCamera(g3d);
    if (camera != NULL) {
        degree = MathUtils_fixDegree(180 + MathUtils_getAnglez(
            GameObject_getPosX(&self->base.base), GameObject_getPosZ(&self->base.base),
            camera->m03, camera->m23) - Matrix_getRotZ(Character_getTransform(self->base.base.character)));
    }

    if (self->nextFront != NULL) nextSpr = self->nextFront;

    if (self->nextBack != NULL && degree > 90 && degree < 270) nextSpr = self->nextBack;

    if (self->nextSide != NULL && degree > 45 && degree < 135) {
        Sprite_setMirX(self->nextSide, false);
        nextSpr = self->nextSide;
    }

    if (self->nextSide != NULL && degree > 225 && degree < 315) {
        Sprite_setMirX(self->nextSide, true);
        nextSpr = self->nextSide;
    }

    if (self->currentSprite != NULL && nextSpr != NULL &&
        (self->currentSprite == self->deathFront || self->currentSprite == self->deathSide ||
         self->currentSprite == self->deathBack)) return;

    if (self->currentSprite != NULL && nextSpr != NULL &&
        (self->currentSprite == self->damageFront || self->currentSprite == self->damageSide ||
         self->currentSprite == self->damageBack ||
         self->currentSprite == self->attackFront || self->currentSprite == self->attackSide ||
         self->currentSprite == self->attackBack) &&
        !(nextSpr == self->damageFront || nextSpr == self->damageSide || nextSpr == self->damageBack)) {
        int texs_n = 0;
        Sprite_getTextures(self->currentSprite, &texs_n);
        if ((int64_t)(Sprite_getAnimationBegin(self->currentSprite) - qe_current_ms()) >=
            -(int64_t)texs_n * 1000 / Sprite_getAnimSpeed(self->currentSprite)) return;
    }

    if (nextSpr == NULL || nextSpr == self->currentSprite) return;
    self->currentSprite = nextSpr;

    if (self->currentSprite != NULL) Sprite_setAnimationBegin(self->currentSprite, qe_current_ms());
}

static void npc_walk(NPC *self, int spd) {
    Character_moveZ(self->base.base.character, spd);
    NPC_setSprite(self, self->walkFront, self->walkSide, self->walkBack);
}

static void npc_attack(NPC *self, Scene *scene, GameObject *en, int dam) {
    bool wasAlive = !GameObject_isDead(en);

    GameObject_damage(en, dam);
    NPC_setSprite(self, self->attackFront, self->attackSide, self->attackBack);
    if (self->muzzleFlash != NULL) self->lastAttack = qe_current_ms();

    if (self->inPlayerTeam && wasAlive && GameObject_isDead(en)) {
        NPC *npc_target = (NPC *)en;
        Player *player = Scene_findPlayer(scene);
        if (player != NULL) {
            player->frags += npc_target->fragsOnDeath;
            player->money += npc_target->moneyOnDeath;
        }

        Scene_runScript(scene, npc_target->onDeath);
    }
}

void NPC_action(NPC *self, Scene *scene) {
    if (qe_current_ms() - self->lastDamage < self->damageSleepTime) return;

    if (abs(GameObject_getFrameInterDiv(&self->base.base)) % self->reacttimer == 2) {
        House *house = Scene_getHouse(scene);
        void **objs = House_getObjects(house);
        int objs_n = House_getObjectsCount(house);

        if (self->unicalEnemies != NULL && self->unicalEnemies_n > 0) {
            for (int i = 0; i < self->unicalEnemies_n; i++) {
                if (GameObject_isDead(self->unicalEnemies[i])) {
                    for (int j = i; j < self->unicalEnemies_n - 1; j++)
                        self->unicalEnemies[j] = self->unicalEnemies[j + 1];
                    self->unicalEnemies_n--;
                    i--;
                }
            }
        }

        if (self->enemy != NULL && GameObject_isDead(self->enemy)) self->enemy = NULL;

        GameObject *oldEnemy = self->enemy;
        self->enemy = Bot_findBot(&self->base, objs, objs_n,
                                  (Bot *)self, self->toAttack, self->toAttack_n);

        if (self->unicalEnemies != NULL) {
            GameObject *enemy2 = Bot_findBot(&self->base, (void **)self->unicalEnemies,
                                             self->unicalEnemies_n, (Bot *)self, NULL, 0);
            if (enemy2 != NULL) {
                if (self->enemy == NULL) self->enemy = enemy2;
                else if (Character_distance(self->base.base.character, enemy2->character) <
                         Character_distance(self->base.base.character, self->enemy->character)) {
                    self->enemy = enemy2;
                }
            }
        }

        if (oldEnemy != NULL) {
            if (self->enemy == NULL) self->enemy = oldEnemy;
            else if (Character_distance(self->base.base.character, oldEnemy->character) <
                     Character_distance(self->base.base.character, self->enemy->character)) {
                self->enemy = oldEnemy;
            }
        }

        if (self->enemy != NULL && self->maxEnemyDistance > -1) {
            if (Character_distance(self->base.base.character, self->enemy->character) >=
                self->maxEnemyDistance * self->maxEnemyDistance)
                self->enemy = NULL;
        }

        self->follower = Bot_findBot(&self->base, objs, objs_n,
                                     (Bot *)self, self->toFollow, self->toFollow_n);

        GameObject *target = self->enemy != NULL ? self->enemy : self->follower;

        if (target != NULL) {
            if (self->follower != NULL) {
                int64_t followerDist = Character_distance(self->base.base.character,
                                                          self->follower->character);
                if (followerDist >= 90000000LL) target = self->follower;
                if (target == self->follower && followerDist < 9000000LL) target = self->enemy;
            }

            self->state = -1;

            if (target != NULL && target == self->enemy &&
                self->enemy != oldEnemy &&
                !Bot_isTargetVisibleRaycast(&self->base, house, target)) {
                target = NULL;
                self->enemy = NULL;
            }

            if (target != NULL) {
                self->walkingToEnemy = (GameObject_getPart(&self->base.base) == GameObject_getPart(target));

                if (self->walkingToEnemy) {
                    Matrix *em = Character_getTransform(target->character);
                    self->dir.x = em->m03; self->dir.y = em->m13; self->dir.z = em->m23;
                } else {
                    int nextPart = Scene_getNext(scene, GameObject_getPart(&self->base.base),
                                                 GameObject_getPart(target));
                    Portal *portal = Bot_commonPortal(house, GameObject_getPart(&self->base.base), nextPart);
                    if (portal != NULL) Bot_computeCentre(portal, &self->dir);
                }

                Bot_lookAt(&self->base, self->dir.x, self->dir.z);

                int64_t dist = Character_distance(self->base.base.character, target->character);
                int64_t atkRange = Bot_sqr(Character_getRadius(self->base.base.character) +
                                   Character_getRadius(target->character));

                if (self->walkingToEnemy &&
                    dist <= (int64_t)(atkRange * self->attackradius)) {
                    if (target == self->enemy) self->state = self->whenEnemyIsNear;
                } else {
                    if (Character_isCollision(self->base.base.character))
                        Character_jump2(self->base.base.character, self->jumpheight, self->jumpspeed);
                    self->state = self->whenEnemyIsFar;
                }
            }
        }
    }

    if (self->state == NPC_WALK_STATE) npc_walk(self, self->speed);

    if (self->state == self->attackState &&
        GameObject_getFrameInterDiv(&self->base.base) % self->attacktimer == 0 &&
        self->enemy != NULL && self->walkingToEnemy)
        npc_attack(self, scene, self->enemy, self->npc_damage);
}

void NPC_drop(NPC *self, Scene *scene) {
    Bot_drop(&self->base, scene);
    self->state = -1;
}

void NPC_setSprite(NPC *self, Sprite *front, Sprite *side, Sprite *back) {
    self->nextFront = front;
    self->nextSide = side;
    self->nextBack = back;
}

bool NPC_isTimeToRenew(NPC *self) {
    if (self->currentSprite != NULL) {
        if (!(self->currentSprite == self->deathFront || self->currentSprite == self->deathSide ||
              self->currentSprite == self->deathBack) && GameObject_isDead(&self->base.base)) {
            NPC_setSprite(self, self->deathFront, self->deathSide, self->deathBack);
        }
        int texs_n = 0;
        Sprite_getTextures(self->currentSprite, &texs_n);
        if (Sprite_getAnimationBegin(self->currentSprite) - qe_current_ms() <
            -(int64_t)texs_n * 1000 / Sprite_getAnimSpeed(self->currentSprite) &&
            (self->currentSprite == self->deathFront || self->currentSprite == self->deathSide ||
             self->currentSprite == self->deathBack)) {
            return GameObject_isDead(&self->base.base);
        }
        return false;
    }

    return GameObject_isTimeToRenew(&self->base.base);
}

bool NPC_damage(NPC *self, GameObject *obj, int dmg) {
    if (self->friendlyFire != NULL) {
        Bot *objBot = (Bot *)obj;
        if (objBot != NULL && Bot_contains(self->friendlyFire, self->friendlyFire_n, objBot->fraction))
            return false;
    }

    if (dmg >= 0) {
        if (qe_current_ms() - self->lastDamage > self->damageSleepTime)
            self->lastDamage = qe_current_ms();

        int tf = -1;
        Bot *objBot = (Bot *)obj;
        if (objBot != NULL) {
            tf = objBot->fraction;
            if (self->attackOnDamageOnlyPlayer && tf != 0) tf = -1;
        }

        bool canAtk = false;

        if (tf != -1) {
            bool inToAttack = Bot_contains(self->toAttack, self->toAttack_n, tf);
            canAtk |= inToAttack;

            if (self->unicalEnemies != NULL && !inToAttack) {
                bool found = false;
                for (int i = 0; i < self->unicalEnemies_n; i++) {
                    if (self->unicalEnemies[i] == obj) { found = true; break; }
                }
                if (!found) {
                    if (self->unicalEnemies_n >= self->unicalEnemies_cap) {
                        self->unicalEnemies_cap = self->unicalEnemies_cap == 0 ? 8 : self->unicalEnemies_cap * 2;
                        self->unicalEnemies = (GameObject **)realloc(self->unicalEnemies,
                            self->unicalEnemies_cap * sizeof(GameObject *));
                    }
                    self->unicalEnemies[self->unicalEnemies_n++] = obj;
                }
                canAtk = true;
            }
        }
        if (GameObject_getHp(&self->base.base) - dmg > 0)
            NPC_setSprite(self, self->damageFront, self->damageSide, self->damageBack);

        if (self->enemy == NULL && canAtk) self->enemy = obj;
    }

    return Bot_damage(&self->base, obj, dmg);
}
