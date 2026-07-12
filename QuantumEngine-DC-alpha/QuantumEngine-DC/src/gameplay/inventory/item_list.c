/*
 * item_list.c — porte fiel de code/Gameplay/Inventory/ItemList.java
 */
#include "item_list.h"
#include "items_engine.h"
#include "../../utils/data_stream.h"
#include <stdlib.h>
#include <string.h>

#define GROW_SIZE 64

InventoryItemList *InventoryItemList_new(void) {
    InventoryItemList *l = (InventoryItemList *)calloc(1, sizeof(InventoryItemList));
    return l;
}

void InventoryItemList_free(InventoryItemList *l) {
    if (!l) return;
    free(l->ids);
    free(l->counts);
    free(l);
}

static void ensureCapacity(InventoryItemList *l, int needed) {
    if (needed <= l->capacity) return;
    int newCap = l->capacity + GROW_SIZE;
    if (newCap < needed) newCap = needed;
    l->ids = (int *)realloc(l->ids, newCap * sizeof(int));
    l->counts = (int *)realloc(l->counts, newCap * sizeof(int));
    l->capacity = newCap;
}

void InventoryItemList_addItemById(InventoryItemList *l, int id, int count) {
    if (id < 0 || count <= 0) return;
    for (int i = 0; i < l->size; i++) {
        if (l->ids[i] == id) {
            l->counts[i] += count;
            return;
        }
    }
    ensureCapacity(l, l->size + 1);
    l->ids[l->size] = id;
    l->counts[l->size] = count;
    l->size++;
}

void InventoryItemList_addItem(InventoryItemList *l, const char *name, int count) {
    int id = ItemsEngine_getItemId(name);
    InventoryItemList_addItemById(l, id, count);
}

bool InventoryItemList_removeItem(InventoryItemList *l, int id, int count) {
    for (int i = 0; i < l->size; i++) {
        if (l->ids[i] == id) {
            l->counts[i] -= count;
            if (l->counts[i] <= 0) {
                l->size--;
                l->ids[i] = l->ids[l->size];
                l->counts[i] = l->counts[l->size];
            }
            return true;
        }
    }
    return false;
}

int InventoryItemList_getCount(InventoryItemList *l, int id) {
    for (int i = 0; i < l->size; i++) {
        if (l->ids[i] == id) return l->counts[i];
    }
    return 0;
}

int InventoryItemList_getSize(InventoryItemList *l) { return l->size; }
int InventoryItemList_getId(InventoryItemList *l, int index) { return l->ids[index]; }
int InventoryItemList_getCountAt(InventoryItemList *l, int index) { return l->counts[index]; }

void InventoryItemList_save(InventoryItemList *l, void *dos) {
    DataOutputStream *d = (DataOutputStream *)dos;
    DataOutputStream_writeInt(d, l->size);
    for (int i = 0; i < l->size; i++) {
        DataOutputStream_writeInt(d, l->ids[i]);
        DataOutputStream_writeInt(d, l->counts[i]);
    }
}

void InventoryItemList_load(InventoryItemList *l, void *dis) {
    DataInputStream *d = (DataInputStream *)dis;
    int sz = DataInputStream_readInt(d);
    l->size = 0;
    for (int i = 0; i < sz; i++) {
        int id = DataInputStream_readInt(d);
        int count = DataInputStream_readInt(d);
        InventoryItemList_addItemById(l, id, count);
    }
}
