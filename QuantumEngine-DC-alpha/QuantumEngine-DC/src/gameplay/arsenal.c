/*
 * arsenal.c — porte fiel de code/Gameplay/Arsenal.java
 */
#include "arsenal.h"
#include "weapon.h"
#include <stdlib.h>
#include <string.h>

void Arsenal_init(Arsenal *a, int count) {
    a->weaponCount = count;
    a->weapons = (Weapon **)calloc(count, sizeof(Weapon *));
    a->current = -1;
    a->hasHand = false;
}

void Arsenal_destroy(Arsenal *a) {
    if (!a) return;
    if (a->weapons) {
        for (int i = 0; i < a->weaponCount; i++) {
            if (a->weapons[i]) Weapon_free(a->weapons[i]);
        }
        free(a->weapons);
        a->weapons = NULL;
    }
}

Weapon *Arsenal_currentWeapon(Arsenal *a) {
    if (a->current < 0 || a->current >= a->weaponCount) return NULL;
    return a->weapons[a->current];
}

void Arsenal_next(Arsenal *a) {
    if (a->weaponCount <= 0) return;
    int start = a->current;
    do {
        a->current++;
        if (a->current >= a->weaponCount) a->current = 0;
        if (a->weapons[a->current] != NULL) return;
    } while (a->current != start);
}

void Arsenal_previous(Arsenal *a) {
    if (a->weaponCount <= 0) return;
    int start = a->current;
    do {
        a->current--;
        if (a->current < 0) a->current = a->weaponCount - 1;
        if (a->weapons[a->current] != NULL) return;
    } while (a->current != start);
}

void Arsenal_drawWeapon(Arsenal *a, Graphics *g, int screenY, int w, int h, GameScreen *gs) {
    Weapon *wp = Arsenal_currentWeapon(a);
    if (wp) Weapon_drawWeapon(wp, g, screenY, w, h, gs);
}

void Arsenal_drawReloadAnimation(Arsenal *a, Graphics *g, int screenY, int w, int h) {
    Weapon *wp = Arsenal_currentWeapon(a);
    if (wp) Weapon_drawReloadAnimation(wp, g, screenY, w, h);
}
