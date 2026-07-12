/*
 * npc_spawner.c — porte fiel de code/Gameplay/Objects/NPCSpawner.java
 */
#include "npc_spawner.h"
#include <stdlib.h>
#include <string.h>

/* Dependencias externas */
extern bool NPC_isDead(NPC *npc);
extern void NPC_spawn(NPC *npc, int rot);
extern bool NPC_isVisible(NPC *npc);
extern long NPC_getDistanceToPlayer(NPC *npc);
extern void ScriptEngine_run(const char *script);
extern long System_currentTimeMillis(void);

void NPCSpawner_init(NPCSpawner *self, int spawnerId) {
    memset(self, 0, sizeof(NPCSpawner));
    self->spawnerId = spawnerId;
    self->canSpawn = 1;
    self->respawnIn = 0;
    self->rot = 0;
    self->visibilityChecker = false;
    self->distanceToSpawn = 0;
    self->bots = NULL;
    self->botsCount = 0;
    self->botsCapacity = 0;
    self->onSpawn = NULL;
    self->onSpawnCount = 0;
}

void NPCSpawner_destroy(NPCSpawner *self) {
    if (!self) return;
    if (self->bots) {
        free(self->bots);
        self->bots = NULL;
    }
    if (self->onSpawn) {
        for (int i = 0; i < self->onSpawnCount; i++) {
            if (self->onSpawn[i]) free(self->onSpawn[i]);
        }
        free(self->onSpawn);
        self->onSpawn = NULL;
    }
}

void NPCSpawner_addBot(NPCSpawner *self, NPC *bot) {
    if (self->botsCount >= self->botsCapacity) {
        int newCap = self->botsCapacity == 0 ? 4 : self->botsCapacity * 2;
        NPC **newBots = (NPC **)realloc(self->bots, newCap * sizeof(NPC *));
        if (!newBots) return;
        self->bots = newBots;
        self->botsCapacity = newCap;
    }
    self->bots[self->botsCount++] = bot;
}

void NPCSpawner_update(NPCSpawner *self) {
    if (self->canSpawn <= 0) return;

    for (int i = 0; i < self->botsCount; i++) {
        NPC *bot = self->bots[i];
        if (bot == NULL) continue;
        if (!NPC_isDead(bot)) continue;

        /* Visibility check */
        if (self->visibilityChecker) {
            if (NPC_isVisible(bot)) continue;
        }

        /* Distance check */
        if (self->distanceToSpawn > 0) {
            long dist = NPC_getDistanceToPlayer(bot);
            if (dist < self->distanceToSpawn) continue;
        }

        /* Spawn the bot */
        NPC_spawn(bot, self->rot);
        self->canSpawn--;

        /* Run onSpawn scripts */
        for (int j = 0; j < self->onSpawnCount; j++) {
            if (self->onSpawn[j]) {
                ScriptEngine_run(self->onSpawn[j]);
            }
        }

        if (self->canSpawn <= 0) break;
    }
}

void NPCSpawner_setOnSpawn(NPCSpawner *self, char **scripts, int count) {
    self->onSpawn = scripts;
    self->onSpawnCount = count;
}

void NPCSpawner_setCanSpawn(NPCSpawner *self, int canSpawn) {
    self->canSpawn = canSpawn;
}

void NPCSpawner_setRespawnIn(NPCSpawner *self, int respawnIn) {
    self->respawnIn = respawnIn;
}

void NPCSpawner_setVisibilityChecker(NPCSpawner *self, bool check) {
    self->visibilityChecker = check;
}

void NPCSpawner_setDistanceToSpawn(NPCSpawner *self, long distance) {
    self->distanceToSpawn = distance;
}

void NPCSpawner_setRot(NPCSpawner *self, int rot) {
    self->rot = rot;
}
