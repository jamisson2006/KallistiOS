/*
 * items_bag.h — porte fiel de code/Gameplay/Objects/ItemsBag.java
 */
#ifndef QE_GAMEPLAY_OBJECTS_ITEMS_BAG_H
#define QE_GAMEPLAY_OBJECTS_ITEMS_BAG_H

#include <stdbool.h>
#include "../../math/vector3d.h"

typedef struct RoomObject        RoomObject;
typedef struct Mesh              Mesh;
typedef struct Sprite            Sprite;
typedef struct InventoryItemList InventoryItemList;

typedef struct ItemsBag {
    RoomObject        base;       /* heranca: RoomObject */
    Mesh             *mesh;
    Sprite           *sprite;
    InventoryItemList *items;
    Vector3D          pos;
} ItemsBag;

void ItemsBag_init(ItemsBag *self);
void ItemsBag_initWithMesh(ItemsBag *self, Mesh *mesh, InventoryItemList *items);
void ItemsBag_initWithSprite(ItemsBag *self, Sprite *sprite, InventoryItemList *items);
void ItemsBag_destroy(ItemsBag *self);

void ItemsBag_activate(ItemsBag *self);
void ItemsBag_render(ItemsBag *self);
int  ItemsBag_getPart(ItemsBag *self);

#endif
