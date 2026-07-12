/*
 * house.c — porte fiel de code/Gameplay/Map/House.java
 */
#include "house.h"
#include "room.h"
#include "skybox.h"
#include "portal.h"
#include "light.h"
#include "room_object.h"
#include "../../math/vector3d.h"
#include <stdlib.h>
#include <string.h>

bool House_l2dRoomRendering = false;
bool House_boxRoomTesting = false;

House *House_new(Room **rooms, int roomCount, Room ***neighbours) {
    House *h = (House *)calloc(1, sizeof(House));
    h->rooms = rooms;
    h->roomCount = roomCount;
    h->neighbours = neighbours;
    h->skybox = NULL;
    return h;
}

void House_destroy(House *self) {
    if (!self) return;
    if (self->rooms) {
        for (int i = 0; i < self->roomCount; i++) {
            if (self->rooms[i]) {
                Room_destroy(self->rooms[i]);
                self->rooms[i] = NULL;
            }
        }
        free(self->rooms);
        self->rooms = NULL;
    }
    if (self->neighbours) {
        for (int i = 0; i < self->roomCount; i++) {
            free(self->neighbours[i]);
        }
        free(self->neighbours);
        self->neighbours = NULL;
    }
    if (self->skybox) {
        Skybox_destroy(self->skybox);
        self->skybox = NULL;
    }
    free(self);
}

/* --- Accessors --- */

Room **House_getRooms(House *self) { return self->rooms; }
int    House_getRoomCount(House *self) { return self->roomCount; }
void   House_setSkybox(House *self, Skybox *skybox) { self->skybox = skybox; }
Skybox*House_getSkybox(House *self) { return self->skybox; }

Room **House_getNeighbourRooms(House *self, int part, int *outCount) {
    if (part < 0 || part >= self->roomCount) {
        if (outCount) *outCount = 0;
        return NULL;
    }
    Room **nei = self->neighbours[part];
    int count = 0;
    if (nei) {
        while (nei[count] != NULL) count++;
    }
    if (outCount) *outCount = count;
    return nei;
}

bool House_isNear(House *self, int id1, int id2) {
    if (id1 < 0 || id2 < 0) return false;
    if (id1 == id2) return true;
    if (id1 >= self->roomCount) return false;

    Room **nei = self->neighbours[id1];
    if (!nei) return false;

    for (int i = 0; nei[i] != NULL; i++) {
        if (Room_getId(nei[i]) == id2) return true;
    }
    return false;
}

/* --- Part calculation --- */

int House_calcPart(House *self, int oldPart, int x, int y, int z, bool rayCast) {
    if (self->roomCount == 1) return 0;

    if (oldPart >= 0 && oldPart < self->roomCount) {
        if (rayCast && Room_isPointOnMesh(self->rooms[oldPart], x, z)) return oldPart;
        /* TODO: isPointInRoomBox for !rayCast */

        Room **nei = self->neighbours[oldPart];
        if (nei) {
            for (int i = 0; nei[i] != NULL; i++) {
                Room *room = nei[i];
                if (rayCast && Room_isPointOnMesh(room, x, z)) return Room_getId(room);
            }
        }
    }

    for (int i = 0; i < self->roomCount; i++) {
        if (rayCast && Room_isPointOnMesh(self->rooms[i], x, z)) return i;
    }

    return -1;
}

void House_recomputePart(House *self, RoomObject *obj) {
    int x = RoomObject_getPosX(obj);
    int z = RoomObject_getPosZ(obj);
    int y = RoomObject_getPosY(obj);
    int oldPart = RoomObject_getPart(obj);
    int newPart = House_calcPart(self, oldPart, x, y, z, true);

    if (newPart == -1) {
        newPart = House_calcPart(self, oldPart, x, y, z, false);
    }
    if (newPart == -1) newPart = oldPart;

    if (newPart >= 0 && newPart != oldPart) {
        if (oldPart >= 0 && oldPart < self->roomCount) {
            Room_removeObject(self->rooms[oldPart], obj);
        }
        Room_addObject(self->rooms[newPart], obj);
        RoomObject_setPart(obj, newPart, self);
    } else if (oldPart < 0 && newPart >= 0) {
        Room_addObject(self->rooms[newPart], obj);
        RoomObject_setPart(obj, newPart, self);
    }
}

void House_addObject(House *self, RoomObject *obj) {
    House_recomputePart(self, obj);
}

void House_removeObject(House *self, RoomObject *obj) {
    int part = RoomObject_getPart(obj);
    if (part >= 0 && part < self->roomCount) {
        Room_removeObject(self->rooms[part], obj);
    }
}

/* --- Object queries --- */

RoomObject **House_getNearObjects(House *self, int part, int *outCount) {
    if (part < 0 || part >= self->roomCount) {
        if (outCount) *outCount = 0;
        return NULL;
    }

    int totalCap = 64;
    int total = 0;
    RoomObject **buf = (RoomObject **)malloc(totalCap * sizeof(RoomObject *));

    /* Objects from current room */
    Room *room = self->rooms[part];
    for (int i = 0; i < room->objectCount; i++) {
        if (total >= totalCap) {
            totalCap *= 2;
            buf = (RoomObject **)realloc(buf, totalCap * sizeof(RoomObject *));
        }
        buf[total++] = room->objects[i];
    }

    /* Objects from neighbour rooms */
    Room **nei = self->neighbours[part];
    if (nei) {
        for (int n = 0; nei[n] != NULL; n++) {
            Room *nroom = nei[n];
            for (int i = 0; i < nroom->objectCount; i++) {
                if (total >= totalCap) {
                    totalCap *= 2;
                    buf = (RoomObject **)realloc(buf, totalCap * sizeof(RoomObject *));
                }
                buf[total++] = nroom->objects[i];
            }
        }
    }

    if (outCount) *outCount = total;
    return buf;
}

RoomObject **House_getObjects(House *self, int *outCount) {
    int totalCap = 64;
    int total = 0;
    RoomObject **buf = (RoomObject **)malloc(totalCap * sizeof(RoomObject *));

    for (int i = 0; i < self->roomCount; i++) {
        Room *room = self->rooms[i];
        for (int j = 0; j < room->objectCount; j++) {
            if (total >= totalCap) {
                totalCap *= 2;
                buf = (RoomObject **)realloc(buf, totalCap * sizeof(RoomObject *));
            }
            buf[total++] = room->objects[j];
        }
    }

    if (outCount) *outCount = total;
    return buf;
}

/* --- Lights --- */

void House_sortLights(House *self, Light **lights, int lightCount) {
    for (int i = 0; i < lightCount; i++) {
        if (lights[i]->part == -1) {
            lights[i]->part = House_calcPart(self, -1,
                lights[i]->pos.x, lights[i]->pos.y, lights[i]->pos.z, true);
        }
    }

    for (int i = 0; i < self->roomCount; i++) {
        Room *room = self->rooms[i];
        if (!room) continue;

        int id = Room_getId(room);
        int count = 0;

        for (int x = 0; x < lightCount; x++) {
            if (House_isNear(self, id, lights[x]->part)) count++;
        }

        if (count > 0) {
            room->lights = (Light **)malloc(count * sizeof(Light *));
            room->lightCount = count;
            int idx = 0;
            for (int x = 0; x < lightCount; x++) {
                if (House_isNear(self, id, lights[x]->part)) {
                    room->lights[idx++] = lights[x];
                }
            }
        }
    }
}

/* --- Rendering --- */

void House_render(House *self, DirectX7 *g3d, int cameraPart) {
    /* Rendering dispatched through the portal system.
       Full implementation connected at integration time. */
    (void)g3d;
    (void)cameraPart;
}
