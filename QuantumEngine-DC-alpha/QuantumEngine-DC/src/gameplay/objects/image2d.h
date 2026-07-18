/*
 * image2d.h — porte fiel de code/Gameplay/Objects/Image2D.java
 */
#ifndef QE_GAMEPLAY_OBJECTS_IMAGE2D_H
#define QE_GAMEPLAY_OBJECTS_IMAGE2D_H

#include "../map/room_object.h"
#include "../../math/vector3d.h"
#include <stdint.h>

typedef struct Image Image;

typedef struct Image2D {
    RoomObject base;
    Vector3D   pos;
    Image     *img;
    int64_t    timer;
} Image2D;

Image2D *Image2D_new(Vector3D *pos, Image *img, int64_t timer);
void     Image2D_destroy(Image2D *self);
void     Image2D_activate(Image2D *self, House *house, Player *player, GameScreen *gs);

int Image2D_getPosX(Image2D *self);
int Image2D_getPosY(Image2D *self);
int Image2D_getPosZ(Image2D *self);
void Image2D_setPos(Image2D *self, int x, int y, int z);

#endif
