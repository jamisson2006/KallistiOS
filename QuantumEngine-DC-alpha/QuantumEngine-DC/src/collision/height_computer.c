/*
 * height_computer.c — porte fiel de code/Collision/HeightComputer.java
 */
#include "height_computer.h"
#include <stddef.h>
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

static Height s_height;

bool HeightComputer_isPointAABBCollision(int x, int z, int minx, int maxx, int minz, int maxz) {
    return !(x < minx || z < minz || x > maxx || z > maxz);
}

int HeightComputer_computeHeightSimple(Mesh *mesh, int x, int y, int z) {
    Height_reset(&s_height);
    Height_getPosition(&s_height)->x = x;
    Height_getPosition(&s_height)->y = y;
    Height_getPosition(&s_height)->z = z;
    HeightComputer_computeHeight(mesh, &s_height);
    return Height_getHeight(&s_height);
}

void HeightComputer_computeHeightMatrix(Mesh *mesh, Matrix *matrix, Height *height) {
    HeightComputer_computeHeightMatrixFull(mesh, matrix, height, true);
}

void HeightComputer_computeHeight(Mesh *mesh, Height *height) {
    HeightComputer_computeHeightMatrixFull(mesh, NULL, height, true);
}

void HeightComputer_computeHeightMatrixFull(Mesh *mesh, Matrix *matrix, Height *height, bool updatePos) {
    if (matrix != NULL) {
        DirectX7_transformSave(mesh->vertices, mesh->vertices_n, matrix);
    }

    RenderObject **polygons = mesh->polygons;
    int poly_n = mesh->polygons_n;
    Vector3D *pos = Height_getPosition(height);
    const int x = pos->x, y = pos->y, z = pos->z;

    for (int i = 0; i < poly_n; i++) {
        RenderObject *el = polygons[i];
        int polyY = INT32_MAX;
        int cx = INT32_MAX, cz = INT32_MAX, cy = INT32_MAX;
        int mat = -1;

        if (el->vt == Polygon4V_vt()) {
            Polygon4V *p = (Polygon4V *)el;
            Vertex *a = p->a;
            Vertex *b = p->b;
            Vertex *c = p->c;
            Vertex *d = p->d;
            mat = p->tex;

            const int ax = a->x, az = a->z;
            const int bx = b->x, bz = b->z;
            const int dx = d->x, dz = d->z;

            int maxx = ax;
            if (bx > maxx) maxx = bx;
            if (c->x > maxx) maxx = c->x;
            if (dx > maxx) maxx = dx;

            int minx = ax;
            if (bx < minx) minx = bx;
            if (c->x < minx) minx = c->x;
            if (dx < minx) minx = dx;

            int maxz = az;
            if (bz > maxz) maxz = bz;
            if (c->z > maxz) maxz = c->z;
            if (dz > maxz) maxz = dz;

            int minz = az;
            if (bz < minz) minz = bz;
            if (c->z < minz) minz = c->z;
            if (dz < minz) minz = dz;

            if (minx > x) continue;
            if (minz > z) continue;
            if (maxx < x) continue;
            if (maxz < z) continue;

            if (el->ny >= 0) {
                if (el->ny > 2048 && ((a->y + b->y + c->y + d->y) >> 2) > y)
                    Height_setUnderRoof(height, true);
                continue;
            }

            if (MathUtils2_isPointOnPolygon2D_4norY(x, z, ax, az, bx, bz,
                    c->x, c->z, dx, dz, p->base.ny)) {
                polyY = (y - ((x - ax) * p->base.nx + (y - a->y) * p->base.ny + (z - az) * p->base.nz) / p->base.ny);
                cx = (ax + bx + c->x + dx) >> 2;
                cz = (az + bz + c->z + dz) >> 2;
                cy = (a->y + b->y + c->y + d->y) >> 2;
            }
        } else if (el->vt == Polygon3V_vt()) {
            Polygon3V *p = (Polygon3V *)el;
            Vertex *a = p->a;
            Vertex *b = p->b;
            Vertex *c = p->c;
            mat = p->tex;

            const int ax = a->x, az = a->z;
            const int bx = b->x, bz = b->z;

            int maxx = ax;
            if (bx > maxx) maxx = bx;
            if (c->x > maxx) maxx = c->x;

            int minx = ax;
            if (bx < minx) minx = bx;
            if (c->x < minx) minx = c->x;

            int maxz = az;
            if (bz > maxz) maxz = bz;
            if (c->z > maxz) maxz = c->z;

            int minz = az;
            if (bz < minz) minz = bz;
            if (c->z < minz) minz = c->z;

            if (minx > x) continue;
            if (minz > z) continue;
            if (maxx < x) continue;
            if (maxz < z) continue;

            if (el->ny >= 0) {
                if (el->ny > 2048 && ((a->y + b->y + c->y) / 3) > y)
                    Height_setUnderRoof(height, true);
                continue;
            }

            if (MathUtils2_isPointOnPolygon2D_3norY(x, z, a->x, a->z,
                    b->x, b->z, c->x, c->z, p->base.ny)) {
                polyY = (y - ((x - ax) * p->base.nx + (y - a->y) * p->base.ny + (z - az) * p->base.nz) / p->base.ny);
                cx = (a->x + b->x + c->x) / 3;
                cz = (a->z + b->z + c->z) / 3;
                cy = (a->y + b->y + c->y) / 3;
            }
        }

        if (mat != -1) {
            Texture *tex = mesh->texture->textures[mat];
            if (!tex->collision) continue;
        }

        if (polyY < y && polyY > Height_getHeight(height)) {
            Height_set(height, polyY, el, cx, cz, cy, matrix, updatePos);
        }
    }
}
