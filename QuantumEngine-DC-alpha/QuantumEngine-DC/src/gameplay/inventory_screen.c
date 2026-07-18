/*
 * inventory_screen.c — porte fiel de code/Gameplay/InventoryScreen.java
 */
#include "inventory_screen.h"
#include "inventory/item_list.h"
#include "inventory/items_engine.h"
#include <stdlib.h>
#include <string.h>

extern void GameScreen_start(GameScreen *gs);
extern void GameScreen_runScript(GameScreen *gs, char **lines, int count);
extern char **StringTools_cutOnStrings(const char *str, char sep, int *outCount);
extern int    Player_getHp(Player *player);
extern int    Player_getMoney(Player *player);

extern void Scene_dropItem(void *scene, const char *name, int count, Player *player);

bool InventoryScreen_proportionalInventory = true;

static void loadItem(InventoryScreen *self) {
    if (InventoryItemList_getSize(self->items) <= 0) {
        self->itemName = NULL;
        self->itemScript = NULL;
        self->itemPath = NULL;
        self->iconItem = NULL;
        return;
    }

    int itemId = InventoryItemList_getId(self->items, self->index);
    self->itemCount = InventoryItemList_getCountAt(self->items, self->index);

    /* Item info from ItemsEngine */
    extern void *ItemsEngine_items[];
    extern char *IniFile_get(void *ini, const char *key);
    extern int   IniFile_getInt(void *ini, const char *key, int def);

    void *item = ItemsEngine_items[itemId];
    char *path = IniFile_get(item, "ICON");

    self->itemName = IniFile_get(item, "NAME");
    self->itemScript = IniFile_get(item, "ON_ACTIVATE");
    self->itemCanThrow = IniFile_getInt(item, "CAN_THROW", 1) == 1;

    self->itemPath = path;
    /* Image loading done at integration time */
}

InventoryScreen *InventoryScreen_new(GameScreen *gs, Player *player) {
    InventoryScreen *self = (InventoryScreen *)calloc(1, sizeof(InventoryScreen));
    self->gameScreen = gs;
    self->player = player;
    self->index = 0;
    self->sideAnimBegin = 0;

    /* Get player items list */
    extern InventoryItemList *Player_getItems(Player *player);
    self->items = Player_getItems(player);

    loadItem(self);
    return self;
}

void InventoryScreen_destroy(InventoryScreen *self) {
    if (!self) return;
    self->iconItem = NULL;
    self->itemName = NULL;
    self->itemScript = NULL;
    self->itemPath = NULL;
    self->items = NULL;
    self->player = NULL;
    free(self);
}

void InventoryScreen_paint(InventoryScreen *self, Graphics *g) {
    /* Paint — connected at integration with DC framebuffer */
    (void)g;
    (void)self;
}

void InventoryScreen_onRightSoftKey(InventoryScreen *self) {
    GameScreen_start(self->gameScreen);
    self->gameScreen = NULL;
}

void InventoryScreen_onLeftSoftKey(InventoryScreen *self) {
    if (InventoryItemList_getSize(self->items) <= 0 || !self->itemCanThrow) return;

    int oldCount = InventoryItemList_getCountAt(self->items, self->index);
    int itemId = InventoryItemList_getId(self->items, self->index);

    extern void *ItemsEngine_items[];
    extern char *IniFile_get(void *ini, const char *key);

    void *item = ItemsEngine_items[itemId];
    const char *itemName = IniFile_get(item, "NAME");

    extern void *GameScreen_getScene(GameScreen *gs);
    Scene_dropItem(GameScreen_getScene(self->gameScreen), itemName, 1, self->player);
    InventoryItemList_removeItem(self->items, itemId, 1);

    char *onThrow = IniFile_get(item, "ON_THROW");
    if (onThrow != NULL) {
        int count = 0;
        char **lines = StringTools_cutOnStrings(onThrow, ';', &count);
        GameScreen_runScript(self->gameScreen, lines, count);
        free(lines);
    }

    if (oldCount == 1) {
        if (self->index >= InventoryItemList_getSize(self->items)) {
            self->index = InventoryItemList_getSize(self->items) - 1;
        }
        loadItem(self);
    } else {
        self->itemCount--;
    }
}

void InventoryScreen_onKey5(InventoryScreen *self) {
    if (InventoryItemList_getSize(self->items) <= 0 || self->itemScript == NULL) return;

    int count = 0;
    char **lines = StringTools_cutOnStrings(self->itemScript, ';', &count);
    GameScreen_runScript(self->gameScreen, lines, count);
    free(lines);

    if (self->index >= InventoryItemList_getSize(self->items)) {
        self->index = InventoryItemList_getSize(self->items) - 1;
    }
    loadItem(self);
}

void InventoryScreen_onKey6(InventoryScreen *self) {
    int sz = InventoryItemList_getSize(self->items);
    if (sz <= 0) return;
    self->index++;
    self->index %= sz;
    loadItem(self);
}

void InventoryScreen_onKey4(InventoryScreen *self) {
    int sz = InventoryItemList_getSize(self->items);
    if (sz <= 0) return;
    self->index--;
    if (self->index < 0) self->index += sz;
    loadItem(self);
}

void InventoryScreen_onKey8(InventoryScreen *self) { InventoryScreen_onKey6(self); }
void InventoryScreen_onKey2(InventoryScreen *self) { InventoryScreen_onKey4(self); }
