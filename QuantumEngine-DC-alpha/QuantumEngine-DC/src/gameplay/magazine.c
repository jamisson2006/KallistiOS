/*
 * magazine.c — porte fiel de code/Gameplay/Magazine.java
 */
#include "magazine.h"
#include "../utils/qfps.h"

void Magazine_init(Magazine *m, int capacity, int ammo, int reloadTime) {
    m->capacity = capacity;
    m->ammo = ammo;
    m->rounds = capacity;
    m->reloadTime = reloadTime;
    m->frame = 0;
    m->reloading = false;
}

void Magazine_update(Magazine *m, int frameTime) {
    if (m->reloading) {
        m->frame += frameTime;
        if (m->frame >= m->reloadTime) {
            m->reloading = false;
            m->frame = 0;
            int need = m->capacity - m->rounds;
            if (need > m->ammo) need = m->ammo;
            m->rounds += need;
            m->ammo -= need;
        }
    }
}

bool Magazine_shoot(Magazine *m) {
    if (m->rounds <= 0 || m->reloading) return false;
    m->rounds--;
    if (m->rounds <= 0 && m->ammo > 0) {
        Magazine_reload(m);
    }
    return true;
}

void Magazine_reload(Magazine *m) {
    if (m->reloading || m->ammo <= 0 || m->rounds >= m->capacity) return;
    m->reloading = true;
    m->frame = 0;
}

void Magazine_addAmmo(Magazine *m, int amount) {
    m->ammo += amount;
}

int Magazine_getRounds(Magazine *m) { return m->rounds; }
int Magazine_getAmmo(Magazine *m) { return m->ammo; }
bool Magazine_isReloading(Magazine *m) { return m->reloading; }
