/*
 * sprite_object.c — porte fiel de code/Gameplay/Objects/SpriteObject.java
 */
#include "sprite_object.h"
#include <stdlib.h>
#include <string.h>

/* Dependencias externas (Sprite, RoomObject) */
extern int  Sprite_getPart(Sprite *spr);
extern void Sprite_render(Sprite *spr);
extern void Sprite_setAddsz(Sprite *spr, int addsz);
extern void Sprite_setFlicker(Sprite *spr, bool *flicker, int count);
extern void Sprite_setColorFlicker(Sprite *spr, int *colorFlicker, int count);
extern void Sprite_setFrame(Sprite *spr, int frame);

void SpriteObject_init(SpriteObject *self, Sprite *spr) {
    memset(self, 0, sizeof(SpriteObject));
    self->spr = spr;
    self->addsz = 0;
    self->flicker = NULL;
    self->flickerCount = 0;
    self->colorFlicker = NULL;
    self->colorFlickerCount = 0;
    self->intframe = 0;
}

void SpriteObject_destroy(SpriteObject *self) {
    if (!self) return;
    if (self->flicker) {
        free(self->flicker);
        self->flicker = NULL;
    }
    if (self->colorFlicker) {
        free(self->colorFlicker);
        self->colorFlicker = NULL;
    }
}

int SpriteObject_getPart(SpriteObject *self) {
    return Sprite_getPart(self->spr);
}

void SpriteObject_render(SpriteObject *self) {
    if (self->spr == NULL) return;
    if (self->addsz != 0) {
        Sprite_setAddsz(self->spr, self->addsz);
    }
    if (self->flicker != NULL) {
        Sprite_setFlicker(self->spr, self->flicker, self->flickerCount);
    }
    if (self->colorFlicker != NULL) {
        Sprite_setColorFlicker(self->spr, self->colorFlicker, self->colorFlickerCount);
    }
    Sprite_setFrame(self->spr, self->intframe);
    Sprite_render(self->spr);
}

void SpriteObject_update(SpriteObject *self) {
    self->intframe++;
}
