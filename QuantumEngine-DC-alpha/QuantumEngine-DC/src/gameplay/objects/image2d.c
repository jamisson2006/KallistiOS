/*
 * image2d.c — porte fiel de code/Gameplay/Objects/Image2D.java
 */
#include "image2d.h"
#include <stdlib.h>
#include <string.h>

extern int64_t GameScreen_time;
extern void GameScreen_setOverlay(GameScreen *gs, Image *img, int64_t timeout);

extern bool RoomObject_isAllCollected(RoomObject *self, Player *player, House *house, GameScreen *gs);
extern void RoomObject_errMsg(RoomObject *self, GameScreen *gs);
extern void RoomObject_prMsg(RoomObject *self, GameScreen *gs);
extern void RoomObject_give_script(char **script, int count, Player *player, House *house, GameScreen *gs);
extern void RoomObject_give_name(const char *name, Player *player);
extern bool RoomObject_contains(const char **vec, int count, const char *name);
extern void House_removeObject(House *house, RoomObject *obj);

extern void Asset_playSoundIfEnabled(const char *sound);

Image2D *Image2D_new(Vector3D *pos2, Image *img2, int64_t timer2) {
    Image2D *self = (Image2D *)calloc(1, sizeof(Image2D));
    self->base.activable = true;
    self->base.singleUse = true;
    self->base.destroyOnUse = true;
    self->img = img2;
    self->timer = timer2;
    Vector3D_set(&self->pos, pos2->x, pos2->y, pos2->z);
    return self;
}

void Image2D_destroy(Image2D *self) {
    if (!self) return;
}

int Image2D_getPosX(Image2D *self) { return self->pos.x; }
int Image2D_getPosY(Image2D *self) { return self->pos.y; }
int Image2D_getPosZ(Image2D *self) { return self->pos.z; }

void Image2D_setPos(Image2D *self, int x, int y, int z) {
    Vector3D_set(&self->pos, x, y, z);
}

void Image2D_activate(Image2D *self, House *house, Player *player, GameScreen *gs) {
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

    if (!base->singleUse || !base->activated) {
        RoomObject_give_script(base->additional, base->additionalCount, player, house, gs);
    }

    if (!RoomObject_contains((const char **)base->need, base->needCount, base->name)) {
        RoomObject_give_name(base->name, player);
        GameScreen_setOverlay(gs, self->img, self->timer);
        base->activated = true;
        if (base->timeToReset > 0) base->lastActivate = GameScreen_time;
    }

    if (base->destroyOnUse) House_removeObject(house, (RoomObject *)self);
}
