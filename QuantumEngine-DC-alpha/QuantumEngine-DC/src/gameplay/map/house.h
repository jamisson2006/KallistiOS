/*
 * house.h — porte fiel de code/Gameplay/Map/House.java
 */
#ifndef QE_GAMEPLAY_MAP_HOUSE_H
#define QE_GAMEPLAY_MAP_HOUSE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct Room       Room;
typedef struct RoomObject RoomObject;
typedef struct Light      Light;
typedef struct Skybox     Skybox;
typedef struct DirectX7   DirectX7;
typedef struct Vector3D   Vector3D;

typedef struct House {
    Room  **rooms;
    int     roomCount;

    /* neighbours[i][j] — pointer to Room if rooms i and j are neighbours, else NULL */
    Room ***neighbours;

    Skybox *skybox;
} House;

/* Static config flags */
extern bool House_l2dRoomRendering;
extern bool House_boxRoomTesting;

/* Construction / destruction */
House *House_new(Room **rooms, int roomCount, Room ***neighbours);
void   House_destroy(House *self);

/* Physics / collision */
int  House_sphereCast(House *self, int part, int x, int y, int z, int radius);
int  House_rayCast(House *self, int part, int x1, int y1, int z1, int x2, int y2, int z2);
int  House_computeHeight(House *self, int part, int x, int z);
int  House_computeHeightFull(House *self, int x, int z);

/* Object queries */
RoomObject **House_getNearObjects(House *self, int part, int *outCount);
RoomObject **House_getObjects(House *self, int *outCount);

/* Rendering */
void House_render(House *self, DirectX7 *g3d, int cameraPart);

/* Room / part logic */
int   House_calcPart(House *self, int x, int y, int z);
void  House_addObject(House *self, RoomObject *obj, int part);
void  House_removeObject(House *self, RoomObject *obj, int part);
void  House_recomputePart(House *self, RoomObject *obj);

/* Lights */
void  House_sortLights(House *self, int x, int y, int z, int part);

/* Neighbour queries */
Room **House_getNeighbourRooms(House *self, int part, int *outCount);
bool   House_isNear(House *self, int roomA, int roomB);

/* Accessors */
Room **House_getRooms(House *self);
int    House_getRoomCount(House *self);
void   House_setSkybox(House *self, Skybox *skybox);
Skybox*House_getSkybox(House *self);

#endif
