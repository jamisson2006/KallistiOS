/*
 * key_object.c — porte fiel de code/Gameplay/Objects/KeyObject.java (Quantum Engine J2ME)
 * @author Roman Lahin
 *
 * KeyObject estende RoomObject. Ao ativar, verifica isAllCollected,
 * toca som, mostra mensagem e executa give(additional).
 */
#include "key_object.h"
#include "../map/room_object.h"
#include "../../math/vector3d.h"
#include <stdlib.h>
#include <string.h>

/* Dependencias externas */
extern void Asset_playSound(const char *file);
extern void GameScreen_activateDialog(GameScreen *gs, const char *msg, int64_t timeout, int type);
extern void Player_addUsedPoint(Player *player, const char *name);

KeyObject *KeyObject_new(void) {
    KeyObject *self = (KeyObject *)calloc(1, sizeof(KeyObject));
    /* defaults da base */
    self->base.activable = true;
    self->base.clickable = true;
    self->base.visible = true;
    self->base.pointRadius = 20;
    Vector3D_set(&self->pos, 0, 0, 0);
    return self;
}

void KeyObject_free(KeyObject *self) {
    if (!self) return;
    /* Libera campos de string da base */
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

void KeyObject_activate(KeyObject *self, Player *player, House *house, GameScreen *gs) {
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

    /* Executa script */
    if (base->additional && base->additionalCount > 0) {
        RoomObject_give(base->additional, base->additionalCount, player, house, gs);
    }

    /* Marca como usado (singleUse) */
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
