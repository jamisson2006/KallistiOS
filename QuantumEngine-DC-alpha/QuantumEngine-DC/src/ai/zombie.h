/*
 * zombie.h — porte fiel de code/AI/Zombie.java
 */
#ifndef QE_AI_ZOMBIE_H
#define QE_AI_ZOMBIE_H

#include "bot.h"

typedef struct MeshImage    MeshImage;
typedef struct BoundingBox  BoundingBox;
typedef struct MultyTexture MultyTexture;
typedef struct DirectX7     DirectX7;
typedef struct Scene        Scene;

typedef struct Zombie {
    Bot          base;
    MeshImage   *meshImage;
    GameObject  *enemy;
    bool         notCol;
    int          state;
    Vector3D     dir;
} Zombie;

extern int8_t       Zombie_fallDeath;
extern bool         Zombie_bloodHas;
extern int          Zombie_maxHP;
extern int          Zombie_model_height;
extern BoundingBox *Zombie_boundingBox;
extern int         *Zombie_attackTo;
extern int          Zombie_attackTo_n;
extern int          Zombie_enemyReaction;
extern int          Zombie_animSpeed;
extern int          Zombie_attackDamage;
extern int          Zombie_walkSpeed;
extern int          Zombie_attackAnimSpeed;
extern float        Zombie_jumpSpeed;
extern int          Zombie_reactTimer;
extern MultyTexture *Zombie_texture;
extern int          Zombie_jumpHeight;
extern int          Zombie_attackState;
extern MeshImage   *Zombie_model;
extern float        Zombie_attackRadius;
extern int          Zombie_moneyOnDeath;
extern int          Zombie_attackTimer;
extern int          Zombie_AI;

Zombie *Zombie_new(Vector3D *pos);
void    Zombie_set(Zombie *self, Vector3D *pos);
void    Zombie_destroy(Zombie *self);
void    Zombie_render(Zombie *self, DirectX7 *g3d, int x1, int y1, int x2, int y2);
void    Zombie_action(Zombie *self, Scene *scene);
void    Zombie_drop(Zombie *self, Scene *scene);

#endif
