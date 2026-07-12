/*
 * shop_object.c — porte fiel de code/Gameplay/Objects/ShopObject.java
 */
#include "shop_object.h"
#include <stdlib.h>
#include <string.h>

extern int64_t GameScreen_time;
extern void GameScreen_openShop(GameScreen *gs, int *items, char **files, int *prices, int count);

extern bool RoomObject_isAllCollected(RoomObject *self, Player *player, House *house, GameScreen *gs);
extern void RoomObject_errMsg(RoomObject *self, GameScreen *gs);
extern void RoomObject_prMsg(RoomObject *self, GameScreen *gs);
extern void RoomObject_give_script(char **script, int count, Player *player, House *house, GameScreen *gs);
extern bool RoomObject_contains(const char **vec, int count, const char *name);
extern void House_removeObject(House *house, RoomObject *obj);
extern void Asset_playSoundIfEnabled(const char *sound);

ShopObject *ShopObject_new(int *items, int *prices, char **files, int count,
                           int x, int y, int z) {
    ShopObject *self = (ShopObject *)calloc(1, sizeof(ShopObject));
    Vector3D_set(&self->pos, x, y, z);
    self->items = items;
    self->prices = prices;
    self->files = files;
    self->itemCount = count;
    self->base.activable = true;
    self->base.clickable = true;
    return self;
}

void ShopObject_destroy(ShopObject *self) {
    if (!self) return;
    free(self->items);
    free(self->prices);
    if (self->files) {
        for (int i = 0; i < self->itemCount; i++) free(self->files[i]);
        free(self->files);
    }
}

int ShopObject_getPosX(ShopObject *self) { return self->pos.x; }
int ShopObject_getPosY(ShopObject *self) { return self->pos.y; }
int ShopObject_getPosZ(ShopObject *self) { return self->pos.z; }

void ShopObject_setPos(ShopObject *self, int x, int y, int z) {
    Vector3D_set(&self->pos, x, y, z);
}

void ShopObject_activate(ShopObject *self, House *house, Player *player, GameScreen *gs) {
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
        base->activated = true;
        if (base->timeToReset > 0) base->lastActivate = GameScreen_time;
        GameScreen_openShop(gs, self->items, self->files, self->prices, self->itemCount);
    }

    if (base->destroyOnUse) House_removeObject(house, (RoomObject *)self);
}
