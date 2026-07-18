/*
 * sphere_cast.h — porte fiel de code/Collision/SphereCast.java
 */
#ifndef QE_COLLISION_SPHERE_CAST_H
#define QE_COLLISION_SPHERE_CAST_H

#include <stdbool.h>
#include "../math/vector3d.h"

typedef struct Mesh   Mesh;
typedef struct Matrix Matrix;

bool SphereCast_isSphereAABBCollision(Vector3D *pos, int rad, int minx, int maxx, int minz, int maxz);

bool SphereCast_sphereCast(Mesh *mesh, Vector3D *pos, int rad);
bool SphereCast_sphereCastMatrix(Mesh *mesh, Matrix *matrix, Vector3D *pos, int rad);
bool SphereCast_sphereCastFull(Mesh *mesh, Vector3D *pos, int rad, bool recalc);

#endif
