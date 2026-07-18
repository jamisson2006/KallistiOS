/*
 * sprite_object.h — porte fiel de code/Gameplay/Objects/SpriteObject.java
 */
#ifndef QE_GAMEPLAY_OBJECTS_SPRITE_OBJECT_H
#define QE_GAMEPLAY_OBJECTS_SPRITE_OBJECT_H

#include <stdbool.h>

typedef struct Sprite      Sprite;
typedef struct RoomObject  RoomObject;

typedef struct SpriteObject {
    RoomObject  base;       /* heranca: RoomObject como primeiro campo */
    Sprite     *spr;
    int         addsz;
    bool       *flicker;
    int         flickerCount;
    int        *colorFlicker;
    int         colorFlickerCount;
    int         intframe;
} SpriteObject;

void SpriteObject_init(SpriteObject *self, Sprite *spr);
void SpriteObject_destroy(SpriteObject *self);

int  SpriteObject_getPart(SpriteObject *self);
void SpriteObject_render(SpriteObject *self);
void SpriteObject_update(SpriteObject *self);

#endif
