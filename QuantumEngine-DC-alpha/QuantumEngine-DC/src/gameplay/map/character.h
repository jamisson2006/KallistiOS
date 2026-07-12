/*
 * character.h — porte fiel de code/Gameplay/Map/Character.java
 */
#ifndef QE_GAMEPLAY_MAP_CHARACTER_H
#define QE_GAMEPLAY_MAP_CHARACTER_H

#include <stdbool.h>
#include "../../math/vector3d.h"
#include "../../math/matrix.h"

typedef struct House House;

typedef struct Height {
    int floor;
    int ceiling;
} Height;

typedef struct Character {
    Matrix    transform;
    Vector3D  speed;
    Vector3D  speedNoFriction;
    int       player_radius;
    int       player_height;
    Height    height;
    bool      onFloor;
    bool      fly;
    bool      underRoof;
    bool      collision;
    bool      collidable;
    bool      updatable;
    int       part;         /* current room/part index */
} Character;

void Character_init(Character *self, int radius, int height);
void Character_destroy(Character *self);

/* Physics / collision */
void Character_collisionTest(Character *self, House *house);
void Character_collisionTestStatic(Character *body1, Character *body2);

/* Movement (transform-relative) */
void Character_moveZ(Character *self, int amount);
void Character_moveX(Character *self, int amount);
void Character_jump(Character *self, int force);
void Character_rotY(Character *self, int degrees);
void Character_drop(Character *self);

/* Position */
void Character_setPosition(Character *self, int x, int y, int z);
void Character_setPositionV(Character *self, Vector3D *pos);

/* Accessors */
Matrix  *Character_getTransform(Character *self);
Vector3D *Character_getSpeed(Character *self);
int      Character_getRadius(Character *self);
int      Character_getHeight(Character *self);
int      Character_getPart(Character *self);
int      Character_getX(Character *self);
int      Character_getY(Character *self);
int      Character_getZ(Character *self);

bool     Character_isCollider(Character *self);
bool     Character_isOnFloor(Character *self);

void     Character_setCollision(Character *self, bool collision);
void     Character_setCollidable(Character *self, bool collidable);
void     Character_setUpdatable(Character *self, bool updatable);
void     Character_setFly(Character *self, bool fly);

/* Friction (called by GameObject) */
void     Character_applyFriction(Character *self, float friction);

#endif
