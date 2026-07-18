/*
 * respawn.h — porte fiel de code/Gameplay/Respawn.java
 */
#ifndef QE_GAMEPLAY_RESPAWN_H
#define QE_GAMEPLAY_RESPAWN_H

#include "../math/vector3d.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct House House;

typedef struct Respawn {
    Vector3D point;
    int      part;
    int8_t   mode;
    int8_t   cmode;
    bool     respa;
} Respawn;

void Respawn_init(Respawn *r, Vector3D *pos, House *house);

#endif
