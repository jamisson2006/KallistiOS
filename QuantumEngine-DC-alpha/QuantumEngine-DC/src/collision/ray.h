/*
 * ray.h — porte fiel de code/Collision/Ray.java
 */
#ifndef QE_COLLISION_RAY_H
#define QE_COLLISION_RAY_H

#include <stdbool.h>
#include "../math/vector3d.h"

typedef struct RenderObject RenderObject;

typedef struct Ray {
    bool         collision;
    int          distance;
    RenderObject *triangle;
    RenderObject *origPol;
    bool         doubleSide;
    bool         findNearest;
    bool         infinity;
    bool         ignoreNonShadowed;
    bool         onlyCollidable;
    int          numRoom;
    Vector3D     collisionPoint;
    Vector3D     start;
    Vector3D     dir;
} Ray;

void          Ray_init(Ray *self);
void          Ray_reset(Ray *self);

void          Ray_setTriangle(Ray *self, RenderObject *i);
RenderObject *Ray_getTriangle(const Ray *self);

void          Ray_setNumRoom(Ray *self, int i);
int           Ray_getNumRoom(const Ray *self);

bool          Ray_isCollision(const Ray *self);
int           Ray_getDistance(const Ray *self);

Vector3D     *Ray_getCollisionPoint(Ray *self);
Vector3D     *Ray_getStart(Ray *self);
Vector3D     *Ray_getDir(Ray *self);

#endif
