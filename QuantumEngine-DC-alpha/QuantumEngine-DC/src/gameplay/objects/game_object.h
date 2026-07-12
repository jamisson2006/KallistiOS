/*
 * game_object.h — porte fiel de code/Gameplay/Objects/GameObject.java
 */
#ifndef QE_GAMEPLAY_OBJECTS_GAME_OBJECT_H
#define QE_GAMEPLAY_OBJECTS_GAME_OBJECT_H

#include <stdbool.h>
#include <stdint.h>

typedef struct Character Character;
typedef struct RoomObject RoomObject;

typedef struct GameObject {
    RoomObject  base;       /* heranca: RoomObject */
    Character  *character;
    int         hp;
    int         frame;
    float       friction;
    long        DeathTime;
} GameObject;

void       GameObject_init(GameObject *self, Character *character, int hp);
void       GameObject_destroy(GameObject *self);

void       GameObject_update(GameObject *self);
void       GameObject_damage(GameObject *self, int dmg);
bool       GameObject_isDead(GameObject *self);
bool       GameObject_isTimeToRenew(GameObject *self);

int        GameObject_getHp(GameObject *self);
void       GameObject_setHp(GameObject *self, int hp);
float      GameObject_getFriction(GameObject *self);
void       GameObject_setFriction(GameObject *self, float friction);
Character *GameObject_getCharacter(GameObject *self);
int        GameObject_getPart(GameObject *self);

#endif
