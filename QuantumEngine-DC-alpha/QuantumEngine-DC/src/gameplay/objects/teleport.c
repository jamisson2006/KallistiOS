/*
 * teleport.c — porte fiel de code/Gameplay/Objects/Teleport.java (Quantum Engine J2ME)
 * @author Roman Lahin
 *
 * Teleport estende RoomObject. Ao ativar, teleporta o jogador para npos com rotacao pRot.
 */
#include "teleport.h"
#include "../map/room_object.h"
#include "../../math/vector3d.h"
#include <stdlib.h>
#include <string.h>

/* Dependencias externas */
extern void Asset_playSound(const char *file);
extern void GameScreen_activateDialog(GameScreen *gs, const char *msg, int64_t timeout, int type);
extern void Player_addUsedPoint(Player *player, const char *name);
extern void Player_setPos(Player *player, int x, int y, int z);
extern void Player_setRotation(Player *player, int rot);

Teleport *Teleport_new(void) {
    Teleport *self = (Teleport *)calloc(1, sizeof(Teleport));
    self->base.activable = true;
    self->base.clickable = true;
    self->base.visible = true;
    self->base.pointRadius = 20;
    self->pRot = 0;
    Vector3D_set(&self->pos, 0, 0, 0);
    Vector3D_set(&self->npos, 0, 0, 0);
    return self;
}

void Teleport_free(Teleport *self) {
    if (!self) return;
    if (self->base.name) free(self->base.name);
    if (self->base.desc) free(self->base.desc);
    if (self->base.sound) free(self->base.sound);
    if (self->base.message) free(self->base.message);
    if (self->base.errMessage) free(self->base.errMessage);
    if (self->base.need) {
        for (int i = 0; i < self->base.needCount; i++) free(self->base.need[i]);
        free(self->base.need);
    }
    if (self->base.additional) {
        for (int i = 0; i < self->base.additionalCount; i++) free(self->base.additional[i]);
        free(self->base.additional);
    }
    free(self);
}

void Teleport_activate(Teleport *self, Player *player, House *house, GameScreen *gs) {
    RoomObject *base = &self->base;

    if (!base->activable) return;

    /* Verifica condicoes */
    if (!RoomObject_isAllCollected(base, player, house, gs)) {
        if (base->errMessage && strlen(base->errMessage) > 0) {
            GameScreen_activateDialog(gs, base->errMessage, base->errMessageTimeOut, base->errMessageType);
        }
        return;
    }

    /* Toca som */
    if (base->sound && strlen(base->sound) > 0) {
        Asset_playSound(base->sound);
    }

    /* Mostra mensagem */
    if (base->message && strlen(base->message) > 0) {
        GameScreen_activateDialog(gs, base->message, base->messageTimeOut, base->messageType);
    }

    /* Teleporta o jogador */
    Player_setPos(player, self->npos.x, self->npos.y, self->npos.z);
    Player_setRotation(player, self->pRot);

    /* Executa script */
    if (base->additional && base->additionalCount > 0) {
        RoomObject_give(base->additional, base->additionalCount, player, house, gs);
    }

    /* singleUse */
    if (base->singleUse) {
        base->activable = false;
        if (base->name) {
            Player_addUsedPoint(player, base->name);
        }
    }

    /* destroyOnUse */
    if (base->destroyOnUse) {
        extern void House_removeObject(House *house, RoomObject *obj);
        House_removeObject(house, base);
    }

    base->activated = true;
}
