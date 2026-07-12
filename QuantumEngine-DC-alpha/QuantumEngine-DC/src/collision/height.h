/*
 * height.h — porte fiel de code/Collision/Height.java
 */
#ifndef QE_COLLISION_HEIGHT_H
#define QE_COLLISION_HEIGHT_H

#include <stdbool.h>
#include "../math/vector3d.h"

typedef struct RenderObject RenderObject;
typedef struct Matrix       Matrix;

typedef struct Height {
    int          height;
    Vector3D     position;
    RenderObject *polygon;
    int          cx, cz, cy;
    Matrix       *oldMatrix;
    bool         updatePos;
    bool         underRoof;
} Height;

void          Height_init(Height *self);
void          Height_reset(Height *self);
void          Height_set(Height *self, int height, RenderObject *polygon,
                         int cx, int cz, int cy, Matrix *oldMatrix, bool updatePos);

void          Height_setPolygon(Height *self, RenderObject *polygon);
RenderObject *Height_getPolygon(const Height *self);

int           Height_getCentreX(const Height *self);
int           Height_getCentreZ(const Height *self);
int           Height_getCentreY(const Height *self);

void          Height_setHeight(Height *self, int height);
int           Height_getHeight(const Height *self);

void          Height_setMatrix(Height *self, Matrix *matrix);
Matrix       *Height_getMatrix(const Height *self);

void          Height_setUnderRoof(Height *self, bool x);
bool          Height_updatePosition(const Height *self);
Vector3D     *Height_getPosition(Height *self);
bool          Height_isUnderRoof(const Height *self);

#endif
