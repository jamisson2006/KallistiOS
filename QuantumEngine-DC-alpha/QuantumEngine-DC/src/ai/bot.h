/*
 * bot.h — porte fiel de code/AI/Bot.java
 */
#ifndef QE_AI_BOT_H
#define QE_AI_BOT_H

#include <stdbool.h>
#include <stdint.h>
#include "../gameplay/objects/game_object.h"
#include "../math/vector3d.h"

typedef struct Blood    Blood;
typedef struct Trace    Trace;
typedef struct Scene    Scene;
typedef struct House    House;
typedef struct Portal   Portal;
typedef struct Player   Player;
typedef struct DirectX7 DirectX7;
typedef struct Mesh     Mesh;
typedef struct RenderObject RenderObject;

typedef struct Bot {
    GameObject  base;
    Blood      *blood;
    Vector3D    bloodSpeed;
    Vector3D    bloodPos;
    Trace      *bloodwall;
    int         fraction;
    int8_t      deathFall;
    bool        visiblityCheck;
    bool        hasBlood;
} Bot;

extern bool Bot_raycastTargetVisibility;

void Bot_init(Bot *self);
void Bot_set(Bot *self, Vector3D *pos);
void Bot_destroy(Bot *self);

void Bot_update(Bot *self, Scene *scene, Player *player);
bool Bot_damage(Bot *self, GameObject *obj, int dmg);

void Bot_renderBlood(Bot *self, DirectX7 *g3d, int sz);
void Bot_drop(Bot *self, Scene *scene);
void Bot_dropSide(Bot *self, Scene *scene);
void Bot_spawnBlood(Bot *self, Scene *scene);

void Bot_lookAt(Bot *self, int x, int z);

GameObject *Bot_findBot(Bot *self, void **objs, int objs_n, Bot *ignore, int *fractions, int fractions_n);
bool Bot_isTargetVisibleRaycast(Bot *self, House *house, GameObject *target);
bool Bot_isTargetInFOV(GameObject *observer, GameObject *observable);

Portal *Bot_commonPortal(House *house, int part1, int part2);
void    Bot_computeCentre(Portal *portal, Vector3D *center);
bool    Bot_contains(int *list, int list_n, int need);
int64_t Bot_sqr(int x);

void    Bot_increaseMeshSz(Mesh *mesh, int z);

Trace  *Bot_createTrace(Bot *self, Vector3D *vector3f, RenderObject *meshr);

#endif
