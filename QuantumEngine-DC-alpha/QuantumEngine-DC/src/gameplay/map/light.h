/*
 * light.h — porte fiel de code/Gameplay/Map/Light.java
 */
#ifndef QE_GAMEPLAY_MAP_LIGHT_H
#define QE_GAMEPLAY_MAP_LIGHT_H

#include "../../math/vector3d.h"
#include <stdint.h>

typedef struct Light {
    int      color[3];
    Vector3D pos;
    Vector3D *direction;
    int16_t  ceilingFix;
    int16_t  floorFix;
    int      part;
} Light;

static inline void Light_init(Light *l, int *posArr, int *colorArr) {
    l->pos.x = posArr[0];
    l->pos.y = posArr[1];
    l->pos.z = posArr[2];
    l->color[0] = colorArr[0];
    l->color[1] = colorArr[1];
    l->color[2] = colorArr[2];
    l->direction = NULL;
    l->ceilingFix = 0;
    l->floorFix = 0;
    l->part = -1;
}

#endif
