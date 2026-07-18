/*
 * teleport.h — porte fiel de code/Gameplay/Objects/Teleport.java (Quantum Engine J2ME)
 * @author Roman Lahin
 */
#ifndef QE_GAMEPLAY_OBJECTS_TELEPORT_H
#define QE_GAMEPLAY_OBJECTS_TELEPORT_H

#include "../map/room_object.h"
#include "../../math/vector3d.h"

typedef struct Teleport {
    RoomObject base;  /* heranca */
    Vector3D   pos;   /* posicao do trigger */
    Vector3D   npos;  /* posicao destino */
    int        pRot;  /* rotacao destino do player */
} Teleport;

Teleport *Teleport_new(void);
void      Teleport_free(Teleport *self);
void      Teleport_activate(Teleport *self, Player *player, House *house, GameScreen *gs);

#endif
