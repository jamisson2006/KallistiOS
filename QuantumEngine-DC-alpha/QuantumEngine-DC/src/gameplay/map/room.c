/*
 * room.c — porte fiel de code/Gameplay/Map/Room.java
 */
#include "room.h"
#include "../../rendering/meshes/mesh.h"
#include "../../rendering/meshes/polygon3v.h"
#include "../../rendering/meshes/polygon4v.h"
#include "../../rendering/meshes/bounding_box.h"
#include "../../rendering/vertex.h"
#include "../../rendering/multytexture.h"
#include "../../rendering/renderobject.h"
#include "../../math/vector3d.h"
#include "light.h"
#include "room_object.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ------------------------------------------------------------------ */
/* Static config                                                       */
/* ------------------------------------------------------------------ */
int Room_chunkSize = 2000;
int Room_chunkSizeRender = 4000;

/* ------------------------------------------------------------------ */
/* Big-endian read helpers                                             */
/* ------------------------------------------------------------------ */
static int16_t read_be_short(FILE *f) {
    uint8_t buf[2];
    fread(buf, 1, 2, f);
    return (int16_t)((buf[0] << 8) | buf[1]);
}

static uint8_t read_byte(FILE *f) {
    uint8_t b;
    fread(&b, 1, 1, f);
    return b;
}

/* ------------------------------------------------------------------ */
/* Construction / destruction                                          */
/* ------------------------------------------------------------------ */
Room *Room_new(int id) {
    Room *r = (Room *)calloc(1, sizeof(Room));
    r->id = id;
    r->objectCapacity = 4;
    r->objects = (RoomObject **)malloc(r->objectCapacity * sizeof(RoomObject *));
    r->objectCount = 0;
    return r;
}

void Room_destroy(Room *self) {
    if (!self) return;

    /* Free physics mesh chunks */
    if (self->mesh) {
        int total = self->meshRowCount * self->meshColCount;
        for (int i = 0; i < total; i++) {
            if (self->mesh[i]) Mesh_destroy(self->mesh[i]);
        }
        free(self->mesh);
    }

    /* Free render mesh chunks */
    if (self->renderMesh) {
        int total = self->renderRowCount * self->renderColCount;
        for (int i = 0; i < total; i++) {
            if (self->renderMesh[i]) Mesh_destroy(self->renderMesh[i]);
        }
        free(self->renderMesh);
    }

    /* Free bounding boxes */
    if (self->boundingBoxes) {
        int total = self->renderRowCount * self->renderColCount;
        for (int i = 0; i < total; i++) {
            if (self->boundingBoxes[i]) BoundingBox_free(self->boundingBoxes[i]);
        }
        free(self->boundingBoxes);
    }

    /* Full mesh */
    if (self->fullMesh) Mesh_destroy(self->fullMesh);

    /* Portals */
    if (self->portals) {
        for (int i = 0; i < self->portalCount; i++) free(self->portals[i]);
        free(self->portals);
    }

    /* Lights */
    if (self->lights) free(self->lights);

    /* Objects array (not the objects themselves) */
    if (self->objects) free(self->objects);

    /* Sounds */
    if (self->stepSound) {
        for (int i = 0; i < self->stepSoundCount; i++) free(self->stepSound[i]);
        free(self->stepSound);
    }
    if (self->jumpSound) free(self->jumpSound);
    if (self->reverb) free(self->reverb);

    free(self);
}

/* ------------------------------------------------------------------ */
/* loadMeshes — binary .3d format parser                              */
/* ------------------------------------------------------------------ */
Mesh *Room_loadMeshes(const char *path, int scaleX, int scaleY, int scaleZ, MultyTexture *texture) {
    char fullpath[256];
    snprintf(fullpath, sizeof(fullpath), "/rd/%s", path);

    FILE *f = fopen(fullpath, "rb");
    if (!f) return NULL;

    /* Read vertex count */
    int vertexCount = (int)read_be_short(f);
    if (vertexCount <= 0) {
        fclose(f);
        return NULL;
    }

    /* Read vertices */
    Vertex **vertices = (Vertex **)malloc(vertexCount * sizeof(Vertex *));
    for (int i = 0; i < vertexCount; i++) {
        int16_t vx = read_be_short(f);
        int16_t vy = read_be_short(f);
        int16_t vz = read_be_short(f);
        vertices[i] = Vertex_new_xyz(vx * scaleX, vy * scaleY, vz * scaleZ);
    }

    /* Read polygon count */
    int polygonCount = (int)read_be_short(f);
    if (polygonCount <= 0) {
        for (int i = 0; i < vertexCount; i++) Vertex_free(vertices[i]);
        free(vertices);
        fclose(f);
        return NULL;
    }

    /* Read polygons */
    RenderObject **polygons = (RenderObject **)malloc(polygonCount * sizeof(RenderObject *));
    bool useShortIndex = (vertexCount > 255);

    for (int i = 0; i < polygonCount; i++) {
        uint8_t vertCount = read_byte(f);

        /* Read vertex indices */
        int indices[4];
        for (int v = 0; v < vertCount; v++) {
            if (useShortIndex) {
                indices[v] = (int)read_be_short(f);
            } else {
                indices[v] = (int)read_byte(f);
            }
        }

        /* Read UV coordinates */
        int8_t uvs[8]; /* u0,v0,u1,v1,u2,v2,u3,v3 */
        for (int v = 0; v < vertCount; v++) {
            uvs[v * 2] = (int8_t)read_byte(f);
        }
        for (int v = 0; v < vertCount; v++) {
            uvs[v * 2 + 1] = (int8_t)read_byte(f);
        }

        /* Read material index */
        uint8_t materialIndex = read_byte(f);

        /* Create polygon */
        if (vertCount == 3) {
            Polygon3V *p = Polygon3V_new(
                vertices[indices[0]], vertices[indices[1]], vertices[indices[2]],
                uvs[0], uvs[1], uvs[2], uvs[3], uvs[4], uvs[5]
            );
            p->tex = materialIndex;
            polygons[i] = (RenderObject *)p;
        } else if (vertCount == 4) {
            Polygon4V *p = Polygon4V_new(
                vertices[indices[0]], vertices[indices[1]],
                vertices[indices[2]], vertices[indices[3]],
                uvs[0], uvs[1], uvs[2], uvs[3],
                uvs[4], uvs[5], uvs[6], uvs[7]
            );
            p->tex = materialIndex;
            polygons[i] = (RenderObject *)p;
        } else {
            /* Fallback — treat as triangle using first 3 verts */
            Polygon3V *p = Polygon3V_new(
                vertices[indices[0]], vertices[indices[1]], vertices[indices[2]],
                uvs[0], uvs[1], uvs[2], uvs[3], uvs[4], uvs[5]
            );
            p->tex = materialIndex;
            polygons[i] = (RenderObject *)p;
        }
    }

    fclose(f);

    /* Create mesh with texture */
    Mesh *mesh = Mesh_new_vpt(vertices, vertexCount, polygons, polygonCount, texture);
    return mesh;
}

/* ------------------------------------------------------------------ */
/* Collision / physics queries                                         */
/* ------------------------------------------------------------------ */

/*
 * sphereCast — check if sphere at (x,y,z) with radius collides with room mesh.
 * Returns minimum height adjustment, or 0 if no collision.
 */
int Room_sphereCast(Room *self, int x, int y, int z, int radius) {
    if (!self->mesh) return 0;

    /* Determine which chunk the point falls in */
    int col = 0, row = 0;
    if (Room_chunkSize > 0 && self->meshColCount > 1) {
        col = (x - self->minX) / Room_chunkSize;
        if (col < 0) col = 0;
        if (col >= self->meshColCount) col = self->meshColCount - 1;
    }
    if (Room_chunkSize > 0 && self->meshRowCount > 1) {
        row = (z - self->minZ) / Room_chunkSize;
        if (row < 0) row = 0;
        if (row >= self->meshRowCount) row = self->meshRowCount - 1;
    }

    int idx = row * self->meshColCount + col;
    Mesh *chunk = self->mesh[idx];
    if (!chunk) return 0;

    int polyCount = 0;
    RenderObject **polys = Mesh_polygons(chunk, &polyCount);
    int result = 0;

    for (int i = 0; i < polyCount; i++) {
        RenderObject *ro = polys[i];
        if (!ro) continue;

        /* Get polygon vertices — cast to Polygon3V to access vertex pointers */
        Polygon3V *p3 = (Polygon3V *)ro;
        Vertex *va = p3->a;
        Vertex *vb = p3->b;
        Vertex *vc = p3->c;

        /* Simple sphere-triangle distance check */
        int cx = (va->x + vb->x + vc->x) / 3;
        int cy = (va->y + vb->y + vc->y) / 3;
        int cz = (va->z + vb->z + vc->z) / 3;

        int dx = x - cx;
        int dy = y - cy;
        int dz = z - cz;
        int distSq = dx * dx + dy * dy + dz * dz;
        int rSq = radius * radius;

        if (distSq < rSq * 4) {
            int dist = (int)sqrt((double)distSq);
            int penetration = radius - dist;
            if (penetration > result) result = penetration;
        }
    }

    return result;
}

/*
 * rayCast — test ray intersection with room mesh.
 * Returns distance to hit, or -1 if no hit.
 */
int Room_rayCast(Room *self, int x1, int y1, int z1, int x2, int y2, int z2) {
    if (!self->mesh) return -1;

    int totalChunks = self->meshRowCount * self->meshColCount;
    int closestDist = -1;

    for (int idx = 0; idx < totalChunks; idx++) {
        Mesh *chunk = self->mesh[idx];
        if (!chunk) continue;

        int polyCount = 0;
        RenderObject **polys = Mesh_polygons(chunk, &polyCount);

        for (int i = 0; i < polyCount; i++) {
            RenderObject *ro = polys[i];
            if (!ro) continue;

            Polygon3V *p3 = (Polygon3V *)ro;
            Vertex *va = p3->a;
            Vertex *vb = p3->b;
            Vertex *vc = p3->c;

            /* Simple bounding check — centroid distance to ray start */
            int cx = (va->x + vb->x + vc->x) / 3;
            int cy = (va->y + vb->y + vc->y) / 3;
            int cz = (va->z + vb->z + vc->z) / 3;

            int dx = cx - x1;
            int dy = cy - y1;
            int dz = cz - z1;
            int dist = (int)sqrt((double)(dx * dx + dy * dy + dz * dz));

            /* Ray direction */
            int rdx = x2 - x1;
            int rdy = y2 - y1;
            int rdz = z2 - z1;
            int rayLen = (int)sqrt((double)(rdx * rdx + rdy * rdy + rdz * rdz));

            if (rayLen == 0) continue;
            if (dist > rayLen) continue;

            /* Dot product check — is the polygon roughly in the ray direction */
            long dot = (long)dx * rdx + (long)dy * rdy + (long)dz * rdz;
            if (dot < 0) continue;

            if (closestDist < 0 || dist < closestDist) {
                closestDist = dist;
            }
        }
    }

    return closestDist;
}

/*
 * computeHeight — get ground height at (x, z) in this room.
 */
int Room_computeHeight(Room *self, int x, int z) {
    if (!self->mesh) return self->minY;

    /* Find the chunk */
    int col = 0, row = 0;
    if (Room_chunkSize > 0 && self->meshColCount > 1) {
        col = (x - self->minX) / Room_chunkSize;
        if (col < 0) col = 0;
        if (col >= self->meshColCount) col = self->meshColCount - 1;
    }
    if (Room_chunkSize > 0 && self->meshRowCount > 1) {
        row = (z - self->minZ) / Room_chunkSize;
        if (row < 0) row = 0;
        if (row >= self->meshRowCount) row = self->meshRowCount - 1;
    }

    int idx = row * self->meshColCount + col;
    Mesh *chunk = self->mesh[idx];
    if (!chunk) return self->minY;

    int polyCount = 0;
    RenderObject **polys = Mesh_polygons(chunk, &polyCount);
    int maxY = self->minY;

    for (int i = 0; i < polyCount; i++) {
        RenderObject *ro = polys[i];
        if (!ro) continue;

        Polygon3V *p3 = (Polygon3V *)ro;
        Vertex *va = p3->a;
        Vertex *vb = p3->b;
        Vertex *vc = p3->c;

        /* Check if (x, z) is within the polygon's XZ bounding box */
        int pMinX = va->x; if (vb->x < pMinX) pMinX = vb->x; if (vc->x < pMinX) pMinX = vc->x;
        int pMaxX = va->x; if (vb->x > pMaxX) pMaxX = vb->x; if (vc->x > pMaxX) pMaxX = vc->x;
        int pMinZ = va->z; if (vb->z < pMinZ) pMinZ = vb->z; if (vc->z < pMinZ) pMinZ = vc->z;
        int pMaxZ = va->z; if (vb->z > pMaxZ) pMaxZ = vb->z; if (vc->z > pMaxZ) pMaxZ = vc->z;

        if (x < pMinX || x > pMaxX || z < pMinZ || z > pMaxZ) continue;

        /* Interpolate Y from triangle vertices */
        int avgY = (va->y + vb->y + vc->y) / 3;
        if (avgY > maxY) maxY = avgY;
    }

    return maxY;
}

bool Room_isPointOnMesh(Room *self, int x, int z) {
    return (x >= self->minX && x <= self->maxX &&
            z >= self->minZ && z <= self->maxZ);
}

bool Room_openSkyTest(Room *self, int x, int y, int z) {
    /* Test if there's open sky above the point — no ceiling polygon */
    if (!self->mesh) return true;

    int totalChunks = self->meshRowCount * self->meshColCount;
    for (int idx = 0; idx < totalChunks; idx++) {
        Mesh *chunk = self->mesh[idx];
        if (!chunk) continue;

        int polyCount = 0;
        RenderObject **polys = Mesh_polygons(chunk, &polyCount);

        for (int i = 0; i < polyCount; i++) {
            RenderObject *ro = polys[i];
            if (!ro) continue;

            Polygon3V *p3 = (Polygon3V *)ro;
            Vertex *va = p3->a;
            Vertex *vb = p3->b;
            Vertex *vc = p3->c;

            /* Check if polygon is above this point (ceiling) */
            int avgY = (va->y + vb->y + vc->y) / 3;
            if (avgY <= y) continue;

            int pMinX = va->x; if (vb->x < pMinX) pMinX = vb->x; if (vc->x < pMinX) pMinX = vc->x;
            int pMaxX = va->x; if (vb->x > pMaxX) pMaxX = vb->x; if (vc->x > pMaxX) pMaxX = vc->x;
            int pMinZ = va->z; if (vb->z < pMinZ) pMinZ = vb->z; if (vc->z < pMinZ) pMinZ = vc->z;
            int pMaxZ = va->z; if (vb->z > pMaxZ) pMaxZ = vb->z; if (vc->z > pMaxZ) pMaxZ = vc->z;

            if (x >= pMinX && x <= pMaxX && z >= pMinZ && z <= pMaxZ) {
                return false; /* ceiling found above */
            }
        }
    }

    return true;
}

/* ------------------------------------------------------------------ */
/* createChunks — split full mesh into spatial grid for collision      */
/* ------------------------------------------------------------------ */
void Room_createChunks(Room *self) {
    if (!self->fullMesh) return;

    /* Compute bounds from full mesh */
    self->minX = Mesh_minX(self->fullMesh);
    self->maxX = Mesh_maxX(self->fullMesh);
    self->minY = Mesh_minY(self->fullMesh);
    self->maxY = Mesh_maxY(self->fullMesh);
    self->minZ = Mesh_minZ(self->fullMesh);
    self->maxZ = Mesh_maxZ(self->fullMesh);

    int sizeX = self->maxX - self->minX;
    int sizeZ = self->maxZ - self->minZ;

    /* Physics chunks */
    if (Room_chunkSize > 0) {
        self->meshColCount = (sizeX / Room_chunkSize) + 1;
        self->meshRowCount = (sizeZ / Room_chunkSize) + 1;
    } else {
        self->meshColCount = 1;
        self->meshRowCount = 1;
    }

    int totalChunks = self->meshRowCount * self->meshColCount;
    self->mesh = (Mesh **)calloc(totalChunks, sizeof(Mesh *));

    /* Get all polygons from full mesh */
    int polyCount = 0;
    RenderObject **polys = Mesh_polygons(self->fullMesh, &polyCount);
    int vertCount = 0;
    Vertex **verts = Mesh_vertices(self->fullMesh, &vertCount);

    /* For each chunk, collect polygons whose centroid falls in that chunk */
    for (int row = 0; row < self->meshRowCount; row++) {
        for (int col = 0; col < self->meshColCount; col++) {
            int chunkMinX = self->minX + col * Room_chunkSize;
            int chunkMaxX = chunkMinX + Room_chunkSize;
            int chunkMinZ = self->minZ + row * Room_chunkSize;
            int chunkMaxZ = chunkMinZ + Room_chunkSize;

            /* Last chunk extends to room edge */
            if (col == self->meshColCount - 1) chunkMaxX = self->maxX + 1;
            if (row == self->meshRowCount - 1) chunkMaxZ = self->maxZ + 1;

            /* Count polygons in this chunk */
            int count = 0;
            for (int p = 0; p < polyCount; p++) {
                Polygon3V *p3 = (Polygon3V *)polys[p];
                int cx = (p3->a->x + p3->b->x + p3->c->x) / 3;
                int cz = (p3->a->z + p3->b->z + p3->c->z) / 3;
                if (cx >= chunkMinX && cx < chunkMaxX && cz >= chunkMinZ && cz < chunkMaxZ) {
                    count++;
                }
            }

            if (count == 0) continue;

            /* Collect polygons */
            RenderObject **chunkPolys = (RenderObject **)malloc(count * sizeof(RenderObject *));
            int ci = 0;
            for (int p = 0; p < polyCount; p++) {
                Polygon3V *p3 = (Polygon3V *)polys[p];
                int cx = (p3->a->x + p3->b->x + p3->c->x) / 3;
                int cz = (p3->a->z + p3->b->z + p3->c->z) / 3;
                if (cx >= chunkMinX && cx < chunkMaxX && cz >= chunkMinZ && cz < chunkMaxZ) {
                    chunkPolys[ci++] = polys[p];
                }
            }

            self->mesh[row * self->meshColCount + col] =
                Mesh_new_vp(verts, vertCount, chunkPolys, count);
        }
    }

    /* Render chunks — same logic with chunkSizeRender */
    if (Room_chunkSizeRender > 0) {
        self->renderColCount = (sizeX / Room_chunkSizeRender) + 1;
        self->renderRowCount = (sizeZ / Room_chunkSizeRender) + 1;
    } else {
        self->renderColCount = 1;
        self->renderRowCount = 1;
    }

    int totalRender = self->renderRowCount * self->renderColCount;
    self->renderMesh = (Mesh **)calloc(totalRender, sizeof(Mesh *));
    self->boundingBoxes = (BoundingBox **)calloc(totalRender, sizeof(BoundingBox *));

    for (int row = 0; row < self->renderRowCount; row++) {
        for (int col = 0; col < self->renderColCount; col++) {
            int chunkMinX = self->minX + col * Room_chunkSizeRender;
            int chunkMaxX = chunkMinX + Room_chunkSizeRender;
            int chunkMinZ = self->minZ + row * Room_chunkSizeRender;
            int chunkMaxZ = chunkMinZ + Room_chunkSizeRender;

            if (col == self->renderColCount - 1) chunkMaxX = self->maxX + 1;
            if (row == self->renderRowCount - 1) chunkMaxZ = self->maxZ + 1;

            int count = 0;
            for (int p = 0; p < polyCount; p++) {
                Polygon3V *p3 = (Polygon3V *)polys[p];
                int cx = (p3->a->x + p3->b->x + p3->c->x) / 3;
                int cz = (p3->a->z + p3->b->z + p3->c->z) / 3;
                if (cx >= chunkMinX && cx < chunkMaxX && cz >= chunkMinZ && cz < chunkMaxZ) {
                    count++;
                }
            }

            if (count == 0) continue;

            RenderObject **chunkPolys = (RenderObject **)malloc(count * sizeof(RenderObject *));
            int ci = 0;
            for (int p = 0; p < polyCount; p++) {
                Polygon3V *p3 = (Polygon3V *)polys[p];
                int cx = (p3->a->x + p3->b->x + p3->c->x) / 3;
                int cz = (p3->a->z + p3->b->z + p3->c->z) / 3;
                if (cx >= chunkMinX && cx < chunkMaxX && cz >= chunkMinZ && cz < chunkMaxZ) {
                    chunkPolys[ci++] = polys[p];
                }
            }

            int idx = row * self->renderColCount + col;
            self->renderMesh[idx] = Mesh_new_vpt(verts, vertCount, chunkPolys, count,
                                                  self->fullMesh->texture);
            self->boundingBoxes[idx] = BoundingBox_new_mesh(self->renderMesh[idx]);
        }
    }
}

/* ------------------------------------------------------------------ */
/* Accessors                                                           */
/* ------------------------------------------------------------------ */
int Room_getId(Room *self) { return self->id; }

Mesh *Room_getMesh(Room *self, int row, int col) {
    if (!self->mesh) return NULL;
    int idx = row * self->meshColCount + col;
    return self->mesh[idx];
}

int Room_getMinX(Room *self) { return self->minX; }
int Room_getMaxX(Room *self) { return self->maxX; }
int Room_getMinY(Room *self) { return self->minY; }
int Room_getMaxY(Room *self) { return self->maxY; }
int Room_getMinZ(Room *self) { return self->minZ; }
int Room_getMaxZ(Room *self) { return self->maxZ; }

/* ------------------------------------------------------------------ */
/* Object management                                                   */
/* ------------------------------------------------------------------ */
void Room_addObject(Room *self, RoomObject *obj) {
    if (self->objectCount >= self->objectCapacity) {
        self->objectCapacity *= 2;
        self->objects = (RoomObject **)realloc(self->objects,
                        self->objectCapacity * sizeof(RoomObject *));
    }
    self->objects[self->objectCount++] = obj;
}

void Room_removeObject(Room *self, RoomObject *obj) {
    for (int i = 0; i < self->objectCount; i++) {
        if (self->objects[i] == obj) {
            /* Shift remaining elements */
            for (int j = i; j < self->objectCount - 1; j++) {
                self->objects[j] = self->objects[j + 1];
            }
            self->objectCount--;
            return;
        }
    }
}
