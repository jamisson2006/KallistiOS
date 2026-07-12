/*
 * ray_cast.c — porte fiel de code/Collision/RayCast.java
 */
#include "ray_cast.h"
#include "../rendering/meshes/mesh.h"
#include "../rendering/meshes/polygon3v.h"
#include "../rendering/meshes/polygon4v.h"
#include "../rendering/renderobject.h"
#include "../rendering/vertex.h"
#include "../rendering/texture.h"
#include "../rendering/multytexture.h"
#include "../rendering/directx7.h"
#include "../math/math_utils2.h"
#include <limits.h>
#include <math.h>

static Vector3D s_temp;
static Vector3D s_colPoint;
static Vector3D s_normal;
static Vector3D s_v1, s_v2, s_v3, s_v4;
static bool s_roomNeed = false;
static int  s_room = -1;

static int rc_max2(int a, int b) { return a > b ? a : b; }
static int rc_min2(int a, int b) { return a < b ? a : b; }
static int rc_max3(int a, int b, int c) { return rc_max2(a, rc_max2(b, c)); }
static int rc_min3(int a, int b, int c) { return rc_min2(a, rc_min2(b, c)); }
static int rc_max4(int a, int b, int c, int d) { return rc_max2(rc_max2(a, b), rc_max2(c, d)); }
static int rc_min4(int a, int b, int c, int d) { return rc_min2(rc_min2(a, b), rc_min2(c, d)); }

static int64_t rayTracing3(Ray *ray, Vector3D *a, Vector3D *b, Vector3D *c,
                           Vector3D *nor, Vector3D *start, Vector3D *dir,
                           Vector3D *pos, Vector3D *check) {
    pos->x = start->x - check->x;
    pos->y = start->y - check->y;
    pos->z = start->z - check->z;
    int dot = (int)(Vector3D_dotLong(dir, nor) >> 12);
    if (dot <= 0) return INT64_MAX;
    dot = (int)(-Vector3D_dotLong(pos, nor) / dot);
    if (dot < -1 || (dot > 4096 && !ray->infinity)) return INT64_MAX;
    pos->x = start->x + (dir->x * dot >> 12);
    pos->y = start->y + (dir->y * dot >> 12);
    pos->z = start->z + (dir->z * dot >> 12);
    if (MathUtils2_isPointOnPolygon3v(pos, a, b, c, nor)) return dot;
    return INT64_MAX;
}

static int64_t rayTracing4(Ray *ray, Vector3D *a, Vector3D *b, Vector3D *c, Vector3D *d,
                           Vector3D *nor, Vector3D *start, Vector3D *dir,
                           Vector3D *pos, Vector3D *check) {
    pos->x = start->x - check->x;
    pos->y = start->y - check->y;
    pos->z = start->z - check->z;
    int dot = (int)(Vector3D_dotLong(dir, nor) >> 12);
    if (dot <= 0) return INT64_MAX;
    dot = (int)(-Vector3D_dotLong(pos, nor) / dot);
    if (dot < -1 || (dot > 4096 && !ray->infinity)) return INT64_MAX;
    pos->x = start->x + (dir->x * dot >> 12);
    pos->y = start->y + (dir->y * dot >> 12);
    pos->z = start->z + (dir->z * dot >> 12);
    if (MathUtils2_isPointOnPolygon4v(pos, a, b, c, d, nor)) return dot;
    return INT64_MAX;
}

void RayCast_superFastRayCast(Mesh *mesh, Ray *ray) {
    RenderObject **elements = mesh->polygons;
    int elems_n = mesh->polygons_n;
    Vector3D *start = &ray->start;
    Vector3D *dir = &ray->dir;

    int dirLen = Vector3D_length(dir);
    for (int i = 0; i < elems_n; i++) {
        RenderObject *poly = elements[i];
        int64_t dis = INT64_MAX;
        int mat = -1;

        if (poly->vt == Polygon4V_vt()) {
            Polygon4V *p = (Polygon4V *)poly;

            s_normal.x = p->base.nx; s_normal.y = p->base.ny; s_normal.z = p->base.nz;

            s_v1.x = p->a->x; s_v1.y = p->a->y; s_v1.z = p->a->z;
            s_v2.x = p->b->x; s_v2.y = p->b->y; s_v2.z = p->b->z;
            s_v3.x = p->c->x; s_v3.y = p->c->y; s_v3.z = p->c->z;
            s_v4.x = p->d->x; s_v4.y = p->d->y; s_v4.z = p->d->z;
            dis = rayTracing4(ray, &s_v1, &s_v2, &s_v3, &s_v4, &s_normal, start, dir, &s_colPoint, &s_v1);
            mat = p->tex;
        } else if (poly->vt == Polygon3V_vt()) {
            Polygon3V *p = (Polygon3V *)poly;

            s_normal.x = p->base.nx; s_normal.y = p->base.ny; s_normal.z = p->base.nz;

            s_v1.x = p->a->x; s_v1.y = p->a->y; s_v1.z = p->a->z;
            s_v2.x = p->b->x; s_v2.y = p->b->y; s_v2.z = p->b->z;
            s_v3.x = p->c->x; s_v3.y = p->c->y; s_v3.z = p->c->z;
            dis = rayTracing3(ray, &s_v1, &s_v2, &s_v3, &s_normal, start, dir, &s_colPoint, &s_v1);
            mat = p->tex;
        }

        if (mat != -1) {
            Texture *tex = mesh->texture->textures[mat];
            if (ray->ignoreNonShadowed && !tex->castShadow) continue;
            if (ray->onlyCollidable && !tex->collision) continue;
        }

        if (dis != INT64_MAX && ray->origPol != poly && dis >= 0) {
            int64_t distance = (int64_t)dirLen * dis >> 12;
            if (distance > INT32_MAX) distance = INT32_MAX;
            if (distance < ray->distance) {
                ray->collision = true;
                ray->distance = (int)distance;
                ray->collisionPoint.x = s_colPoint.x;
                ray->collisionPoint.y = s_colPoint.y;
                ray->collisionPoint.z = s_colPoint.z;
                ray->triangle = poly;
                if (!ray->findNearest) return;
            }
        }
    }
}

void RayCast_rayCastRecalc(Mesh *mesh, Ray *ray, bool reCalcNorm) {
    RenderObject **elements = mesh->polygons;
    int elems_n = mesh->polygons_n;
    Vector3D *start = &ray->start;
    Vector3D *dir = &ray->dir;

    const int sx = start->x, sy = start->y, sz = start->z;
    const int ex = sx + dir->x, ey = sy + dir->y, ez = sz + dir->z;

    const int x1 = rc_min2(sx, ex);
    const int y1 = rc_min2(sy, ey);
    const int z1 = rc_min2(sz, ez);

    const int x2 = rc_max2(sx, ex);
    const int y2 = rc_max2(sy, ey);
    const int z2 = rc_max2(sz, ez);

    int dirLen = Vector3D_length(dir);
    for (int i = 0; i < elems_n; i++) {
        RenderObject *poly = elements[i];

        int64_t dis = INT64_MAX;
        int mat = -1;

        if (poly->vt == Polygon3V_vt()) {
            Polygon3V *p = (Polygon3V *)poly;
            Vertex *a = p->a, *b = p->b, *c = p->c;
            if (!ray->infinity) {
                if (rc_max3(a->x, b->x, c->x) < x1) continue;
                if (rc_min3(a->x, b->x, c->x) > x2) continue;
                if (rc_max3(a->z, b->z, c->z) < z1) continue;
                if (rc_min3(a->z, b->z, c->z) > z2) continue;
                if (rc_max3(a->y, b->y, c->y) < y1) continue;
                if (rc_min3(a->y, b->y, c->y) > y2) continue;
            }

            if (!reCalcNorm) {
                s_normal.x = p->base.nx; s_normal.y = p->base.ny; s_normal.z = p->base.nz;
            } else {
                int64_t xx = (int64_t)(a->y - b->y) * (a->z - c->z) - (int64_t)(a->z - b->z) * (a->y - c->y);
                int64_t yy = (int64_t)(a->z - b->z) * (a->x - c->x) - (int64_t)(a->x - b->x) * (a->z - c->z);
                int64_t zz = (int64_t)(a->x - b->x) * (a->y - c->y) - (int64_t)(a->y - b->y) * (a->x - c->x);
                double sq = sqrt((double)(xx * xx + yy * yy + zz * zz)) / 4096.0;
                s_normal.x = (int)(xx / sq);
                s_normal.y = (int)(yy / sq);
                s_normal.z = (int)(zz / sq);
            }

            s_v1.x = a->x; s_v1.y = a->y; s_v1.z = a->z;
            s_v2.x = b->x; s_v2.y = b->y; s_v2.z = b->z;
            s_v3.x = c->x; s_v3.y = c->y; s_v3.z = c->z;
            dis = rayTracing3(ray, &s_v1, &s_v2, &s_v3, &s_normal, start, dir, &s_colPoint, &s_v1);
            mat = p->tex;
        } else if (poly->vt == Polygon4V_vt()) {
            Polygon4V *p = (Polygon4V *)poly;
            Vertex *a = p->a, *b = p->b, *c = p->c, *d = p->d;
            if (!ray->infinity) {
                if (rc_max4(a->x, b->x, c->x, d->x) < x1) continue;
                if (rc_min4(a->x, b->x, c->x, d->x) > x2) continue;
                if (rc_max4(a->z, b->z, c->z, d->z) < z1) continue;
                if (rc_min4(a->z, b->z, c->z, d->z) > z2) continue;
                if (rc_max4(a->y, b->y, c->y, d->y) < y1) continue;
                if (rc_min4(a->y, b->y, c->y, d->y) > y2) continue;
            }

            if (!reCalcNorm) {
                s_normal.x = p->base.nx; s_normal.y = p->base.ny; s_normal.z = p->base.nz;
            } else {
                int64_t xx = (int64_t)(a->y - b->y) * (a->z - d->z) - (int64_t)(a->z - b->z) * (a->y - d->y);
                int64_t yy = (int64_t)(a->z - b->z) * (a->x - d->x) - (int64_t)(a->x - b->x) * (a->z - d->z);
                int64_t zz = (int64_t)(a->x - b->x) * (a->y - d->y) - (int64_t)(a->y - b->y) * (a->x - d->x);
                double sq = sqrt((double)(xx * xx + yy * yy + zz * zz)) / 4096.0;
                s_normal.x = (int)(xx / sq);
                s_normal.y = (int)(yy / sq);
                s_normal.z = (int)(zz / sq);
            }

            s_v1.x = a->x; s_v1.y = a->y; s_v1.z = a->z;
            s_v2.x = b->x; s_v2.y = b->y; s_v2.z = b->z;
            s_v3.x = c->x; s_v3.y = c->y; s_v3.z = c->z;
            s_v4.x = d->x; s_v4.y = d->y; s_v4.z = d->z;
            dis = rayTracing4(ray, &s_v1, &s_v2, &s_v3, &s_v4, &s_normal, start, dir, &s_colPoint, &s_v1);
            mat = p->tex;
        }

        if (mat != -1) {
            Texture *tex = mesh->texture->textures[mat];
            if (ray->ignoreNonShadowed && !tex->castShadow) continue;
            if (ray->onlyCollidable && !tex->collision) continue;
        }

        if (dis != INT64_MAX && ray->origPol != poly && dis >= 0) {
            int64_t distance = (int64_t)dirLen * dis >> 12;
            if (distance > INT32_MAX) distance = INT32_MAX;
            if (distance < ray->distance) {
                ray->collision = true;
                ray->distance = (int)distance;
                ray->collisionPoint.x = s_colPoint.x;
                ray->collisionPoint.y = s_colPoint.y;
                ray->collisionPoint.z = s_colPoint.z;
                ray->triangle = poly;
                if (!ray->findNearest) return;
            }
        }
    }
}

int64_t RayCast_isRayOnPolygon4(Vector3D *a, Vector3D *b, Vector3D *c, Vector3D *d,
                                Vector3D *nor, Vector3D *start, Vector3D *dir) {
    s_temp.x = start->x - a->x;
    s_temp.y = start->y - a->y;
    s_temp.z = start->z - a->z;
    int dot = (int)(Vector3D_dotLong(dir, nor) >> 12);
    if (dot <= 0) return INT64_MAX;
    dot = (int)(-Vector3D_dotLong(&s_temp, nor) / dot);
    if (dot < -1) return INT64_MAX;
    s_temp.x = start->x + (dir->x * dot >> 12);
    s_temp.y = start->y + (dir->y * dot >> 12);
    s_temp.z = start->z + (dir->z * dot >> 12);
    if (MathUtils2_isPointOnPolygon4v(&s_temp, a, b, c, d, nor))
        return (int64_t)dot * Vector3D_length(dir) >> 12;
    return INT64_MAX;
}

int64_t RayCast_isRayOnPolygon3(Vector3D *a, Vector3D *b, Vector3D *c,
                                Vector3D *nor, Vector3D *start, Vector3D *dir) {
    s_temp.x = start->x - a->x;
    s_temp.y = start->y - a->y;
    s_temp.z = start->z - a->z;
    int dot = (int)(Vector3D_dotLong(dir, nor) >> 12);
    if (dot <= 0) return INT64_MAX;
    dot = (int)(-Vector3D_dotLong(&s_temp, nor) / dot);
    if (dot < -1) return INT64_MAX;
    s_temp.x = start->x + (dir->x * dot >> 12);
    s_temp.y = start->y + (dir->y * dot >> 12);
    s_temp.z = start->z + (dir->z * dot >> 12);
    if (MathUtils2_isPointOnPolygon3v(&s_temp, a, b, c, nor))
        return (int64_t)dot * Vector3D_length(dir) >> 12;
    return INT64_MAX;
}

void RayCast_rayCastMatrix(Mesh *mesh, Ray *ray, Matrix *mat) {
    DirectX7_transformSave(mesh->vertices, mesh->vertices_n, mat);
    RayCast_rayCastRecalc(mesh, ray, true);
    DirectX7_transformReturn(mesh->vertices, mesh->vertices_n);
}

void RayCast_rayCastRoom(Mesh *mesh, Ray *ray, int rom) {
    s_roomNeed = true;
    s_room = rom;
    RayCast_rayCast(mesh, ray);
    s_roomNeed = false;
}

void RayCast_rayCast(Mesh *mesh, Ray *ray) {
    RayCast_rayCastRecalc(mesh, ray, false);
}
