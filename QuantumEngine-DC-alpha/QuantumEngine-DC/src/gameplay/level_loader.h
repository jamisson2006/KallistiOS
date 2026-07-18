/*
 * level_loader.h — porte fiel de code/Gameplay/LevelLoader.java
 */
#ifndef QE_GAMEPLAY_LEVEL_LOADER_H
#define QE_GAMEPLAY_LEVEL_LOADER_H

#include <stdbool.h>
#include <stdint.h>

typedef struct Scene      Scene;
typedef struct GameScreen GameScreen;
typedef struct Player     Player;
typedef struct RoomObject RoomObject;
typedef struct House      House;
typedef struct NPC        NPC;

/* Static flags */
extern bool LevelLoader_defaultOneBot;

/* Main entry — create a scene from a level file */
Scene *LevelLoader_createScene(int width, int height, const char *file, GameScreen *gs);

/* Load objects into an existing scene from a level file */
void LevelLoader_loadObjects(const char *level, void *lvl, Scene *scene,
                             Player *p, bool onlyObjs);

/* Load lights from level file text */
void LevelLoader_loadLights(const char *file, void *lvl);

/* Load an NPC from ini data */
NPC *LevelLoader_loadNPC(void *obj, const char *key, int *ps);

/* Load common RoomObject properties */
void LevelLoader_loadRM(void *txt, RoomObject *obj, void *lvl, const char *key, const char *prefix);

/* Create script from text block */
char **LevelLoader_createScript(const char *str, int *outCount);

/* Load script from file or inline */
char **LevelLoader_loadScriptFromFile(const char *text, int *outCount);

/* Helper chain — getString with fallback through lvl/settings */
char *LevelLoader_getString(const char *s, void *s1, void *s2);
char *LevelLoader_getStringKey(const char *s, void *s1, void *s2, const char *key);
char *LevelLoader_getStringdef(const char *s, void *s1, void *s2, const char *def);
int   LevelLoader_getInt(const char *s, void *s1, void *s2, int def);
int   LevelLoader_getIntKey(const char *s, void *s1, void *s2, const char *key, int def);
float LevelLoader_getFloat(const char *s, void *s1, void *s2, float def);
float LevelLoader_getFloatKey(const char *s, void *s1, void *s2, const char *key, float def);
bool  LevelLoader_getBoolean(const char *s, void *s1, void *s2, bool def);
bool  LevelLoader_getBooleanKey(const char *s, void *s1, void *s2, const char *key, bool def);
int64_t LevelLoader_getLongKey(const char *s, void *s1, void *s2, const char *key, int64_t def);

#endif
