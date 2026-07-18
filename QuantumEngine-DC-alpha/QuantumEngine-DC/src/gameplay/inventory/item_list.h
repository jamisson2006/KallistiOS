/*
 * item_list.h — porte fiel de code/Gameplay/Inventory/ItemList.java
 */
#ifndef QE_GAMEPLAY_INVENTORY_ITEM_LIST_H
#define QE_GAMEPLAY_INVENTORY_ITEM_LIST_H

#include <stdbool.h>

typedef struct InventoryItemList {
    int *ids;
    int *counts;
    int  size;
    int  capacity;
} InventoryItemList;

InventoryItemList *InventoryItemList_new(void);
void  InventoryItemList_free(InventoryItemList *l);
void  InventoryItemList_addItem(InventoryItemList *l, const char *name, int count);
void  InventoryItemList_addItemById(InventoryItemList *l, int id, int count);
bool  InventoryItemList_removeItem(InventoryItemList *l, int id, int count);
int   InventoryItemList_getCount(InventoryItemList *l, int id);
int   InventoryItemList_getSize(InventoryItemList *l);
int   InventoryItemList_getId(InventoryItemList *l, int index);
int   InventoryItemList_getCountAt(InventoryItemList *l, int index);
void  InventoryItemList_save(InventoryItemList *l, void *dos);
void  InventoryItemList_load(InventoryItemList *l, void *dis);

#endif
