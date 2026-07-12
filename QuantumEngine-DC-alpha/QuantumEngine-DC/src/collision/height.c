/*
 * height.c — porte fiel de code/Collision/Height.java
 */
#include "height.h"
#include <limits.h>

void Height_init(Height *self) {
    self->height = INT32_MIN;
    self->position.x = 0;
    self->position.y = 0;
    self->position.z = 0;
    self->polygon = NULL;
    self->cx = 0;
    self->cz = 0;
    self->cy = 0;
    self->oldMatrix = NULL;
    self->updatePos = false;
    self->underRoof = false;
}

void Height_reset(Height *self) {
    self->polygon = NULL;
    self->cx = INT32_MIN;
    self->cz = INT32_MIN;
    self->height = INT32_MIN;
    self->oldMatrix = NULL;
    self->updatePos = false;
    self->underRoof = false;
}

void Height_set(Height *self, int height, RenderObject *polygon,
                int cx, int cz, int cy, Matrix *oldMatrix, bool updatePos) {
    self->height = height;
    self->polygon = polygon;
    self->cx = cx;
    self->cz = cz;
    self->cy = cy;
    self->oldMatrix = oldMatrix;
    self->updatePos = updatePos;
}

void Height_setPolygon(Height *self, RenderObject *polygon) {
    self->polygon = polygon;
}

RenderObject *Height_getPolygon(const Height *self) {
    return self->polygon;
}

int Height_getCentreX(const Height *self) { return self->cx; }
int Height_getCentreZ(const Height *self) { return self->cz; }
int Height_getCentreY(const Height *self) { return self->cy; }

void Height_setHeight(Height *self, int height) { self->height = height; }
int  Height_getHeight(const Height *self) { return self->height; }

void    Height_setMatrix(Height *self, Matrix *matrix) { self->oldMatrix = matrix; }
Matrix *Height_getMatrix(const Height *self) { return self->oldMatrix; }

void Height_setUnderRoof(Height *self, bool x) { self->underRoof = x; }
bool Height_updatePosition(const Height *self) { return self->updatePos; }

Vector3D *Height_getPosition(Height *self) { return &self->position; }

bool Height_isUnderRoof(const Height *self) { return self->underRoof; }
