/*
 * lvl_change.c — porte fiel de code/Gameplay/Objects/LVLChange.java
 */
#include "lvl_change.h"
#include <stdlib.h>

extern int64_t GameScreen_time;
extern void GameScreen_loadLevel(GameScreen *gs, int lvl, Vector3D *pos, bool changeMus,
                                  bool fullMove, bool showLoadScreen);
extern void GameScreen_setNewPos(GameScreen *gs, Vector3D *pos);
extern void Player_rotYn(Player *player, int rot);

extern bool RoomObject_isAllCollected(RoomObject *self, Player *player, House *house, GameScreen *gs);
extern void RoomObject_errMsg(RoomObject *self, GameScreen *gs);
extern void RoomObject_prMsg(RoomObject *self, GameScreen *gs);
extern void RoomObject_give_script(char **script, int count, Player *player, House *house, GameScreen *gs);
extern void RoomObject_give_name(const char *name, Player *player);
extern bool RoomObject_contains(const char **vec, int count, const char *name);
extern void House_removeObject(House *house, RoomObject *obj);
extern void Asset_playSoundIfEnabled(const char *sound);

LVLChange *LVLChange_new(int x, int y, int z, Vector3D *newPos, int level) {
    LVLChange *self = (LVLChange *)calloc(1, sizeof(LVLChange));
    Vector3D_set(&self->pos, x, y, z);
    Vector3D_set(&self->np, newPos->x, newPos->y, newPos->z);
    self->lvl = level;
    self->pRot = 0;
    self->saveMus = false;
    self->fullMove = false;
    self->showLoadScreen = true;
    return self;
}

void LVLChange_destroy(LVLChange *self) {
    if (!self) return;
}

int LVLChange_getPosX(LVLChange *self) { return self->pos.x; }
int LVLChange_getPosY(LVLChange *self) { return self->pos.y; }
int LVLChange_getPosZ(LVLChange *self) { return self->pos.z; }

void LVLChange_setPos(LVLChange *self, int x, int y, int z) {
    Vector3D_set(&self->pos, x, y, z);
}

void LVLChange_activate(LVLChange *self, House *house, Player *player, GameScreen *gs) {
    RoomObject *base = &self->base;

    if (base->lastActivate >= 0 && (base->lastActivate + base->timeToReset > GameScreen_time)) return;

    if (!RoomObject_isAllCollected(base, player, house, gs)) {
        if (base->errMessage != NULL) RoomObject_errMsg(base, gs);
        return;
    }

    if (base->sound != NULL) Asset_playSoundIfEnabled(base->sound);

    if (!base->singleUse || !base->activated) {
        if (base->message != NULL) RoomObject_prMsg(base, gs);
    }

    if (base->destroyOnUse) House_removeObject(house, (RoomObject *)self);

    if (!base->singleUse || !base->activated) {
        RoomObject_give_script(base->additional, base->additionalCount, player, house, gs);
    }

    if (!RoomObject_contains((const char **)base->need, base->needCount, base->name)) {
        RoomObject_give_name(base->name, player);
        base->activated = true;
        if (base->timeToReset > 0) base->lastActivate = GameScreen_time;

        Vector3D newPos;
        Vector3D_set(&newPos, self->np.x, self->np.y, self->np.z);
        GameScreen_setNewPos(gs, &newPos);
        Player_rotYn(player, self->pRot);
        GameScreen_loadLevel(gs, self->lvl, &self->pos, !self->saveMus,
                             self->fullMove, self->showLoadScreen);
    }
}
