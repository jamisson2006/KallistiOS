/*
 * ray_cast.h — porte fiel de code/Collision/RayCast.java
 */
#ifndef QE_COLLISION_RAY_CAST_H
#define QE_COLLISION_RAY_CAST_H

#include <stdint.h>
#include "ray.h"
#include "../math/vector3d.h"

typedef struct Mesh   Mesh;
typedef struct Matrix Matrix;

void    RayCast_superFastRayCast(Mesh *mesh, Ray *ray);

void    RayCast_rayCast(Mesh *mesh, Ray *ray);
void    RayCast_rayCastRecalc(Mesh *mesh, Ray *ray, bool reCalcNorm);
void    RayCast_rayCastMatrix(Mesh *mesh, Ray *ray, Matrix *mat);
void    RayCast_rayCastRoom(Mesh *mesh, Ray *ray, int rom);

int64_t RayCast_isRayOnPolygon4(Vector3D *a, Vector3D *b, Vector3D *c, Vector3D *d,
                                Vector3D *nor, Vector3D *start, Vector3D *dir);
int64_t RayCast_isRayOnPolygon3(Vector3D *a, Vector3D *b, Vector3D *c,
                                Vector3D *nor, Vector3D *start, Vector3D *dir);

#endif
