/*
 * lvl_change.h — porte fiel de code/Gameplay/Objects/LVLChange.java
 */
#ifndef QE_GAMEPLAY_OBJECTS_LVL_CHANGE_H
#define QE_GAMEPLAY_OBJECTS_LVL_CHANGE_H

#include "../map/room_object.h"
#include "../../math/vector3d.h"
#include <stdbool.h>

typedef struct LVLChange {
    RoomObject base;
    Vector3D   pos;
    int        lvl;
    Vector3D   np;
    int        pRot;
    bool       saveMus;
    bool       fullMove;
    bool       showLoadScreen;
} LVLChange;

LVLChange *LVLChange_new(int x, int y, int z, Vector3D *newPos, int level);
void       LVLChange_destroy(LVLChange *self);
void       LVLChange_activate(LVLChange *self, House *house, Player *player, GameScreen *gs);

int  LVLChange_getPosX(LVLChange *self);
int  LVLChange_getPosY(LVLChange *self);
int  LVLChange_getPosZ(LVLChange *self);
void LVLChange_setPos(LVLChange *self, int x, int y, int z);

#endif
