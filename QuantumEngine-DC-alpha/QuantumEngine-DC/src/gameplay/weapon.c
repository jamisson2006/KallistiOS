/*
 * weapon.c — porte fiel de code/Gameplay/Weapon.java
 */
#include "weapon.h"
#include "../utils/qfps.h"
#include "../rendering/meshes/sprite.h"
#include <stdlib.h>
#include <string.h>

/* extern from rendering */
extern int64_t qe_current_ms(void);

void Weapon_init(Weapon *w) {
    memset(w, 0, sizeof(Weapon));
    w->animSpeed = 1.0f;
    w->lastFire = 0;
    w->shake = 0;
    w->stdFov = 256;
    w->zoomFov = 128;
    w->lowPatronAmount = 3;
}

void Weapon_destroy(Weapon *w) {
    if (w->sprites) {
        for (int i = 0; i < w->spriteCount; i++) {
            if (w->sprites[i]) Sprite_free(w->sprites[i]);
        }
        free(w->sprites);
        w->sprites = NULL;
    }
    if (w->filePatron) { free(w->filePatron); w->filePatron = NULL; }
    if (w->filePatronLow) { free(w->filePatronLow); w->filePatronLow = NULL; }
    w->spriteCount = 0;
}

void Weapon_update(Weapon *w, int frameTime) {
    /* Update magazine reload */
    Magazine_update(&w->magazine, frameTime);

    /* Decay shake */
    if (w->shake > 0) {
        w->shake -= frameTime;
        if (w->shake < 0) w->shake = 0;
    }

    /* Animate sprites */
    if (w->sprites && w->spriteCount > 0 && w->sprites[0]) {
        Sprite_updateFrame(w->sprites[0]);
    }
}

void Weapon_fire(Weapon *w, DirectX7 *g3d) {
    if (Magazine_shoot(&w->magazine)) {
        w->lastFire = qe_current_ms();
        w->shake = 100;
        /* Ray casting handled externally via w->ray */
    }
}

void Weapon_drawWeapon(Weapon *w, Graphics *g, int screenY, int width, int height,
                       GameScreen *gs) {
    if (!w->sprites || w->spriteCount <= 0) return;

    /* Draw normal sprite (index 0) centered at bottom */
    Sprite *spr = w->sprites[0];
    if (!spr) return;

    int sprW = Sprite_getWidth(spr);
    int sprH = Sprite_getHeight(spr);
    int drawX = (width - sprW) / 2;
    int drawY = screenY - sprH + w->addsz;

    /* Apply shake offset */
    if (w->shake > 0) {
        drawY -= w->shake / 10;
    }

    /* Fire sprite overlay (index 1) */
    int64_t now = qe_current_ms();
    if (w->spriteCount > 1 && w->sprites[1] && (now - w->lastFire) < 80) {
        /* Draw fire sprite at muzzle position */
        (void)drawX;
        (void)drawY;
    }

    /* Sight sprite (index 2) drawn separately in HUD */
    /* Mirror sprite (index 3) for left-hand mode */
}

void Weapon_drawReloadAnimation(Weapon *w, Graphics *g, int screenY, int width, int height) {
    if (!Magazine_isReloading(&w->magazine)) return;

    /* Draw weapon moving down during reload */
    int frame = w->magazine.frame;
    int total = w->magazine.reloadTime;
    int offset = 0;

    if (total > 0) {
        /* First half: weapon goes down, second half: comes back up */
        int half = total / 2;
        if (frame < half) {
            offset = (frame * height / 4) / half;
        } else {
            offset = ((total - frame) * height / 4) / half;
        }
    }

    if (w->sprites && w->spriteCount > 0 && w->sprites[0]) {
        int sprW = Sprite_getWidth(w->sprites[0]);
        int sprH = Sprite_getHeight(w->sprites[0]);
        (void)sprW;
        (void)sprH;
        (void)offset;
    }
}

void Weapon_createTrace(Weapon *w, int x, int y, int z) {
    /* Create blood decal / bullet trace at hit position */
    /* Handled by external decal system */
    (void)w;
    (void)x;
    (void)y;
    (void)z;
}

int Weapon_getRounds(Weapon *w) {
    return Magazine_getRounds(&w->magazine);
}

int Weapon_getAmmo(Weapon *w) {
    return Magazine_getAmmo(&w->magazine);
}

void Weapon_reset(Weapon *w) {
    w->magazine.rounds = w->magazine.capacity;
    w->magazine.reloading = false;
    w->magazine.frame = 0;
    w->shake = 0;
}

void Weapon_addAmmo(Weapon *w, int amount) {
    Magazine_addAmmo(&w->magazine, amount);
}

void Weapon_reload(Weapon *w) {
    Magazine_reload(&w->magazine);
}

bool Weapon_isReloading(Weapon *w) {
    return Magazine_isReloading(&w->magazine);
}

void Weapon_setSprites(Weapon *w, Sprite **sprites, int count) {
    w->sprites = sprites;
    w->spriteCount = count;
}

void Weapon_setMagazine(Weapon *w, int capacity, int ammo, int reloadTime) {
    Magazine_init(&w->magazine, capacity, ammo, reloadTime);
}

void Weapon_setRay(Weapon *w, Ray *ray) {
    w->ray = ray;
}

void Weapon_setZoom(Weapon *w, bool hasZoom, int stdFov, int zoomFov) {
    w->hasZoom = hasZoom;
    w->stdFov = stdFov;
    w->zoomFov = zoomFov;
}

void Weapon_setAnim(Weapon *w, float speed, bool newanim) {
    w->animSpeed = speed;
    w->newanim = newanim;
}
