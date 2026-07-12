/*
 * key_object.h — porte fiel de code/Gameplay/Objects/KeyObject.java (Quantum Engine J2ME)
 * @author Roman Lahin
 */
#ifndef QE_GAMEPLAY_OBJECTS_KEY_OBJECT_H
#define QE_GAMEPLAY_OBJECTS_KEY_OBJECT_H

#include "../map/room_object.h"
#include "../../math/vector3d.h"

typedef struct KeyObject {
    RoomObject base;  /* heranca: RoomObject como primeiro campo */
    Vector3D   pos;
} KeyObject;

KeyObject *KeyObject_new(void);
void       KeyObject_free(KeyObject *self);
void       KeyObject_activate(KeyObject *self, Player *player, House *house, GameScreen *gs);

#endif
