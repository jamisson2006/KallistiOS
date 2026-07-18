/*
 * items_engine.h — porte fiel de code/Gameplay/Inventory/ItemsEngine.java
 */
#ifndef QE_GAMEPLAY_INVENTORY_ITEMS_ENGINE_H
#define QE_GAMEPLAY_INVENTORY_ITEMS_ENGINE_H

typedef struct IniFile IniFile;

extern IniFile **ItemsEngine_items;
extern int       ItemsEngine_itemCount;

void ItemsEngine_init(void);
int  ItemsEngine_getItemId(const char *name);

#endif
