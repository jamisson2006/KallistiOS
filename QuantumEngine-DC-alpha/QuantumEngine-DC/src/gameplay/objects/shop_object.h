/*
 * shop_object.h — porte fiel de code/Gameplay/Objects/ShopObject.java
 */
#ifndef QE_GAMEPLAY_OBJECTS_SHOP_OBJECT_H
#define QE_GAMEPLAY_OBJECTS_SHOP_OBJECT_H

#include "../map/room_object.h"
#include "../../math/vector3d.h"

typedef struct ShopObject {
    RoomObject base;
    Vector3D   pos;
    int       *items;
    int       *prices;
    char     **files;
    int        itemCount;
} ShopObject;

ShopObject *ShopObject_new(int *items, int *prices, char **files, int count,
                           int x, int y, int z);
void        ShopObject_destroy(ShopObject *self);
void        ShopObject_activate(ShopObject *self, House *house, Player *player, GameScreen *gs);

int  ShopObject_getPosX(ShopObject *self);
int  ShopObject_getPosY(ShopObject *self);
int  ShopObject_getPosZ(ShopObject *self);
void ShopObject_setPos(ShopObject *self, int x, int y, int z);

#endif
