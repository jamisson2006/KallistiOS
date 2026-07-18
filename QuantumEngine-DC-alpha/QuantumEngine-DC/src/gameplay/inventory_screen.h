/*
 * inventory_screen.h — porte fiel de code/Gameplay/InventoryScreen.java
 */
#ifndef QE_GAMEPLAY_INVENTORY_SCREEN_H
#define QE_GAMEPLAY_INVENTORY_SCREEN_H

#include <stdbool.h>
#include <stdint.h>

typedef struct Player            Player;
typedef struct GameScreen        GameScreen;
typedef struct InventoryItemList InventoryItemList;
typedef struct Graphics          Graphics;
typedef struct Image             Image;

typedef struct InventoryScreen {
    GameScreen        *gameScreen;
    Player            *player;
    InventoryItemList *items;

    int     index;
    char   *itemName;
    char   *itemScript;
    char   *itemPath;
    bool    itemCanThrow;
    int     itemCount;
    Image  *iconItem;

    int64_t sideAnimBegin;
    int     sideAnimOld;
} InventoryScreen;

extern bool InventoryScreen_proportionalInventory;

InventoryScreen *InventoryScreen_new(GameScreen *gs, Player *player);
void             InventoryScreen_destroy(InventoryScreen *self);
void             InventoryScreen_paint(InventoryScreen *self, Graphics *g);
void             InventoryScreen_onLeftSoftKey(InventoryScreen *self);
void             InventoryScreen_onRightSoftKey(InventoryScreen *self);
void             InventoryScreen_onKey4(InventoryScreen *self);
void             InventoryScreen_onKey6(InventoryScreen *self);
void             InventoryScreen_onKey2(InventoryScreen *self);
void             InventoryScreen_onKey8(InventoryScreen *self);
void             InventoryScreen_onKey5(InventoryScreen *self);

#endif
