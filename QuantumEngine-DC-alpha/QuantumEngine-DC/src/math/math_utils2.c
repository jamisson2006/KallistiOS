/*
 * math_utils2.c — porte fiel de code/Math/MathUtils2.java (Quantum Engine J2ME)
 */
#include "math_utils2.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Vertex definido em Rendering. Aqui declaramos accessors extern. */
extern int Vertex_x(const Vertex *v);
extern int Vertex_y(const Vertex *v);
extern int Vertex_z(const Vertex *v);

static Vector3D tVec;

float MathUtils2_invSqrt(float val) {
    /* Constante 1597463007 (0x5F375A86) — a mesma do Java (Robertson).
     * Diferente da versao Quake (0x5F3759DF). Preservada fielmente. */
    float xhalf = 0.5f * val;
    union { float f; int32_t i; } u;
    u.f = val;
    u.i = 1597463007 - (u.i >> 1);
    val = u.f;
    return val * (1.5f - xhalf * val * val);
}

Vector3D *MathUtils2_createNormal(const Vector3D *a, const Vector3D *b, const Vector3D *c) {
    Vector3D *nor = Vector3D_new();
    MathUtils2_calcNormal(nor, a->x, a->y, a->z, b->x, b->y, b->z, c->x, c->y, c->z);
    return nor;
}
void MathUtils2_calcNormalV(Vector3D *nor, const Vector3D *a, const Vector3D *b, const Vector3D *c) {
    MathUtils2_calcNormal(nor, a->x, a->y, a->z, b->x, b->y, b->z, c->x, c->y, c->z);
}
Vector3D *MathUtils2_calcNormalVx(const Vertex *a, const Vertex *b, const Vertex *c) {
    return MathUtils2_calcNormalNew(
        Vertex_x(a), Vertex_y(a), Vertex_z(a),
        Vertex_x(b), Vertex_y(b), Vertex_z(b),
        Vertex_x(c), Vertex_y(c), Vertex_z(c));
}

void MathUtils2_calcNormal(Vector3D *nor,
                           int ax, int ay, int az,
                           int bx, int by, int bz,
                           int cx, int cy, int cz) {
    double x = (double)((long)(ay-by)*(az-cz)) - (double)((long)(az-bz)*(ay-cy));
    double y = (double)((long)(az-bz)*(ax-cx)) - (double)((long)(ax-bx)*(az-cz));
    double z = (double)((long)(ax-bx)*(ay-cy)) - (double)((long)(ay-by)*(ax-cx));
    double sqr = sqrt(x*x + y*y + z*z) / 4096.0;
    if (sqr == 0.0) sqr = 1.0;
    Vector3D_set(nor, (int)(x/sqr), (int)(y/sqr), (int)(z/sqr));
}

Vector3D *MathUtils2_calcNormalNew(int ax, int ay, int az,
                                   int bx, int by, int bz,
                                   int cx, int cy, int cz) {
    Vector3D *nor = Vector3D_new_xyz(0, 0, 0);
    MathUtils2_calcNormal(nor, ax, ay, az, bx, by, bz, cx, cy, cz);
    return nor;
}

int MathUtils2_computePolygonY(const Vertex *a, int nx, int ny, int nz, int px, int py, int pz) {
    if (ny >= 0) return INT_MAX;
    return py - ((px - Vertex_x(a))*nx + (py - Vertex_y(a))*ny + (pz - Vertex_z(a))*nz) / ny;
}

long MathUtils2_side(int px, int py, int ax, int ay, int bx, int by) {
    return (long)(bx-ax)*(py-ay) - (long)(px-ax)*(by-ay);
}

int MathUtils2_distanceToFaceV(const Vector3D *a, const Vector3D *nor, int px, int py, int pz) {
    Vector3D_set(&tVec, px - a->x, py - a->y, pz - a->z);
    return Vector3D_dot(&tVec, nor) >> 12;
}
int MathUtils2_distanceToFaceX(const Vertex *a, const Vector3D *nor, int px, int py, int pz) {
    Vector3D_set(&tVec, px - Vertex_x(a), py - Vertex_y(a), pz - Vertex_z(a));
    return Vector3D_dot(&tVec, nor) >> 12;
}

int MathUtils2_distanceToLine(const Vector3D *point, const Vector3D *a, const Vector3D *b) {
    int dx = b->x - a->x, dy = b->y - a->y, dz = b->z - a->z;
    int wx = point->x - a->x, wy = point->y - a->y, wz = point->z - a->z;

    const int fp = 14;
    long dp = (long)dx*dx + dy*dy + dz*dz;
    long dt = 0;
    if (dp != 0) dt = (((long)wx*dx + wy*dy + wz*dz) << fp) / dp;
    if (dt < 0) dt = 0;
    if (dt > 16384) dt = 16384;

    dx = a->x + (int)(dx * dt >> fp);
    dy = a->y + (int)(dy * dt >> fp);
    dz = a->z + (int)(dz * dt >> fp);

    dx -= point->x; dy -= point->y; dz -= point->z;
    return dx*dx + dy*dy + dz*dz;
}

int MathUtils2_distanceToRay(const Vector3D *point, const Vector3D *a, const Vector3D *dir) {
    int dx = dir->x, dy = dir->y, dz = dir->z;
    int wx = point->x - a->x, wy = point->y - a->y, wz = point->z - a->z;

    const int fp = 14;
    long dp = (long)dx*dx + dy*dy + dz*dz;
    long dt = 0;
    if (dp != 0) dt = (((long)wx*dx + wy*dy + wz*dz) << fp) / dp;
    if (dt < 0) dt = 0;
    if (dt > 16384) dt = 16384;

    dx = a->x + (int)(dx * dt >> fp);
    dy = a->y + (int)(dy * dt >> fp);
    dz = a->z + (int)(dz * dt >> fp);

    dx -= point->x; dy -= point->y; dz -= point->z;
    return dx*dx + dy*dy + dz*dz;
}

int MathUtils2_isPointOnPolygon2D_4(int px, int py, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) {
    return  (long)(x2-x1)*(py-y1) <= (long)(px-x1)*(y2-y1) &&
            (long)(x3-x2)*(py-y2) <= (long)(px-x2)*(y3-y2) &&
            (long)(x4-x3)*(py-y3) <= (long)(px-x3)*(y4-y3) &&
            (long)(x1-x4)*(py-y4) <= (long)(px-x4)*(y1-y4);
}
int MathUtils2_isPointOnPolygon2D_3(int px, int py, int x1, int y1, int x2, int y2, int x3, int y3) {
    return  (long)(x2-x1)*(py-y1) <= (long)(px-x1)*(y2-y1) &&
            (long)(x3-x2)*(py-y2) <= (long)(px-x2)*(y3-y2) &&
            (long)(x1-x3)*(py-y3) <= (long)(px-x3)*(y1-y3);
}

int MathUtils2_isPointOnPolygon2D_4norY(int px, int pz,
                                        int ax, int az, int bx, int bz, int cx, int cz, int dx, int dz,
                                        int norY) {
    if (norY >= 0)
        return MathUtils2_isPointOnPolygon2D_4(px, pz, ax, az, bx, bz, cx, cz, dx, dz);
    return MathUtils2_isPointOnPolygon2D_4(px, pz, dx, dz, cx, cz, bx, bz, ax, az);
}
int MathUtils2_isPointOnPolygon2D_3norY(int px, int pz,
                                        int ax, int az, int bx, int bz, int cx, int cz,
                                        int norY) {
    if (norY >= 0)
        return MathUtils2_isPointOnPolygon2D_3(px, pz, ax, az, bx, bz, cx, cz);
    return MathUtils2_isPointOnPolygon2D_3(px, pz, cx, cz, bx, bz, ax, az);
}

int MathUtils2_isPointOnPolygon4v(const Vector3D *point, const Vector3D *a, const Vector3D *b,
                                  const Vector3D *c, const Vector3D *d, const Vector3D *normal) {
    return MathUtils2_isPointOnPolygon4(point->x, point->y, point->z,
        a->x, a->y, a->z, b->x, b->y, b->z, c->x, c->y, c->z, d->x, d->y, d->z,
        normal->x, normal->y, normal->z);
}

int MathUtils2_isPointOnPolygon4(int pointx, int pointy, int pointz,
                                 int ax, int ay, int az,
                                 int bx, int by, int bz,
                                 int cx, int cy, int cz,
                                 int dx, int dy, int dz,
                                 int normalx, int normaly, int normalz) {
    const int nx = normalx > 0 ? normalx : -normalx;
    const int ny = normaly > 0 ? normaly : -normaly;
    const int nz = normalz > 0 ? normalz : -normalz;

    if (nx >= ny && nx >= nz) {
        if (normalx >= 0)
            return MathUtils2_isPointOnPolygon2D_4(pointz, pointy, az, ay, bz, by, cz, cy, dz, dy);
        else
            return MathUtils2_isPointOnPolygon2D_4(pointz, pointy, dz, dy, cz, cy, bz, by, az, ay);
    }
    if (ny >= nx && ny >= nz) {
        if (normaly >= 0)
            return MathUtils2_isPointOnPolygon2D_4(pointx, pointz, ax, az, bx, bz, cx, cz, dx, dz);
        else
            return MathUtils2_isPointOnPolygon2D_4(pointx, pointz, dx, dz, cx, cz, bx, bz, ax, az);
    }
    if (nz >= nx && nz >= ny) {
        if (normalz <= 0)
            return MathUtils2_isPointOnPolygon2D_4(pointx, pointy, ax, ay, bx, by, cx, cy, dx, dy);
        else
            return MathUtils2_isPointOnPolygon2D_4(pointx, pointy, dx, dy, cx, cy, bx, by, ax, ay);
    }
    return 1;
}

int MathUtils2_isPointOnPolygon3v(const Vector3D *point, const Vector3D *a, const Vector3D *b,
                                  const Vector3D *c, const Vector3D *normal) {
    return MathUtils2_isPointOnPolygon3(point->x, point->y, point->z,
        a->x, a->y, a->z, b->x, b->y, b->z, c->x, c->y, c->z,
        normal->x, normal->y, normal->z);
}
int MathUtils2_isPointOnPolygon3(int pointx, int pointy, int pointz,
                                 int ax, int ay, int az,
                                 int bx, int by, int bz,
                                 int cx, int cy, int cz,
                                 int normalx, int normaly, int normalz) {
    const int nx = normalx > 0 ? normalx : -normalx;
    const int ny = normaly > 0 ? normaly : -normaly;
    const int nz = normalz > 0 ? normalz : -normalz;

    if (nx >= ny && nx >= nz) {
        if (normalx >= 0)
            return MathUtils2_isPointOnPolygon2D_3(pointz, pointy, az, ay, bz, by, cz, cy);
        return MathUtils2_isPointOnPolygon2D_3(pointz, pointy, cz, cy, bz, by, az, ay);
    }
    if (ny >= nx && ny >= nz) {
        if (normaly >= 0)
            return MathUtils2_isPointOnPolygon2D_3(pointx, pointz, ax, az, bx, bz, cx, cz);
        return MathUtils2_isPointOnPolygon2D_3(pointx, pointz, cx, cz, bx, bz, ax, az);
    }
    if (nz >= nx && nz >= ny) {
        if (normalz <= 0)
            return MathUtils2_isPointOnPolygon2D_3(pointx, pointy, ax, ay, bx, by, cx, cy);
        return MathUtils2_isPointOnPolygon2D_3(pointx, pointy, cx, cy, bx, by, ax, ay);
    }
    return 1;
}
