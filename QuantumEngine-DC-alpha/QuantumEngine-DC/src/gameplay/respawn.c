/*
 * respawn.c — porte fiel de code/Gameplay/Respawn.java
 */
#include "respawn.h"
#include "map/house.h"

void Respawn_init(Respawn *r, Vector3D *pos, House *house) {
    r->point = *pos;
    r->part = House_calcPart(house, pos->x, pos->y, pos->z);
    r->mode = -128;
    r->cmode = -128;
    r->respa = false;
}
