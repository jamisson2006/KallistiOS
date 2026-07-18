/*
 * scene.h — porte fiel de code/Gameplay/Scene.java
 */
#ifndef QE_GAMEPLAY_SCENE_H
#define QE_GAMEPLAY_SCENE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct House        House;
typedef struct DirectX7     DirectX7;
typedef struct Respawn      Respawn;
typedef struct Bot          Bot;
typedef struct Player       Player;
typedef struct GameScreen   GameScreen;
typedef struct Graphics     Graphics;
typedef struct Dijkstra     Dijkstra;
typedef struct RoomObject   RoomObject;
typedef struct ItemsBag     ItemsBag;
typedef struct TPPose       TPPose;
typedef struct Vector3D     Vector3D;

typedef struct Scene {
    House    *house;
    DirectX7 *g3d;

    /* Spawn points */
    Respawn  *start;
    Respawn  *finish;

    /* Enemies */
    Respawn **enemies;
    int       enemyCount;

    /* Bots */
    Bot     **bots;
    int       botCount;

    int       max_enemy_count;
    int       enemy_count;
    int       frequency;
    int       frame;
    int       miny;

    /* Part system */
    int       part;
    bool      deleteAnPart;
    bool      alwaysExit;
    bool      exitWithoutWait;

    /* Need conditions */
    char    **need;
    int       needCount;

    /* Bot limiter */
    int      *botLimiter;
    int       botLimiterCount;

    /* Pathfinding */
    Dijkstra *ways;

    /* RoomObjects (dynamic arrays) */
    RoomObject **rmsBots;
    int          rmsBotsCount;
    bool        *killed;
    int          killedCount;

    RoomObject **rmsObjects;
    int          rmsObjectsCount;
    bool        *destroyed;
    int          destroyedCount;

    /* Scripts to execute */
    char      **scriptsToExec;
    int         scriptsToExecCount;

    /* Camera pose for third-person */
    TPPose     *camPose;

    /* Gravity (static in Java) */
    /* accessed via Scene_getGravity() / Scene_setGravity() */
} Scene;

/* Static gravity */
extern int Scene_gravity;

/* Lifecycle */
Scene  *Scene_new(void);
void    Scene_destroy(Scene *s);
void    Scene_reset(Scene *s);

/* Update/Render */
void    Scene_update(Scene *s, Player *player, GameScreen *gs);
void    Scene_recomputePart(Scene *s);
void    Scene_render(Scene *s, Graphics *g, int parts, int part, Player *player);
void    Scene_flush(Scene *s);

/* Spawn points */
Respawn *Scene_getStartPoint(Scene *s);
Respawn *Scene_getFinishPoint(Scene *s);

/* Level state */
bool    Scene_isLevelCompleted(Scene *s, Player *player, GameScreen *gs);
bool    Scene_isWinner(Scene *s);
bool    Scene_isAllCollected(Scene *s, Player *player, GameScreen *gs);

/* Object management */
RoomObject *Scene_findObject(Scene *s, const char *name);
void    Scene_activateObject(Scene *s, const char *name, Player *player, GameScreen *gs);
void    Scene_deleteUsedObjects(Scene *s);

/* Drawing */
void    Scene_drawPoints(Scene *s, DirectX7 *g3d, Player *player);

/* Utility */
int     Scene_distance(int x1, int y1, int z1, int x2, int y2, int z2);
void    Scene_removeKilledBots(Scene *s);

/* Item bags */
ItemsBag **Scene_getItemBags(Scene *s, int *outCount);

/* Player location */
int     Scene_findPlayer(Scene *s, Player *player);

/* Drop items */
void    Scene_dropItem(Scene *s, int x, int y, int z, const char *item);
void    Scene_dropItemBag(Scene *s, ItemsBag *bag);

/* Scripting */
void    Scene_runScript(Scene *s, char **script, int scriptCount, Player *player,
                        GameScreen *gs);

/* Accessors */
House   *Scene_getHouse(Scene *s);
DirectX7 *Scene_getG3D(Scene *s);
int      Scene_getFrame(Scene *s);
int      Scene_getEnemyCount(Scene *s);
int      Scene_getGravity(void);
void     Scene_setGravity(int g);

#endif
