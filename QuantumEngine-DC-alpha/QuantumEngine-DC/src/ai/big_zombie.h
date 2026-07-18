/*
 * big_zombie.h — porte fiel de code/AI/BigZombie.java
 */
#ifndef QE_AI_BIG_ZOMBIE_H
#define QE_AI_BIG_ZOMBIE_H

#include "bot.h"

typedef struct MeshImage    MeshImage;
typedef struct BoundingBox  BoundingBox;
typedef struct MultyTexture MultyTexture;
typedef struct DirectX7     DirectX7;
typedef struct Scene        Scene;

typedef struct BigZombie {
    Bot          base;
    MeshImage   *meshImage;
    GameObject  *enemy;
    bool         notCol;
    int          state;
    Vector3D     dir;
} BigZombie;

extern int8_t       BigZombie_fallDeath;
extern bool         BigZombie_bloodHas;
extern int          BigZombie_max_hp;
extern int          BigZombie_model_height;
extern BoundingBox *BigZombie_boundingBox;
extern int         *BigZombie_attackTo;
extern int          BigZombie_attackTo_n;
extern int          BigZombie_enemyReaction;
extern int          BigZombie_animSpeed;
extern int          BigZombie_walkSpeed;
extern int          BigZombie_moneyOnDeath;
extern int          BigZombie_attackState;
extern float        BigZombie_jumpSpeed;
extern int          BigZombie_attackTimer;
extern int          BigZombie_attackAnimSpeed;
extern float        BigZombie_jumpSpeed2;
extern int          BigZombie_reactTimer;
extern int          BigZombie_AI;
extern MultyTexture *BigZombie_texture;
extern int          BigZombie_jumpHeight2;
extern int          BigZombie_jumpHeight;
extern int          BigZombie_attackDamage;
extern float        BigZombie_attackRadius;
extern MeshImage   *BigZombie_model;

BigZombie *BigZombie_new(Vector3D *pos);
void       BigZombie_set(BigZombie *self, Vector3D *pos);
void       BigZombie_destroy(BigZombie *self);
void       BigZombie_render(BigZombie *self, DirectX7 *g3d, int x1, int y1, int x2, int y2);
void       BigZombie_action(BigZombie *self, Scene *scene);

#endif
