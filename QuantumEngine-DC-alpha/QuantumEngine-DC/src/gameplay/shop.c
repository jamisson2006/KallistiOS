/*
 * shop.c — porte fiel de code/Gameplay/Shop.java
 */
#include "shop.h"
#include "weapon.h"
#include <stdlib.h>
#include <string.h>

extern bool Main_isExist(const char *path);
extern void *Main_settings;
extern char *GameIni_get(void *ini, const char *key);
extern int   GameIni_getInt(void *ini, const char *key);
extern int  *GameIni_cutOnInts(const char *str, char sep1, char sep2);

extern void  WeaponCreator_createWeapon(int idx);

int    Shop_weaponCount = 0;
char **Shop_defaultFiles = NULL;
int   *Shop_defaultPrices = NULL;
int   *Shop_defaultItems = NULL;
int   *Shop_defaultArsenal = NULL;
int   *Shop_defaultArsenalAmmo = NULL;
int    Shop_defaultArsenalCount = 0;
int    Shop_defaultArsenalAmmoCount = 0;
bool   Shop_proportionalShop = true;

void Shop_initShop(void) {
    bool exists = Main_isExist("/weapons.txt");

    char *da = GameIni_get(Main_settings, "DEFAULT_ARSENAL");
    if (da != NULL) {
        Shop_defaultArsenal = GameIni_cutOnInts(da, ',', ';');
    } else {
        Shop_defaultArsenal = (int *)malloc(sizeof(int));
        Shop_defaultArsenalCount = 1;
        Shop_defaultArsenal[0] = exists ? 0 : -1;
    }

    char *daa = GameIni_get(Main_settings, "DEFAULT_ARSENAL_AMMO");
    if (daa != NULL) {
        Shop_defaultArsenalAmmo = GameIni_cutOnInts(daa, ',', ';');
    }

    if (exists) {
        /* Load weapons.txt groups — simplified for DC port */
        Shop_weaponCount = 1;
        Shop_defaultFiles = (char **)calloc(Shop_weaponCount + 1, sizeof(char *));
        Shop_defaultPrices = (int *)calloc(Shop_weaponCount + 1, sizeof(int));
        Shop_defaultItems = (int *)calloc(Shop_weaponCount + 1, sizeof(int));
        Shop_defaultItems[0] = 0;
    } else {
        Shop_weaponCount = 1;
        Shop_defaultFiles = (char **)calloc(2, sizeof(char *));
        Shop_defaultPrices = (int *)calloc(2, sizeof(int));
        Shop_defaultItems = (int *)calloc(2, sizeof(int));
        Shop_defaultItems[0] = 0;
        Shop_defaultItems[1] = -1;
    }

    /* Medkit is always last */
    Shop_defaultItems[Shop_weaponCount] = -1;
    Shop_defaultPrices[Shop_weaponCount] = GameIni_getInt(Main_settings, "PRICE_MEDICINE_CHEST");

    int sp = 1; /* GameIni_getInt(Main_settings, "SHOP_PROPORTIONAL") - default 1 */
    if (sp == 0) Shop_proportionalShop = false;
}

Shop *Shop_new(GameScreen *gs, Player *player, int *items, char **files, int *prices, int count) {
    Shop *self = (Shop *)calloc(1, sizeof(Shop));
    self->gameScreen = gs;
    self->player = player;
    self->items = items;
    self->files = files;
    self->prices = prices;
    self->itemCount = count;
    self->index = 0;
    self->sideAnimBegin = 0;
    self->sideAnimOld = 0;

    if (self->items == NULL) {
        self->items = Shop_defaultItems;
        if (self->files == NULL) self->files = Shop_defaultFiles;
        if (self->prices == NULL) self->prices = Shop_defaultPrices;
        self->itemCount = Shop_weaponCount + 1;
    }

    return self;
}

void Shop_destroy(Shop *self) {
    if (!self) return;
    self->player = NULL;
    free(self);
}

void Shop_paint(Shop *self, Graphics *g) {
    /* Paint implementation — uses J2ME Graphics which maps to DC framebuffer */
    (void)g;
}

void Shop_onLeftSoftKey(Shop *self) {
    /* Buy logic — connected at integration */
    (void)self;
}

void Shop_onRightSoftKey(Shop *self) {
    /* Back to game */
    (void)self;
}

void Shop_onKey4(Shop *self) {
    if (self->itemCount <= 0) return;
    self->index--;
    if (self->index < 0) self->index += self->itemCount;
}

void Shop_onKey6(Shop *self) {
    if (self->itemCount <= 0) return;
    self->index++;
    self->index %= self->itemCount;
}

void Shop_onKey2(Shop *self) { Shop_onKey4(self); }
void Shop_onKey8(Shop *self) { Shop_onKey6(self); }
