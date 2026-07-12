/*
 * npc.h — porte fiel de code/AI/NPC.java
 */
#ifndef QE_AI_NPC_H
#define QE_AI_NPC_H

#include "bot.h"

typedef struct MeshImage    MeshImage;
typedef struct BoundingBox  BoundingBox;
typedef struct MultyTexture MultyTexture;
typedef struct Sprite       Sprite;
typedef struct Vertex       Vertex;
typedef struct DirectX7     DirectX7;
typedef struct Scene        Scene;

typedef struct NPC {
    Bot          base;

    int          animspeed;
    int          attackanimspeed;
    int          npc_damage;
    float        attackradius;
    int          jumpheight;
    float        jumpspeed;
    int          speed;
    int          reacttimer;
    int          attacktimer;

    int          attackState;
    int          whenEnemyIsFar;
    int          whenEnemyIsNear;

    int          moneyOnDeath;
    int          fragsOnDeath;
    int          fragsOnAnyDeath;
    int          damageSleepTime;
    int64_t      lastDamage;
    int64_t      maxEnemyDistance;
    bool         attackOnDamageOnlyPlayer;
    int          spawnerId;

    int          maxHp;
    int          model_height;
    int          state;

    MeshImage   *meshImage;

    int         *toAttack;
    int          toAttack_n;
    int         *toFollow;
    int          toFollow_n;

    bool         inPlayerTeam;
    int         *friendlyFire;
    int          friendlyFire_n;

    char       **onDeath;
    int          onDeath_n;

    GameObject  *enemy;
    GameObject  *follower;
    GameObject **unicalEnemies;
    int          unicalEnemies_n;
    int          unicalEnemies_cap;
    bool         walkingToEnemy;

    MultyTexture *mt;
    Vector3D     dir;

    Sprite *stayFront, *stayBack, *staySide;
    Sprite *attackFront, *attackBack, *attackSide;
    Sprite *deathFront, *deathBack, *deathSide;
    Sprite *damageFront, *damageBack, *damageSide;
    Sprite *walkFront, *walkBack, *walkSide;
    Sprite *currentSprite, *nextFront, *nextBack, *nextSide;

    Sprite *muzzleFlash;
    Vertex *muzzleFlashPos;
    int     muzzleFlashTimer;
    int64_t lastAttack;

    BoundingBox *boundingBox;
} NPC;

#define NPC_WALK_STATE   1
#define NPC_ATTACK_STATE 2

NPC  *NPC_new(Vector3D *pos, MeshImage *mi, int hp, MultyTexture *mts);
void  NPC_init(NPC *self, Vector3D *pos);
void  NPC_initStaySprite(NPC *self);
void  NPC_destroy(NPC *self);
bool  NPC_checkVisibility(NPC *self, DirectX7 *g3d, int x, int y, int z,
                          int x1, int y1, int x2, int y2);
void  NPC_render(NPC *self, DirectX7 *g3d, int x1, int y1, int x2, int y2);
void  NPC_setSpriteDir(NPC *self, DirectX7 *g3d);
void  NPC_action(NPC *self, Scene *scene);
void  NPC_drop(NPC *self, Scene *scene);
bool  NPC_damage(NPC *self, GameObject *obj, int dmg);
void  NPC_setSprite(NPC *self, Sprite *front, Sprite *side, Sprite *back);
bool  NPC_isTimeToRenew(NPC *self);

#endif
