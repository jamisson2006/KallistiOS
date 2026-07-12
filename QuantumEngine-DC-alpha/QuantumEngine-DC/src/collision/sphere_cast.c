/*
 * sphere_cast.c — porte fiel de code/Collision/SphereCast.java
 */
#include "sphere_cast.h"
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

static Vector3D s_temp;
static Vector3D s_nor;
static Vector3D s_v1, s_v2, s_v3, s_v4;

bool SphereCast_isSphereAABBCollision(Vector3D *pos, int rad, int minx, int maxx, int minz, int maxz) {
    return !(pos->x + rad < minx || pos->z + rad < minz || pos->x - rad > maxx || pos->z - rad > maxz);
}

static int distanceSphereToPolygon3(Vector3D *a, Vector3D *b, Vector3D *c, Vector3D *nor,
                                    Vector3D *point, int rad) {
    s_temp.x = point->x - a->x;
    s_temp.y = point->y - a->y;
    s_temp.z = point->z - a->z;
    int dot = Vector3D_dot(&s_temp, nor) >> 12;
    if (dot > rad) return INT32_MAX;
    s_temp.x = point->x - (nor->x * dot >> 12);
    s_temp.y = point->y - (nor->y * dot >> 12);
    s_temp.z = point->z - (nor->z * dot >> 12);
    if (MathUtils2_isPointOnPolygon3v(&s_temp, a, b, c, nor)) {
        int dis = dot;
        if (dot < 0) dis = -dot;
        dis = rad - dis;
        return dis;
    }

    const int len1 = MathUtils2_distanceToLine(point, a, b);
    const int len2 = MathUtils2_distanceToLine(point, b, c);
    const int len3 = MathUtils2_distanceToLine(point, c, a);

    int min = len1;
    if (len2 < min) min = len2;
    if (len3 < min) min = len3;
    if (min <= rad * rad) return rad - (int)(1.0f / MathUtils2_invSqrt((float)min));
    return INT32_MAX;
}

static int distanceSphereToPolygon4(Vector3D *a, Vector3D *b, Vector3D *c, Vector3D *d,
                                    Vector3D *nor, Vector3D *point, int rad) {
    s_temp.x = point->x - a->x;
    s_temp.y = point->y - a->y;
    s_temp.z = point->z - a->z;
    int dot = Vector3D_dot(&s_temp, nor) >> 12;
    if (dot > rad) return INT32_MAX;
    s_temp.x = point->x - (nor->x * dot >> 12);
    s_temp.y = point->y - (nor->y * dot >> 12);
    s_temp.z = point->z - (nor->z * dot >> 12);
    if (MathUtils2_isPointOnPolygon4v(&s_temp, a, b, c, d, nor)) {
        int dis = dot;
        if (dot < 0) dis = -dot;
        dis = rad - dis;
        return dis;
    }

    const int len1 = MathUtils2_distanceToLine(point, a, b);
    const int len2 = MathUtils2_distanceToLine(point, b, c);
    const int len3 = MathUtils2_distanceToLine(point, c, d);
    const int len4 = MathUtils2_distanceToLine(point, d, a);

    int min = len1;
    if (len2 < min) min = len2;
    if (len3 < min) min = len3;
    if (len4 < min) min = len4;
    if (min <= rad * rad) return rad - (int)(1.0f / MathUtils2_invSqrt((float)min));
    return INT32_MAX;
}

bool SphereCast_sphereCastMatrix(Mesh *mesh, Matrix *matrix, Vector3D *pos, int rad) {
    DirectX7_transformSave(mesh->vertices, mesh->vertices_n, matrix);
    return SphereCast_sphereCastFull(mesh, pos, rad, true);
}

bool SphereCast_sphereCast(Mesh *mesh, Vector3D *pos, int rad) {
    return SphereCast_sphereCastFull(mesh, pos, rad, false);
}

bool SphereCast_sphereCastFull(Mesh *mesh, Vector3D *pos, int rad, bool recalc) {
    RenderObject **elements = mesh->polygons;
    int elems_n = mesh->polygons_n;
    bool col = false;

    for (int i = 0; i < elems_n; i++) {
        RenderObject *poly = elements[i];
        int dis = INT32_MAX;
        int mat = -1;

        if (poly->vt == Polygon4V_vt()) {
            Polygon4V *p = (Polygon4V *)poly;
            Vertex *a = p->a;
            Vertex *b = p->b;
            Vertex *c = p->c;
            Vertex *d = p->d;
            mat = p->tex;

            const int ax = a->x, ay = a->y, az = a->z;
            const int bx = b->x, by = b->y, bz = b->z;
            const int cx = c->x, cy = c->y, cz = c->z;
            const int dx = d->x, dy = d->y, dz = d->z;

            int maxx = ax;
            if (bx > maxx) maxx = bx;
            if (cx > maxx) maxx = cx;
            if (dx > maxx) maxx = dx;

            int minx = ax;
            if (bx < minx) minx = bx;
            if (cx < minx) minx = cx;
            if (dx < minx) minx = dx;

            int maxy = ay;
            if (by > maxy) maxy = by;
            if (cy > maxy) maxy = cy;
            if (dy > maxy) maxy = dy;

            int miny = ay;
            if (by < miny) miny = by;
            if (cy < miny) miny = cy;
            if (dy < miny) miny = dy;

            int maxz = az;
            if (bz > maxz) maxz = bz;
            if (cz > maxz) maxz = cz;
            if (dz > maxz) maxz = dz;

            int minz = az;
            if (bz < minz) minz = bz;
            if (cz < minz) minz = cz;
            if (dz < minz) minz = dz;

            if (maxx < pos->x - rad) continue;
            if (minx > pos->x + rad) continue;
            if (maxz < pos->z - rad) continue;
            if (minz > pos->z + rad) continue;
            if (maxy < pos->y - rad) continue;
            if (miny > pos->y + rad) continue;

            s_v1.x = ax; s_v1.y = ay; s_v1.z = az;
            s_v2.x = bx; s_v2.y = by; s_v2.z = bz;
            s_v3.x = cx; s_v3.y = cy; s_v3.z = cz;
            s_v4.x = dx; s_v4.y = dy; s_v4.z = dz;
            s_nor.x = p->base.nx;
            s_nor.y = p->base.ny;
            s_nor.z = p->base.nz;
            if (recalc) MathUtils2_calcNormalV(&s_nor, &s_v1, &s_v2, &s_v4);
            dis = distanceSphereToPolygon4(&s_v1, &s_v2, &s_v3, &s_v4, &s_nor, pos, rad);
        } else if (poly->vt == Polygon3V_vt()) {
            Polygon3V *p = (Polygon3V *)poly;
            Vertex *a = p->a;
            Vertex *b = p->b;
            Vertex *c = p->c;
            mat = p->tex;

            const int ax = a->x, ay = a->y, az = a->z;
            const int bx = b->x, by = b->y, bz = b->z;
            const int cx = c->x, cy = c->y, cz = c->z;

            int maxx = ax;
            if (bx > maxx) maxx = bx;
            if (cx > maxx) maxx = cx;

            int minx = ax;
            if (bx < minx) minx = bx;
            if (cx < minx) minx = cx;

            int maxy = ay;
            if (by > maxy) maxy = by;
            if (cy > maxy) maxy = cy;

            int miny = ay;
            if (by < miny) miny = by;
            if (cy < miny) miny = cy;

            int maxz = az;
            if (bz > maxz) maxz = bz;
            if (cz > maxz) maxz = cz;

            int minz = az;
            if (bz < minz) minz = bz;
            if (cz < minz) minz = cz;

            if (maxx < pos->x - rad) continue;
            if (minx > pos->x + rad) continue;
            if (maxz < pos->z - rad) continue;
            if (minz > pos->z + rad) continue;
            if (maxy < pos->y - rad) continue;
            if (miny > pos->y + rad) continue;

            s_v1.x = ax; s_v1.y = ay; s_v1.z = az;
            s_v2.x = bx; s_v2.y = by; s_v2.z = bz;
            s_v3.x = cx; s_v3.y = cy; s_v3.z = cz;
            s_nor.x = p->base.nx;
            s_nor.y = p->base.ny;
            s_nor.z = p->base.nz;
            if (recalc) MathUtils2_calcNormalV(&s_nor, &s_v1, &s_v2, &s_v3);
            dis = distanceSphereToPolygon3(&s_v1, &s_v2, &s_v3, &s_nor, pos, rad);
        }

        if (mat != -1) {
            Texture *tex = mesh->texture->textures[mat];
            if (!tex->collision) continue;
        }

        if (dis != INT32_MAX && dis > 0) {
            pos->x += (-s_nor.x * dis >> 12);
            pos->y += (-s_nor.y * dis >> 12);
            pos->z += (-s_nor.z * dis >> 12);
            col = true;
        }
    }
    return col;
}
