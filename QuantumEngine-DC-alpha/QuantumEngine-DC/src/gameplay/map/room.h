/*
 * room.h — porte fiel de code/Gameplay/Map/Room.java
 */
#ifndef QE_GAMEPLAY_MAP_ROOM_H
#define QE_GAMEPLAY_MAP_ROOM_H

#include <stdbool.h>
#include <stdint.h>

typedef struct Mesh         Mesh;
typedef struct BoundingBox  BoundingBox;
typedef struct MultyTexture MultyTexture;
typedef struct Light        Light;
typedef struct RoomObject   RoomObject;
typedef struct DirectX7     DirectX7;
typedef struct Vector3D     Vector3D;

/* Portal — shared vertices between two rooms */
typedef struct Portal {
    int targetRoom;
    int minX, maxX, minY, maxY, minZ, maxZ;
} Portal;

typedef struct Room {
    int id;

    /* Physics collision meshes (2D chunk grid) */
    Mesh **mesh;
    int    meshRowCount;
    int    meshColCount;

    /* Render meshes (2D chunk grid) */
    Mesh **renderMesh;
    int    renderRowCount;
    int    renderColCount;

    /* Bounding boxes per render chunk */
    BoundingBox **boundingBoxes;

    /* Full un-chunked mesh (used for portal detection etc.) */
    Mesh *fullMesh;

    /* Portals to neighbouring rooms */
    Portal **portals;
    int      portalCount;

    /* Axis-aligned bounds */
    int minX, maxX, minY, maxY, minZ, maxZ;

    /* Lights in this room */
    Light **lights;
    int     lightCount;

    /* Objects in this room */
    RoomObject **objects;
    int          objectCount;
    int          objectCapacity;

    /* Sound */
    char **stepSound;
    int    stepSoundCount;
    char  *jumpSound;
    char  *reverb;
} Room;

/* Static config */
extern int Room_chunkSize;
extern int Room_chunkSizeRender;

/* Construction / destruction */
Room *Room_new(int id);
void  Room_destroy(Room *self);

/* Load meshes from .3d binary file */
Mesh *Room_loadMeshes(const char *path, int scaleX, int scaleY, int scaleZ, MultyTexture *texture);

/* Collision / physics queries */
int  Room_sphereCast(Room *self, int x, int y, int z, int radius);
int  Room_rayCast(Room *self, int x1, int y1, int z1, int x2, int y2, int z2);
int  Room_computeHeight(Room *self, int x, int z);
bool Room_isPointOnMesh(Room *self, int x, int z);
bool Room_openSkyTest(Room *self, int x, int y, int z);

/* Chunk creation */
void Room_createChunks(Room *self);

/* Accessors */
int   Room_getId(Room *self);
Mesh *Room_getMesh(Room *self, int row, int col);
int   Room_getMinX(Room *self);
int   Room_getMaxX(Room *self);
int   Room_getMinY(Room *self);
int   Room_getMaxY(Room *self);
int   Room_getMinZ(Room *self);
int   Room_getMaxZ(Room *self);

/* Object management */
void Room_addObject(Room *self, RoomObject *obj);
void Room_removeObject(Room *self, RoomObject *obj);

#endif
