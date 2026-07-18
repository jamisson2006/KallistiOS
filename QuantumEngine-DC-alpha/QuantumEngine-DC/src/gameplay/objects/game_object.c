/*
 * game_object.c — porte fiel de code/Gameplay/Objects/GameObject.java
 */
#include "game_object.h"
#include <stdlib.h>
#include <string.h>

/* Dependencias externas */
extern void Character_applyFriction(Character *ch, float friction);
extern int  Character_getPart(Character *ch);
extern long System_currentTimeMillis(void);

void GameObject_init(GameObject *self, Character *character, int hp) {
    memset(self, 0, sizeof(GameObject));
    self->character = character;
    self->hp = hp;
    self->frame = 0;
    self->friction = 0.0f;
    self->DeathTime = 0;
}

void GameObject_destroy(GameObject *self) {
    if (!self) return;
    /* character ownership is external */
}

void GameObject_update(GameObject *self) {
    if (self->friction != 0.0f && self->character != NULL) {
        Character_applyFriction(self->character, self->friction);
    }
    self->frame++;
}

void GameObject_damage(GameObject *self, int dmg) {
    self->hp -= dmg;
    if (self->hp <= 0) {
        self->hp = 0;
        self->DeathTime = System_currentTimeMillis();
    }
}

bool GameObject_isDead(GameObject *self) {
    return self->hp <= 0;
}

bool GameObject_isTimeToRenew(GameObject *self) {
    if (self->DeathTime == 0) return false;
    long now = System_currentTimeMillis();
    return (now - self->DeathTime) > 5000;
}

int GameObject_getHp(GameObject *self) {
    return self->hp;
}

void GameObject_setHp(GameObject *self, int hp) {
    self->hp = hp;
}

float GameObject_getFriction(GameObject *self) {
    return self->friction;
}

void GameObject_setFriction(GameObject *self, float friction) {
    self->friction = friction;
}

Character *GameObject_getCharacter(GameObject *self) {
    return self->character;
}

int GameObject_getPart(GameObject *self) {
    return Character_getPart(self->character);
}

void GameObject_setCharacterSize(GameObject *self, int height) {
    if (self->character) {
        self->character->player_height = height;
    }
}

int GameObject_getFrameInter(GameObject *self) {
    return self->frame;
}

int GameObject_getFrameInterDiv(GameObject *self) {
    return self->frame;
}
