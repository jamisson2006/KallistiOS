/*
 * shop.h — porte fiel de code/Gameplay/Shop.java
 */
#ifndef QE_GAMEPLAY_SHOP_H
#define QE_GAMEPLAY_SHOP_H

#include <stdbool.h>
#include <stdint.h>

typedef struct Player     Player;
typedef struct GameScreen GameScreen;
typedef struct Graphics   Graphics;

typedef struct Shop {
    GameScreen *gameScreen;
    Player     *player;

    int    *items;
    char  **files;
    int    *prices;
    int     itemCount;
    int     index;

    int64_t sideAnimBegin;
    int     sideAnimOld;
} Shop;

/* Static data */
extern int    Shop_weaponCount;
extern char **Shop_defaultFiles;
extern int   *Shop_defaultPrices;
extern int   *Shop_defaultItems;
extern int   *Shop_defaultArsenal;
extern int   *Shop_defaultArsenalAmmo;
extern int    Shop_defaultArsenalCount;
extern int    Shop_defaultArsenalAmmoCount;
extern bool   Shop_proportionalShop;

void Shop_initShop(void);

Shop *Shop_new(GameScreen *gs, Player *player, int *items, char **files, int *prices, int count);
void  Shop_destroy(Shop *self);
void  Shop_paint(Shop *self, Graphics *g);
void  Shop_onLeftSoftKey(Shop *self);
void  Shop_onRightSoftKey(Shop *self);
void  Shop_onKey4(Shop *self);
void  Shop_onKey6(Shop *self);
void  Shop_onKey2(Shop *self);
void  Shop_onKey8(Shop *self);

#endif
