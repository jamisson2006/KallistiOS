/*
 * weapon.h — porte fiel de code/Gameplay/Weapon.java
 */
#ifndef QE_GAMEPLAY_WEAPON_H
#define QE_GAMEPLAY_WEAPON_H

#include <stdbool.h>
#include <stdint.h>
#include "magazine.h"

typedef struct Sprite   Sprite;
typedef struct Image    Image;
typedef struct Graphics Graphics;
typedef struct DirectX7 DirectX7;
typedef struct Ray      Ray;
typedef struct GameScreen GameScreen;

typedef struct Weapon {
    Magazine  magazine;
    Ray      *ray;

    /* Sprites: normal, fire, sight, mirror variants */
    Sprite  **sprites;
    int       spriteCount;

    /* Patron (ammo) purchase flags */
    bool      patronbuy;
    int       lowPatronAmount;

    /* Zoom */
    bool      hasZoom;
    int       stdFov;
    int       zoomFov;

    /* Animation/behavior */
    bool      twoHands;
    bool      newanim;
    int       shake;
    int64_t   lastFire;
    float     animSpeed;

    /* HUD images */
    Image    *imgPatron;
    Image    *imgPatronLow;
    char     *filePatron;
    char     *filePatronLow;

    /* Extra sizing */
    int       addsz;
} Weapon;

void    Weapon_init(Weapon *w);
void    Weapon_destroy(Weapon *w);

void    Weapon_update(Weapon *w, int frameTime);
void    Weapon_fire(Weapon *w, DirectX7 *g3d);

void    Weapon_drawWeapon(Weapon *w, Graphics *g, int screenY, int width, int height,
                          GameScreen *gs);
void    Weapon_drawReloadAnimation(Weapon *w, Graphics *g, int screenY, int width, int height);

void    Weapon_createTrace(Weapon *w, int x, int y, int z);

int     Weapon_getRounds(Weapon *w);
int     Weapon_getAmmo(Weapon *w);
void    Weapon_reset(Weapon *w);

void    Weapon_addAmmo(Weapon *w, int amount);
void    Weapon_reload(Weapon *w);
bool    Weapon_isReloading(Weapon *w);

void    Weapon_setSprites(Weapon *w, Sprite **sprites, int count);
void    Weapon_setMagazine(Weapon *w, int capacity, int ammo, int reloadTime);
void    Weapon_setRay(Weapon *w, Ray *ray);
void    Weapon_setZoom(Weapon *w, bool hasZoom, int stdFov, int zoomFov);
void    Weapon_setAnim(Weapon *w, float speed, bool newanim);

#endif
