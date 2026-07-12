/*
 * mesh_object.h — porte fiel de code/Gameplay/Objects/MeshObject.java
 */
#ifndef QE_GAMEPLAY_OBJECTS_MESH_OBJECT_H
#define QE_GAMEPLAY_OBJECTS_MESH_OBJECT_H

#include <stdbool.h>
#include "../../math/vector3d.h"

typedef struct RoomObject    RoomObject;
typedef struct Morphing      Morphing;
typedef struct MultyTexture  MultyTexture;
typedef struct BoundingBox   BoundingBox;
typedef struct Character     Character;
typedef struct Matrix        Matrix;

/* Lift states */
#define MESH_OBJECT_STAY_BEGIN  0
#define MESH_OBJECT_DRIVE       1
#define MESH_OBJECT_STAY_END    2
#define MESH_OBJECT_DRIVE_BACK  3

/* Animation/state constants */
#define MESH_OBJECT_STATE_STAY  0
#define MESH_OBJECT_STATE_OPEN  4
#define MESH_OBJECT_STATE_CLOSE 5
#define MESH_OBJECT_STATE_LOOP  6

/* Animation types */
#define MESH_OBJECT_ANIM_LOOP         0
#define MESH_OBJECT_ANIM_OPEN         1
#define MESH_OBJECT_ANIM_OPEN_CLOSE   2

typedef struct MeshObject {
    RoomObject    base;             /* heranca: RoomObject */
    Morphing     *morphing;
    MultyTexture *texture;
    BoundingBox  *bbox;

    /* Lift/path system */
    Vector3D     *poses;
    int           posesCount;
    int          *timeToMove;
    bool          liftCycled;
    bool          liftReUse;
    bool          disactivateOnEnd;
    bool          liftCanBePaused;
    bool          liftRotateToMove;
    int           liftSmoothMove;
    Vector3D     *playerFollowLift;

    /* Animation */
    float         animSpeed;
    int           animType;
    int           state;
    int           liftState;
    int           liftPoseIndex;
    int           liftFrame;

    /* Visual */
    int           addsz;
    bool          precCol;
    bool          ignoreWeaponRayCast;
    bool          realtimeLighting;

    /* Direction interpolation */
    int           targetDirX;
    int           targetDirZ;
    int           smoothDirSpeed;
} MeshObject;

void MeshObject_init(MeshObject *self, Morphing *morphing, MultyTexture *tex, BoundingBox *bbox);
void MeshObject_destroy(MeshObject *self);

void MeshObject_activate(MeshObject *self);
void MeshObject_update(MeshObject *self);

/* Lift interpolation */
void MeshObject_getNewPos(MeshObject *self, Vector3D *out, int frame, int poseFrom, int poseTo, int totalTime);

/* Direction */
void MeshObject_lookAt(MeshObject *self, int x, int z, int speed);
void MeshObject_lookAtDirect(MeshObject *self, int x, int z);

/* Collision */
bool MeshObject_sphereCollisionTest(MeshObject *self, Character *body, int radius);
int  MeshObject_computeHeight(MeshObject *self, int x, int z);
bool MeshObject_rayCast(MeshObject *self, Vector3D *origin, Vector3D *dir);

/* Path setup */
void MeshObject_setPoses(MeshObject *self, Vector3D *poses, int count, int *timeToMove);
void MeshObject_setAnimType(MeshObject *self, int type);
void MeshObject_setAnimSpeed(MeshObject *self, float speed);

#endif
