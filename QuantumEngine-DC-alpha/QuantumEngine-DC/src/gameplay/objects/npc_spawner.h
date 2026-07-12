/*
 * npc_spawner.h — porte fiel de code/Gameplay/Objects/NPCSpawner.java
 */
#ifndef QE_GAMEPLAY_OBJECTS_NPC_SPAWNER_H
#define QE_GAMEPLAY_OBJECTS_NPC_SPAWNER_H

#include <stdbool.h>

typedef struct RoomObject RoomObject;
typedef struct NPC        NPC;

typedef struct NPCSpawner {
    RoomObject  base;           /* heranca: RoomObject */
    NPC       **bots;
    int         botsCount;
    int         botsCapacity;
    int         canSpawn;
    int         respawnIn;
    int         rot;
    bool        visibilityChecker;
    long        distanceToSpawn;
    char      **onSpawn;
    int         onSpawnCount;
    int         spawnerId;
} NPCSpawner;

void NPCSpawner_init(NPCSpawner *self, int spawnerId);
void NPCSpawner_destroy(NPCSpawner *self);

void NPCSpawner_update(NPCSpawner *self);
void NPCSpawner_addBot(NPCSpawner *self, NPC *bot);
void NPCSpawner_setOnSpawn(NPCSpawner *self, char **scripts, int count);
void NPCSpawner_setCanSpawn(NPCSpawner *self, int canSpawn);
void NPCSpawner_setRespawnIn(NPCSpawner *self, int respawnIn);
void NPCSpawner_setVisibilityChecker(NPCSpawner *self, bool check);
void NPCSpawner_setDistanceToSpawn(NPCSpawner *self, long distance);
void NPCSpawner_setRot(NPCSpawner *self, int rot);

#endif
