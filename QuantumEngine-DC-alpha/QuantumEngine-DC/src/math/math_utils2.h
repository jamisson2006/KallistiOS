/*
 * math_utils2.h — porte fiel de code/Math/MathUtils2.java (Quantum Engine J2ME)
 * @author DDDEN!SSS
 */
#ifndef QE_MATH_MATH_UTILS2_H
#define QE_MATH_MATH_UTILS2_H

#include "vector3d.h"

typedef struct Vertex Vertex;

float     MathUtils2_invSqrt(float val);

Vector3D *MathUtils2_createNormal(const Vector3D *a, const Vector3D *b, const Vector3D *c);
void      MathUtils2_calcNormalV (Vector3D *nor, const Vector3D *a, const Vector3D *b, const Vector3D *c);
Vector3D *MathUtils2_calcNormalVx(const Vertex *a, const Vertex *b, const Vertex *c);
void      MathUtils2_calcNormal  (Vector3D *nor,
                                  int ax, int ay, int az,
                                  int bx, int by, int bz,
                                  int cx, int cy, int cz);
Vector3D *MathUtils2_calcNormalNew(int ax, int ay, int az,
                                   int bx, int by, int bz,
                                   int cx, int cy, int cz);

int  MathUtils2_computePolygonY(const Vertex *a, int nx, int ny, int nz, int px, int py, int pz);

long MathUtils2_side(int px, int py, int ax, int ay, int bx, int by);

int  MathUtils2_distanceToFaceV(const Vector3D *a, const Vector3D *nor, int px, int py, int pz);
int  MathUtils2_distanceToFaceX(const Vertex   *a, const Vector3D *nor, int px, int py, int pz);
int  MathUtils2_distanceToLine (const Vector3D *point, const Vector3D *a, const Vector3D *b);
int  MathUtils2_distanceToRay  (const Vector3D *point, const Vector3D *a, const Vector3D *dir);

int  MathUtils2_isPointOnPolygon4v(const Vector3D *point, const Vector3D *a, const Vector3D *b,
                                   const Vector3D *c, const Vector3D *d, const Vector3D *normal);
int  MathUtils2_isPointOnPolygon4 (int pointx, int pointy, int pointz,
                                    int ax, int ay, int az,
                                    int bx, int by, int bz,
                                    int cx, int cy, int cz,
                                    int dx, int dy, int dz,
                                    int normalx, int normaly, int normalz);

int  MathUtils2_isPointOnPolygon3v(const Vector3D *point, const Vector3D *a, const Vector3D *b,
                                    const Vector3D *c, const Vector3D *normal);
int  MathUtils2_isPointOnPolygon3 (int pointx, int pointy, int pointz,
                                    int ax, int ay, int az,
                                    int bx, int by, int bz,
                                    int cx, int cy, int cz,
                                    int normalx, int normaly, int normalz);

/* Overloads 2D (Java: isPointOnPolygon(int...) diversos): */
int  MathUtils2_isPointOnPolygon2D_4norY(int px, int pz,
                                         int ax, int az, int bx, int bz, int cx, int cz, int dx, int dz,
                                         int norY);
int  MathUtils2_isPointOnPolygon2D_3norY(int px, int pz,
                                         int ax, int az, int bx, int bz, int cx, int cz,
                                         int norY);
int  MathUtils2_isPointOnPolygon2D_4    (int px, int py,
                                         int x1, int y1, int x2, int y2,
                                         int x3, int y3, int x4, int y4);
int  MathUtils2_isPointOnPolygon2D_3    (int px, int py,
                                         int x1, int y1, int x2, int y2, int x3, int y3);

#endif
