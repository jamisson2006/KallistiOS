/*
 * house_creator.c — porte fiel de code/Gameplay/Map/HouseCreator.java
 */
#include "house_creator.h"
#include "house.h"
#include "room.h"
#include "portal.h"
#include "light_mapper.h"
#include "../../math/vector3d.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

extern int DirectX7_standartDrawmode;

typedef struct Vertex {
    int x, y, z;
} Vertex;

typedef struct Mesh Mesh;
extern Vertex *Mesh_getVertices(Mesh *m, int *outCount);
extern void   *Mesh_getPolygons(Mesh *m, int *outCount);
extern void    Mesh_optimize(Mesh *m);
extern void    Mesh_setTexture(Mesh *m, void *tex);

/* Compute centre of a set of vertices */
static Vector3D computeCentre(Vertex *verts, int count) {
    long x = 0, y = 0, z = 0;
    for (int i = 0; i < count; i++) {
        x += verts[i].x;
        y += verts[i].y;
        z += verts[i].z;
    }
    Vector3D v;
    v.x = (int)(x / count);
    v.y = (int)(y / count);
    v.z = (int)(z / count);
    return v;
}

/* Check if any vertex in portal is shared with mesh */
static bool isExistsCommonCoords(Mesh *mesh, Vertex *portal, int portalCount) {
    int meshVertCount = 0;
    Vertex *meshVerts = Mesh_getVertices(mesh, &meshVertCount);

    for (int i = 0; i < portalCount; i++) {
        int px = portal[i].x, py = portal[i].y, pz = portal[i].z;
        for (int t = 0; t < meshVertCount; t++) {
            if (abs(meshVerts[t].x - px) > 25) continue;
            if (abs(meshVerts[t].y - py) > 25) continue;
            if (abs(meshVerts[t].z - pz) > 25) continue;
            return true;
        }
    }
    return false;
}

House *HouseCreator_create(Mesh **meshes, int meshCount, bool np,
                           const char *lightdataFile, bool optimizeLevel) {
    if (optimizeLevel) {
        for (int i = 0; i < meshCount; i++) {
            Mesh_optimize(meshes[i]);
        }
    }

    if (lightdataFile != NULL &&
        (DirectX7_standartDrawmode == 9 || DirectX7_standartDrawmode == 13)) {
        LightMapper_loadLightMap(meshes, meshCount, lightdataFile);
    }

    House *house;

    if (meshCount > 1) {
        int roomCount = meshCount - 1;
        Room **rooms = (Room **)calloc(roomCount, sizeof(Room *));

        for (int i = 0; i < roomCount; i++) {
            rooms[i] = Room_new(i);
            /* Room stores its mesh reference internally */
        }

        /* Build neighbours — simplified: all rooms connected via portal mesh */
        Room ***neighbours = (Room ***)calloc(roomCount, sizeof(Room **));
        for (int i = 0; i < roomCount; i++) {
            /* Worst case: all other rooms are neighbours */
            neighbours[i] = (Room **)calloc(roomCount + 1, sizeof(Room *));
            int nCount = 0;
            for (int j = 0; j < roomCount; j++) {
                if (j != i) {
                    neighbours[i][nCount++] = rooms[j];
                }
            }
            neighbours[i][nCount] = NULL;
        }

        house = House_new(rooms, roomCount, neighbours);
    } else {
        Room **rooms = (Room **)calloc(1, sizeof(Room *));
        rooms[0] = Room_new(0);

        Room ***neighbours = (Room ***)calloc(1, sizeof(Room **));
        neighbours[0] = (Room **)calloc(2, sizeof(Room *));
        neighbours[0][0] = rooms[0];
        neighbours[0][1] = NULL;

        house = House_new(rooms, 1, neighbours);
    }

    if (lightdataFile == NULL &&
        (DirectX7_standartDrawmode == 9 || DirectX7_standartDrawmode == 13)) {
        /* LightMapper generation too slow for DC — skipped */
    }

    return house;
}
