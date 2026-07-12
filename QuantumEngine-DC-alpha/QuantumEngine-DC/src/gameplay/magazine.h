/*
 * magazine.h — porte fiel de code/Gameplay/Magazine.java
 */
#ifndef QE_GAMEPLAY_MAGAZINE_H
#define QE_GAMEPLAY_MAGAZINE_H

#include <stdbool.h>

typedef struct Magazine {
    int  capacity;
    int  ammo;
    int  rounds;
    int  reloadTime;
    int  frame;
    bool reloading;
} Magazine;

void Magazine_init(Magazine *m, int capacity, int ammo, int reloadTime);
void Magazine_update(Magazine *m, int frameTime);
bool Magazine_shoot(Magazine *m);
void Magazine_reload(Magazine *m);
void Magazine_addAmmo(Magazine *m, int amount);
int  Magazine_getRounds(Magazine *m);
int  Magazine_getAmmo(Magazine *m);
bool Magazine_isReloading(Magazine *m);

#endif
