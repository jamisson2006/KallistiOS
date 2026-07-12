/*
 * ray.c — porte fiel de code/Collision/Ray.java
 */
#include "ray.h"
#include <limits.h>

void Ray_init(Ray *self) {
    self->collision = false;
    self->distance = INT32_MAX;
    self->triangle = NULL;
    self->origPol = NULL;
    self->doubleSide = false;
    self->findNearest = false;
    self->infinity = false;
    self->ignoreNonShadowed = false;
    self->onlyCollidable = true;
    self->numRoom = -1;
    self->collisionPoint.x = 0;
    self->collisionPoint.y = 0;
    self->collisionPoint.z = 0;
    self->start.x = 0;
    self->start.y = 0;
    self->start.z = 0;
    self->dir.x = 0;
    self->dir.y = 0;
    self->dir.z = 0;
}

void Ray_reset(Ray *self) {
    self->collision = false;
    self->distance = INT32_MAX;
    self->origPol = NULL;
}

void          Ray_setTriangle(Ray *self, RenderObject *i) { self->triangle = i; }
RenderObject *Ray_getTriangle(const Ray *self) { return self->triangle; }

void Ray_setNumRoom(Ray *self, int i) { self->numRoom = i; }
int  Ray_getNumRoom(const Ray *self) { return self->numRoom; }

bool Ray_isCollision(const Ray *self) { return self->collision; }
int  Ray_getDistance(const Ray *self) { return self->distance; }

Vector3D *Ray_getCollisionPoint(Ray *self) { return &self->collisionPoint; }
Vector3D *Ray_getStart(Ray *self) { return &self->start; }
Vector3D *Ray_getDir(Ray *self) { return &self->dir; }
