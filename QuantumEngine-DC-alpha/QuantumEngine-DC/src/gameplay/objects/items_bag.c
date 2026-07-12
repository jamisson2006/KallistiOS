/*
 * items_bag.c — porte fiel de code/Gameplay/Objects/ItemsBag.java
 */
#include "items_bag.h"
#include <stdlib.h>
#include <string.h>

/* Dependencias externas */
extern void Mesh_render(Mesh *mesh);
extern int  Mesh_getPart(Mesh *mesh);
extern void Sprite_render(Sprite *spr);
extern int  Sprite_getPart(Sprite *spr);

extern void Player_addItem(int id, int count);
extern void Player_runPickupScript(int id);
extern int  InventoryItemList_getSize(InventoryItemList *l);
extern int  InventoryItemList_getId(InventoryItemList *l, int index);
extern int  InventoryItemList_getCountAt(InventoryItemList *l, int index);

void ItemsBag_init(ItemsBag *self) {
    memset(self, 0, sizeof(ItemsBag));
    self->mesh = NULL;
    self->sprite = NULL;
    self->items = NULL;
    Vector3D_set(&self->pos, 0, 0, 0);
}

void ItemsBag_initWithMesh(ItemsBag *self, Mesh *mesh, InventoryItemList *items) {
    memset(self, 0, sizeof(ItemsBag));
    self->mesh = mesh;
    self->sprite = NULL;
    self->items = items;
    Vector3D_set(&self->pos, 0, 0, 0);
}

void ItemsBag_initWithSprite(ItemsBag *self, Sprite *sprite, InventoryItemList *items) {
    memset(self, 0, sizeof(ItemsBag));
    self->mesh = NULL;
    self->sprite = sprite;
    self->items = items;
    Vector3D_set(&self->pos, 0, 0, 0);
}

void ItemsBag_destroy(ItemsBag *self) {
    if (!self) return;
    /* items ownership is external; do not free here */
}

void ItemsBag_activate(ItemsBag *self) {
    if (self->items == NULL) return;
    int size = InventoryItemList_getSize(self->items);
    for (int i = 0; i < size; i++) {
        int id = InventoryItemList_getId(self->items, i);
        int count = InventoryItemList_getCountAt(self->items, i);
        Player_addItem(id, count);
        Player_runPickupScript(id);
    }
}

void ItemsBag_render(ItemsBag *self) {
    if (self->mesh != NULL) {
        Mesh_render(self->mesh);
    } else if (self->sprite != NULL) {
        Sprite_render(self->sprite);
    }
}

int ItemsBag_getPart(ItemsBag *self) {
    if (self->mesh != NULL) {
        return Mesh_getPart(self->mesh);
    } else if (self->sprite != NULL) {
        return Sprite_getPart(self->sprite);
    }
    return -1;
}
