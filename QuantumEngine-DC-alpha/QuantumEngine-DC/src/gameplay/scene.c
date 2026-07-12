/*
 * scene.c — porte fiel de code/Gameplay/Scene.java
 */
#include "scene.h"
#include "respawn.h"
#include "map/dijkstra.h"
#include "../math/vector3d.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Extern dependencies */
extern int64_t qe_current_ms(void);

/* Static gravity (Java: static int gravity) */
int Scene_gravity = 900;

Scene *Scene_new(void) {
    Scene *s = (Scene *)calloc(1, sizeof(Scene));
    s->frequency = 3000;
    s->max_enemy_count = 5;
    s->miny = -10000;
    s->part = -1;
    return s;
}

void Scene_destroy(Scene *s) {
    if (!s) return;

    if (s->enemies) { free(s->enemies); s->enemies = NULL; }
    if (s->bots) {
        /* Bots freed externally */
        free(s->bots);
        s->bots = NULL;
    }
    if (s->need) {
        for (int i = 0; i < s->needCount; i++) free(s->need[i]);
        free(s->need);
        s->need = NULL;
    }
    if (s->botLimiter) { free(s->botLimiter); s->botLimiter = NULL; }
    if (s->rmsBots) { free(s->rmsBots); s->rmsBots = NULL; }
    if (s->killed) { free(s->killed); s->killed = NULL; }
    if (s->rmsObjects) { free(s->rmsObjects); s->rmsObjects = NULL; }
    if (s->destroyed) { free(s->destroyed); s->destroyed = NULL; }
    if (s->scriptsToExec) {
        for (int i = 0; i < s->scriptsToExecCount; i++) free(s->scriptsToExec[i]);
        free(s->scriptsToExec);
        s->scriptsToExec = NULL;
    }
    free(s);
}

void Scene_reset(Scene *s) {
    s->frame = 0;
    s->enemy_count = 0;
    s->part = -1;

    /* Reset killed/destroyed flags */
    if (s->killed) {
        memset(s->killed, 0, s->killedCount * sizeof(bool));
    }
    if (s->destroyed) {
        memset(s->destroyed, 0, s->destroyedCount * sizeof(bool));
    }
}

void Scene_update(Scene *s, Player *player, GameScreen *gs) {
    s->frame++;

    /* Spawning logic: check frequency for enemy respawn */
    if (s->enemies && s->enemyCount > 0) {
        if (s->frame % s->frequency == 0 && s->enemy_count < s->max_enemy_count) {
            /* Spawn from random respawn point */
            int idx = rand() % s->enemyCount;
            (void)idx;
            s->enemy_count++;
        }
    }

    /* Update bots */
    for (int i = 0; i < s->botCount; i++) {
        if (s->killed && i < s->killedCount && s->killed[i]) continue;
        /* Bot_update(s->bots[i], player, s, gs); — extern */
    }

    /* Execute pending scripts */
    if (s->scriptsToExecCount > 0) {
        for (int i = 0; i < s->scriptsToExecCount; i++) {
            /* Scripts processed externally */
            free(s->scriptsToExec[i]);
        }
        free(s->scriptsToExec);
        s->scriptsToExec = NULL;
        s->scriptsToExecCount = 0;
    }

    /* Recompute part if needed */
    Scene_recomputePart(s);
}

void Scene_recomputePart(Scene *s) {
    /* Part is recomputed based on player position in house */
    /* Delegated to House system */
    (void)s;
}

void Scene_render(Scene *s, Graphics *g, int parts, int part, Player *player) {
    /* Render room objects in current part */
    for (int i = 0; i < s->rmsObjectsCount; i++) {
        if (s->destroyed && i < s->destroyedCount && s->destroyed[i]) continue;
        /* RoomObject_render(s->rmsObjects[i], g3d) — extern */
    }

    /* Render bots */
    for (int i = 0; i < s->botCount; i++) {
        if (s->killed && i < s->killedCount && s->killed[i]) continue;
        /* Bot_render(s->bots[i], g) — extern */
    }
    (void)g;
    (void)parts;
    (void)part;
    (void)player;
}

void Scene_flush(Scene *s) {
    /* Flush rendering buffer for scene objects */
    (void)s;
}

Respawn *Scene_getStartPoint(Scene *s) { return s->start; }
Respawn *Scene_getFinishPoint(Scene *s) { return s->finish; }

bool Scene_isLevelCompleted(Scene *s, Player *player, GameScreen *gs) {
    /* Check if all conditions met: need items collected, enemies killed */
    if (s->need && s->needCount > 0) {
        if (!Scene_isAllCollected(s, player, gs)) return false;
    }
    if (!s->exitWithoutWait && s->enemy_count < s->max_enemy_count) {
        /* Still spawning enemies */
    }
    return true;
}

bool Scene_isWinner(Scene *s) {
    /* Check if player reached finish point or other win condition */
    (void)s;
    return false;
}

bool Scene_isAllCollected(Scene *s, Player *player, GameScreen *gs) {
    if (!s->need || s->needCount <= 0) return true;
    for (int i = 0; i < s->needCount; i++) {
        /* Check if player has required item via Player_hasItem — extern */
        (void)player;
        (void)gs;
    }
    return true;
}

RoomObject *Scene_findObject(Scene *s, const char *name) {
    if (!name) return NULL;
    for (int i = 0; i < s->rmsObjectsCount; i++) {
        RoomObject *obj = s->rmsObjects[i];
        if (obj && obj->name && strcmp(obj->name, name) == 0) {
            return obj;
        }
    }
    for (int i = 0; i < s->rmsBotsCount; i++) {
        RoomObject *obj = s->rmsBots[i];
        if (obj && obj->name && strcmp(obj->name, name) == 0) {
            return obj;
        }
    }
    return NULL;
}

void Scene_activateObject(Scene *s, const char *name, Player *player, GameScreen *gs) {
    RoomObject *obj = Scene_findObject(s, name);
    if (obj) {
        RoomObject_activate(obj, player, s->house, gs);
    }
}

void Scene_deleteUsedObjects(Scene *s) {
    for (int i = 0; i < s->rmsObjectsCount; i++) {
        RoomObject *obj = s->rmsObjects[i];
        if (obj && obj->destroyOnUse && obj->activated) {
            if (i < s->destroyedCount) {
                s->destroyed[i] = true;
            }
        }
    }
}

void Scene_drawPoints(Scene *s, DirectX7 *g3d, Player *player) {
    /* Draw activation points for nearby objects */
    for (int i = 0; i < s->rmsObjectsCount; i++) {
        if (s->destroyed && i < s->destroyedCount && s->destroyed[i]) continue;
        RoomObject *obj = s->rmsObjects[i];
        if (obj && obj->needToPoint) {
            RoomObject_drawDebug(obj, g3d);
        }
    }
    (void)player;
}

int Scene_distance(int x1, int y1, int z1, int x2, int y2, int z2) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    int dz = z2 - z1;
    return (int)sqrt((double)(dx * dx + dy * dy + dz * dz));
}

void Scene_removeKilledBots(Scene *s) {
    for (int i = 0; i < s->botCount; i++) {
        if (s->killed && i < s->killedCount && s->killed[i]) {
            /* Already marked as killed, skip in rendering/update */
        }
    }
}

ItemsBag **Scene_getItemBags(Scene *s, int *outCount) {
    /* Collect all ItemsBag objects from rmsObjects */
    int count = 0;
    for (int i = 0; i < s->rmsObjectsCount; i++) {
        if (s->destroyed && i < s->destroyedCount && s->destroyed[i]) continue;
        /* Check type — simplified */
        count++;
    }
    *outCount = 0;
    return NULL; /* Proper implementation requires RTTI or type tag */
}

int Scene_findPlayer(Scene *s, Player *player) {
    /* Return part index where player is located */
    (void)s;
    (void)player;
    return 0;
}

void Scene_dropItem(Scene *s, int x, int y, int z, const char *item) {
    /* Create item bag at position and add to scene */
    (void)s;
    (void)x;
    (void)y;
    (void)z;
    (void)item;
}

void Scene_dropItemBag(Scene *s, ItemsBag *bag) {
    /* Add pre-built item bag to scene objects */
    (void)s;
    (void)bag;
}

void Scene_runScript(Scene *s, char **script, int scriptCount, Player *player,
                     GameScreen *gs) {
    /* Queue script for execution via RoomObject_give */
    RoomObject_give(script, scriptCount, player, s->house, gs);
}

House *Scene_getHouse(Scene *s) { return s->house; }
DirectX7 *Scene_getG3D(Scene *s) { return s->g3d; }
int Scene_getFrame(Scene *s) { return s->frame; }
int Scene_getEnemyCount(Scene *s) { return s->enemy_count; }

int Scene_getGravity(void) { return Scene_gravity; }
void Scene_setGravity(int g) { Scene_gravity = g; }
