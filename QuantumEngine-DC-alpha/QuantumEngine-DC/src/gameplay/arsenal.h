/*
 * arsenal.h — porte fiel de code/Gameplay/Arsenal.java
 */
#ifndef QE_GAMEPLAY_ARSENAL_H
#define QE_GAMEPLAY_ARSENAL_H

#include <stdbool.h>

typedef struct Weapon   Weapon;
typedef struct Graphics Graphics;
typedef struct GameScreen GameScreen;

typedef struct Arsenal {
    Weapon **weapons;
    int      weaponCount;
    int      current;
    bool     hasHand;
} Arsenal;

void    Arsenal_init(Arsenal *a, int count);
void    Arsenal_destroy(Arsenal *a);
Weapon *Arsenal_currentWeapon(Arsenal *a);
void    Arsenal_next(Arsenal *a);
void    Arsenal_previous(Arsenal *a);
void    Arsenal_drawWeapon(Arsenal *a, Graphics *g, int screenY, int w, int h, GameScreen *gs);
void    Arsenal_drawReloadAnimation(Arsenal *a, Graphics *g, int screenY, int w, int h);

#endif
