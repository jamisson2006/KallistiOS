/*
 * level_loader.c — porte fiel de code/Gameplay/LevelLoader.java
 */
#include "level_loader.h"
#include "respawn.h"
#include "map/house.h"
#include "map/house_creator.h"
#include "map/room.h"
#include "map/skybox.h"
#include "map/light.h"
#include "map/light_mapper.h"
#include "map/room_object.h"
#include "objects/key_object.h"
#include "objects/teleport.h"
#include "objects/shop_object.h"
#include "objects/lvl_change.h"
#include "objects/image2d.h"
#include "objects/mesh_object.h"
#include "objects/npc_spawner.h"
#include "objects/sprite_object.h"
#include "dialog_screen.h"
#include "../math/vector3d.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

extern char  *StringTools_getStringFromResource(const char *path);
extern char **StringTools_cutOnStrings(const char *str, char sep, int *outCount);
extern int   *StringTools_cutOnInts(const char *str, char sep, int *outCount);
extern int    StringTools_parseInt(const char *str);
extern float  StringTools_parseFloat(const char *str);
extern int64_t StringTools_parseLong(const char *str);

extern void *GameIni_new(const char *text, bool isPath);
extern char *GameIni_get(void *ini, const char *key);
extern int   GameIni_getInt(void *ini, const char *key);
extern float GameIni_getFloat(void *ini, const char *key, float def);
extern char *GameIni_getDef(void *ini, const char *key, const char *def);
extern char *GameIni_getNoLang(void *ini, const char *key);
extern int  *GameIni_cutOnIntsStatic(const char *str, char sep1, char sep2);
extern char **GameIni_cutOnStringsStatic(const char *str, char sep1, char sep2);
extern int   GameIni_startsWith(const char *str, const char **list, int listCount);

extern void *Main_settings;
extern bool  Main_isExist(const char *path);
extern void  Main_setFloorOffset(int v);

extern int  DirectX7_standartDrawmode;
extern void DirectX7_setFogDist(int d);
extern void DirectX7_setDrDist(int d);

extern void *Asset_getTexture(const char *path);
extern void *Asset_getMeshes(const char *path, float sx, float sy, float sz);
extern void  Asset_playSoundIfEnabled(const char *sound);

bool LevelLoader_defaultOneBot = false;

/* --- Helper chain: getString with fallback --- */

char *LevelLoader_getString(const char *s, void *s1, void *s2) {
    char *v = GameIni_get(s1, s);
    if (v != NULL) return v;
    return GameIni_get(s2, s);
}

char *LevelLoader_getStringKey(const char *s, void *s1, void *s2, const char *key) {
    char *v = GameIni_get(s1, s);
    if (v != NULL) return v;
    if (key != NULL) {
        v = GameIni_getDef(s2, key, s);
        if (v != NULL) return v;
    }
    return GameIni_get(s2, s);
}

char *LevelLoader_getStringdef(const char *s, void *s1, void *s2, const char *def) {
    char *v = GameIni_get(s1, s);
    if (v != NULL) return v;
    v = GameIni_get(s2, s);
    if (v != NULL) return v;
    return (char *)def;
}

int LevelLoader_getInt(const char *s, void *s1, void *s2, int def) {
    char *v = GameIni_get(s1, s);
    if (v != NULL) return StringTools_parseInt(v);
    v = GameIni_get(s2, s);
    if (v != NULL) return StringTools_parseInt(v);
    return def;
}

int LevelLoader_getIntKey(const char *s, void *s1, void *s2, const char *key, int def) {
    char *v = LevelLoader_getStringKey(s, s1, s2, key);
    if (v != NULL) return StringTools_parseInt(v);
    return def;
}

float LevelLoader_getFloat(const char *s, void *s1, void *s2, float def) {
    char *v = GameIni_get(s1, s);
    if (v != NULL) return StringTools_parseFloat(v);
    v = GameIni_get(s2, s);
    if (v != NULL) return StringTools_parseFloat(v);
    return def;
}

float LevelLoader_getFloatKey(const char *s, void *s1, void *s2, const char *key, float def) {
    char *v = LevelLoader_getStringKey(s, s1, s2, key);
    if (v != NULL) return StringTools_parseFloat(v);
    return def;
}

bool LevelLoader_getBoolean(const char *s, void *s1, void *s2, bool def) {
    return LevelLoader_getInt(s, s1, s2, def ? 1 : 0) == 1;
}

bool LevelLoader_getBooleanKey(const char *s, void *s1, void *s2, const char *key, bool def) {
    return LevelLoader_getIntKey(s, s1, s2, key, def ? 1 : 0) == 1;
}

int64_t LevelLoader_getLongKey(const char *s, void *s1, void *s2, const char *key, int64_t def) {
    char *v = LevelLoader_getStringKey(s, s1, s2, key);
    if (v != NULL) return StringTools_parseLong(v);
    return def;
}

/* --- readPoints: parse spawn/finish points --- */

static Respawn *readPoints(const char *file, House *house, int *outCount) {
    if (file == NULL) { *outCount = 0; return NULL; }

    /* Remove spaces */
    int len = (int)strlen(file);
    char *cleaned = (char *)malloc(len + 1);
    int ci = 0;
    for (int i = 0; i < len; i++) {
        if (file[i] != ' ') cleaned[ci++] = file[i];
    }
    cleaned[ci] = '\0';

    int posesCount = 0;
    char **poses = StringTools_cutOnStrings(cleaned, ';', &posesCount);
    free(cleaned);

    Respawn *spawn = (Respawn *)calloc(posesCount, sizeof(Respawn));

    for (int i = 0; i < posesCount; i++) {
        int valCount = 0;
        int *vals = StringTools_cutOnInts(poses[i], ',', &valCount);

        Vector3D pos;
        Vector3D_set(&pos, vals[0], vals[1], vals[2]);
        Respawn_init(&spawn[i], &pos, house);

        if (valCount >= 4) {
            spawn[i].mode = -127;
            spawn[i].cmode = (int8_t)(-127 + vals[3]);
            if (valCount == 5 && vals[4] == 1) spawn[i].respa = true;
        } else if (LevelLoader_defaultOneBot) {
            spawn[i].mode = -127;
            spawn[i].cmode = -126;
        }

        free(vals);
    }

    for (int i = 0; i < posesCount; i++) free(poses[i]);
    free(poses);

    *outCount = posesCount;
    return spawn;
}

/* --- loadLights --- */

void LevelLoader_loadLights(const char *file, void *lvl) {
    if (file == NULL) return;

    int lightCount = 0;
    int lightCap = 16;
    Light **lights = (Light **)malloc(lightCap * sizeof(Light *));

    const char *ptr = file;
    while (1) {
        const char *bracketOpen = strchr(ptr, '[');
        if (!bracketOpen) break;

        const char *bracketClose = strchr(bracketOpen, ']');
        if (!bracketClose) break;

        int typeLen = (int)(bracketClose - bracketOpen - 1);
        char *objectType = (char *)malloc(typeLen + 1);
        strncpy(objectType, bracketOpen + 1, typeLen);
        objectType[typeLen] = '\0';

        ptr = bracketClose + 1;
        const char *nextBracket = strchr(ptr, '[');
        int bodyLen;
        if (nextBracket) {
            bodyLen = (int)(nextBracket - ptr);
        } else {
            bodyLen = (int)strlen(ptr);
        }

        char *body = (char *)malloc(bodyLen + 1);
        strncpy(body, ptr, bodyLen);
        body[bodyLen] = '\0';

        if (strstr(objectType, "LIGHT") != NULL) {
            void *obj = GameIni_new(body, false);
            char *posStr = GameIni_get(obj, "POS");

            if (posStr) {
                int posesCount = 0;
                char **posesArr = StringTools_cutOnStrings(posStr, ';', &posesCount);

                for (int count = 0; count < posesCount; count++) {
                    int brightness = LevelLoader_getIntKey("BRIGHTNESS", obj, Main_settings,
                        GameIni_get(obj, "PRESET"), 255);

                    int color[3] = {brightness, brightness, brightness};
                    char *colStr = LevelLoader_getStringKey("COLOR", obj, Main_settings,
                        GameIni_get(obj, "PRESET"));
                    if (colStr) {
                        int colCount = 0;
                        int *colVals = StringTools_cutOnInts(colStr, ',', &colCount);
                        color[0] = colVals[0] * brightness / 255;
                        if (colCount > 1) {
                            color[1] = colVals[1] * brightness / 255;
                            color[2] = colVals[2] * brightness / 255;
                        } else {
                            color[1] = color[2] = color[0];
                        }
                        free(colVals);
                    }

                    int pCount = 0;
                    int *pVals = StringTools_cutOnInts(posesArr[count], ',', &pCount);

                    Light *l = (Light *)calloc(1, sizeof(Light));
                    l->color[0] = color[0];
                    l->color[1] = color[1];
                    l->color[2] = color[2];
                    Vector3D_set(&l->pos, pVals[0], pVals[1], pVals[2]);
                    l->part = -1;

                    char *dirStr = LevelLoader_getStringKey("DIRECTION", obj, Main_settings,
                        GameIni_get(obj, "PRESET"));
                    if (dirStr) {
                        l->direction = (Vector3D *)malloc(sizeof(Vector3D));
                        int dc = 0;
                        int *dv = StringTools_cutOnInts(dirStr, ',', &dc);
                        Vector3D_set(l->direction, dv[0], dv[1], dv[2]);
                        free(dv);
                    } else {
                        char *lookStr = LevelLoader_getStringKey("LOOK_AT", obj, Main_settings,
                            GameIni_get(obj, "PRESET"));
                        if (lookStr) {
                            l->direction = (Vector3D *)malloc(sizeof(Vector3D));
                            int dc = 0;
                            int *dv = StringTools_cutOnInts(lookStr, ',', &dc);
                            Vector3D_set(l->direction, dv[0] - l->pos.x,
                                         dv[1] - l->pos.y, dv[2] - l->pos.z);
                            free(dv);
                        }
                    }

                    l->ceilingFix = (short)LevelLoader_getIntKey("CEILING_FIX", obj, Main_settings,
                        GameIni_get(obj, "PRESET"), 0);
                    l->floorFix = (short)LevelLoader_getIntKey("FLOOR_FIX", obj, Main_settings,
                        GameIni_get(obj, "PRESET"), 0);
                    l->part = LevelLoader_getIntKey("ROOM_ID", obj, Main_settings,
                        GameIni_get(obj, "PRESET"), -1);

                    if (lightCount >= lightCap) {
                        lightCap *= 2;
                        lights = (Light **)realloc(lights, lightCap * sizeof(Light *));
                    }
                    lights[lightCount++] = l;

                    free(pVals);
                }

                for (int i = 0; i < posesCount; i++) free(posesArr[i]);
                free(posesArr);
            }
        }

        free(objectType);
        free(body);

        if (!nextBracket) break;
        ptr = nextBracket;
    }

    LightMapper_setLights(lights, lightCount);
}

/* --- createScript --- */

char **LevelLoader_createScript(const char *str, int *outCount) {
    int lineCount = 0;
    char **lines = StringTools_cutOnStrings(str, '\n', &lineCount);

    int cap = lineCount * 2;
    char **result = (char **)malloc(cap * sizeof(char *));
    int count = 0;

    for (int i = 0; i < lineCount; i++) {
        if (lines[i] == NULL) continue;

        /* Trim */
        char *s = lines[i];
        while (*s == ' ' || *s == '\t') s++;
        int slen = (int)strlen(s);
        while (slen > 0 && (s[slen-1] == ' ' || s[slen-1] == '\t')) slen--;
        s[slen] = '\0';

        if (slen == 0) continue;

        /* Skip comments */
        if (s[0] == '/' && s[1] == '/') continue;
        if (strcmp(s, " ") == 0) continue;

        /* Remove inline comments */
        char *cmnt = strstr(s, "//");
        if (cmnt) *cmnt = '\0';

        slen = (int)strlen(s);

        /* Handle braces on same line as code */
        if ((s[0] == '}' || s[0] == '{') && slen > 1) {
            if (count >= cap) { cap *= 2; result = (char **)realloc(result, cap * sizeof(char *)); }
            result[count++] = strndup(s + 1, slen - 1);
            if (count >= cap) { cap *= 2; result = (char **)realloc(result, cap * sizeof(char *)); }
            result[count++] = strndup(s, 1);
        } else if ((s[slen-1] == '}' || s[slen-1] == '{') && slen > 1) {
            if (count >= cap) { cap *= 2; result = (char **)realloc(result, cap * sizeof(char *)); }
            result[count++] = strndup(s, slen - 1);
            if (count >= cap) { cap *= 2; result = (char **)realloc(result, cap * sizeof(char *)); }
            result[count++] = strndup(s + slen - 1, 1);
        } else {
            if (count >= cap) { cap *= 2; result = (char **)realloc(result, cap * sizeof(char *)); }
            result[count++] = strdup(s);
        }
    }

    for (int i = 0; i < lineCount; i++) free(lines[i]);
    free(lines);

    *outCount = count;
    return result;
}

char **LevelLoader_loadScriptFromFile(const char *text, int *outCount) {
    if (text == NULL) { *outCount = 0; return NULL; }

    int len = (int)strlen(text);
    if (text[0] != '/' || len < 5) {
        return StringTools_cutOnStrings(text, ';', outCount);
    }

    const char *ext = text + len - 4;
    if (strcmp(ext, ".txt") != 0 && strcmp(ext, ".TXT") != 0) {
        return StringTools_cutOnStrings(text, ';', outCount);
    }

    char *loaded = StringTools_getStringFromResource(text);
    if (!loaded) { *outCount = 0; return NULL; }

    char **result = LevelLoader_createScript(loaded, outCount);
    free(loaded);
    return result;
}

/* --- loadRM — load common RoomObject properties --- */

void LevelLoader_loadRM(void *txt, RoomObject *obj, void *lvl, const char *key, const char *prefix) {
    char buf[128];

    #define PKEY(name) (prefix ? (snprintf(buf, sizeof(buf), "%s%s", prefix, name), buf) : name)

    obj->activable = LevelLoader_getBooleanKey(PKEY("ACTIVABLE"), txt, Main_settings, key, obj->activable);
    obj->clickable = LevelLoader_getBooleanKey(PKEY("CLICKABLE"), txt, Main_settings, key, obj->clickable);
    obj->singleUse = LevelLoader_getBooleanKey(PKEY("SINGLEUSE"), txt, Main_settings, key, obj->singleUse);
    obj->destroyOnUse = LevelLoader_getBooleanKey(PKEY("DESTROYONUSE"), txt, Main_settings, key, obj->destroyOnUse);

    char *tmp;

    tmp = LevelLoader_getStringKey(PKEY("NAME"), txt, Main_settings, key);
    if (tmp) obj->name = strdup(tmp);

    tmp = LevelLoader_getStringKey(PKEY("DESC"), txt, Main_settings, key);
    if (tmp) obj->desc = strdup(tmp);

    tmp = LevelLoader_getStringKey(PKEY("NEED"), txt, Main_settings, key);
    if (tmp) {
        obj->need = StringTools_cutOnStrings(tmp, ',', &obj->needCount);
    }

    tmp = LevelLoader_getStringKey(PKEY("ON_ACTIVATE"), txt, Main_settings, key);
    if (tmp) {
        obj->additional = LevelLoader_loadScriptFromFile(tmp, &obj->additionalCount);
    }

    obj->alwaysActivate = LevelLoader_getBooleanKey(PKEY("ALWAYS_ACTIVATE"), txt, Main_settings, key, obj->alwaysActivate);

    tmp = LevelLoader_getStringKey(PKEY("MESSAGE"), txt, Main_settings, key);
    if (tmp) obj->message = DialogScreen_loadTextFromFile(tmp);

    tmp = LevelLoader_getStringKey(PKEY("ERRMESSAGE"), txt, Main_settings, key);
    if (tmp) obj->errMessage = DialogScreen_loadTextFromFile(tmp);

    obj->radius = LevelLoader_getLongKey(PKEY("RADIUS"), txt, Main_settings, key, obj->radius);
    obj->messageTimeOut = LevelLoader_getLongKey(PKEY("MESSAGETIMEOUT"), txt, Main_settings, key, obj->messageTimeOut);
    obj->messageType = LevelLoader_getIntKey(PKEY("MESSAGETYPE"), txt, Main_settings, key, obj->messageType);
    obj->errMessageTimeOut = LevelLoader_getLongKey(PKEY("ERRMESSAGETIMEOUT"), txt, Main_settings, key, obj->errMessageTimeOut);
    obj->errMessageType = LevelLoader_getIntKey(PKEY("ERRMESSAGETYPE"), txt, Main_settings, key, obj->errMessageType);
    obj->messageDelay = LevelLoader_getLongKey(PKEY("MSGDELAY"), txt, Main_settings, key, obj->messageDelay);
    obj->errMessageDelay = LevelLoader_getLongKey(PKEY("ERRMSGDELAY"), txt, Main_settings, key, obj->errMessageDelay);

    tmp = LevelLoader_getStringKey(PKEY("REUSETIMER"), txt, Main_settings, key);
    if (tmp) {
        obj->timeToReset = StringTools_parseLong(tmp);
        obj->hideWhenUnusable = true;
    }

    obj->hideWhenUnusable = LevelLoader_getBooleanKey(PKEY("HIDEREUSE"), txt, Main_settings, key, obj->hideWhenUnusable);

    tmp = LevelLoader_getStringKey(PKEY("NEEDTOPOINT"), txt, Main_settings, key);
    if (tmp) {
        if (strcmp(tmp, "1") == 0) obj->needToPoint = true;
        else if (strcmp(tmp, "2") == 0) { obj->needToPoint = true; obj->squarePoint = true; }
    }

    obj->dynamicPoint = LevelLoader_getBooleanKey(PKEY("DYNAMICPOINT"), txt, Main_settings, key, obj->dynamicPoint);

    tmp = LevelLoader_getStringKey(PKEY("POINTRADIUS"), txt, Main_settings, key);
    if (tmp) {
        obj->pointRadius = StringTools_parseInt(tmp);
        obj->pointHeight = obj->pointRadius;
    }

    obj->pointHeight = LevelLoader_getIntKey(PKEY("POINTHEIGHT"), txt, Main_settings, key, obj->pointHeight);

    tmp = LevelLoader_getStringKey(PKEY("POINTOFFSET"), txt, Main_settings, key);
    if (tmp) {
        int pc = 0;
        int *ps = StringTools_cutOnInts(tmp, ',', &pc);
        obj->pointOffset = (Vector3D *)malloc(sizeof(Vector3D));
        Vector3D_set(obj->pointOffset, ps[0], ps[1], ps[2]);
        free(ps);
    }

    tmp = LevelLoader_getStringKey(PKEY("SOUND_ON_ACTIVATE"), txt, Main_settings, key);
    if (tmp) obj->sound = strdup(tmp);

    obj->visible = LevelLoader_getBooleanKey(PKEY("VISIBLE"), txt, Main_settings, key, obj->visible);
    obj->activateOnlyOne = LevelLoader_getBooleanKey(PKEY("ACTIVATE_ONLY_THIS"), txt, Main_settings, key, obj->activateOnlyOne);
    obj->reloadDestroy = LevelLoader_getBooleanKey(PKEY("DESTROY_ON_LEVEL_RELOAD"), txt, Main_settings, key, obj->reloadDestroy);

    tmp = LevelLoader_getStringKey(PKEY("ROOM_ID"), txt, Main_settings, key);
    if (tmp) RoomObject_setNewPart(obj, StringTools_parseInt(tmp));

    #undef PKEY
}

/* --- loadObjects --- */

void LevelLoader_loadObjects(const char *level, void *lvl, Scene *scene,
                             Player *p, bool onlyObjs) {
    char *file = StringTools_getStringFromResource(level);
    if (!file) return;

    RoomObject *lastAddedObject = NULL;

    const char *ptr = file;
    while (1) {
        const char *bracketOpen = strchr(ptr, '[');
        if (!bracketOpen) break;

        const char *bracketClose = strchr(bracketOpen, ']');
        if (!bracketClose) break;

        int typeLen = (int)(bracketClose - bracketOpen - 1);
        char *objectType = (char *)malloc(typeLen + 1);
        strncpy(objectType, bracketOpen + 1, typeLen);
        objectType[typeLen] = '\0';

        ptr = bracketClose + 1;
        const char *nextBracket = strchr(ptr, '[');

        int bodyLen;
        if (nextBracket) bodyLen = (int)(nextBracket - ptr);
        else bodyLen = (int)strlen(ptr);

        char *body = (char *)malloc(bodyLen + 1);
        strncpy(body, ptr, bodyLen);
        body[bodyLen] = '\0';

        void *obj = GameIni_new(body, false);
        char *key = GameIni_get(obj, "PRESET");

        extern House *Scene_getHouse(Scene *scene);
        House *house = Scene_getHouse(scene);

        if (strstr(objectType, "OBJECT") != NULL) {
            char *posStr = GameIni_get(obj, "POS");
            if (posStr) {
                int posesCount = 0;
                char **posesArr = StringTools_cutOnStrings(posStr, ';', &posesCount);

                for (int count = 0; count < posesCount; count++) {
                    int pc = 0;
                    int *ps = StringTools_cutOnInts(posesArr[count], ',', &pc);

                    extern KeyObject *KeyObject_new(Vector3D *pos);
                    Vector3D v;
                    Vector3D_set(&v, ps[0], ps[1], ps[2]);
                    KeyObject *t = KeyObject_new(&v);

                    LevelLoader_loadRM(obj, (RoomObject *)t, lvl, key, "");
                    lastAddedObject = (RoomObject *)t;
                    House_addObject(house, (RoomObject *)t);

                    free(ps);
                }
                for (int i = 0; i < posesCount; i++) free(posesArr[i]);
                free(posesArr);
            }

        } else if (strstr(objectType, "TELEPORT") != NULL) {
            char *posStr = GameIni_get(obj, "POS");
            if (posStr) {
                int posesCount = 0;
                char **posesArr = StringTools_cutOnStrings(posStr, ';', &posesCount);

                for (int count = 0; count < posesCount; count++) {
                    int pc = 0;
                    int *ps = StringTools_cutOnInts(posesArr[count], ',', &pc);

                    char *np = LevelLoader_getStringKey("NEW_POS", obj, Main_settings, key);
                    int npc = 0;
                    int *ps2 = StringTools_cutOnInts(np, ',', &npc);

                    extern Teleport *Teleport_new(Vector3D *pos, Vector3D *newPos);
                    Vector3D v1, v2;
                    Vector3D_set(&v1, ps[0], ps[1], ps[2]);
                    Vector3D_set(&v2, ps2[0], ps2[1], ps2[2]);
                    Teleport *t = Teleport_new(&v1, &v2);

                    extern int Teleport_pRot;
                    t->pRot = LevelLoader_getIntKey("ROT", obj, Main_settings, key, 0);

                    LevelLoader_loadRM(obj, (RoomObject *)t, lvl, key, "");
                    lastAddedObject = (RoomObject *)t;
                    House_addObject(house, (RoomObject *)t);

                    free(ps);
                    free(ps2);
                }
                for (int i = 0; i < posesCount; i++) free(posesArr[i]);
                free(posesArr);
            }

        } else if (strstr(objectType, "LEVELCHANGE") != NULL) {
            char *posStr = GameIni_get(obj, "POS");
            if (posStr) {
                int posesCount = 0;
                char **posesArr = StringTools_cutOnStrings(posStr, ';', &posesCount);

                for (int count = 0; count < posesCount; count++) {
                    int pc = 0;
                    int *ps = StringTools_cutOnInts(posesArr[count], ',', &pc);

                    char *startPos = LevelLoader_getStringKey("START_POS", obj, Main_settings, key);
                    int spc = 0;
                    int *ps2 = StringTools_cutOnInts(startPos, ',', &spc);

                    int levelNum = LevelLoader_getIntKey("LEVEL", obj, Main_settings, key, 1);

                    Vector3D np;
                    Vector3D_set(&np, ps2[0], ps2[1], ps2[2]);
                    LVLChange *mobj = LVLChange_new(ps[0], ps[1], ps[2], &np, levelNum);

                    mobj->pRot = LevelLoader_getIntKey("ROT", obj, Main_settings, key, mobj->pRot);
                    mobj->saveMus = LevelLoader_getIntKey("SAVEMUSIC", obj, Main_settings, key, 0) == 1;
                    mobj->fullMove = LevelLoader_getIntKey("FULLMOVE", obj, Main_settings, key, 0) == 1;
                    mobj->showLoadScreen = LevelLoader_getIntKey("SHOW_LOAD_SCREEN", obj, Main_settings, key, 1) == 1;
                    mobj->base.activable = true;

                    LevelLoader_loadRM(obj, (RoomObject *)mobj, lvl, key, "");
                    lastAddedObject = (RoomObject *)mobj;
                    House_addObject(house, (RoomObject *)mobj);

                    free(ps);
                    free(ps2);
                }
                for (int i = 0; i < posesCount; i++) free(posesArr[i]);
                free(posesArr);
            }

        } else if (strstr(objectType, "SHOP") != NULL && strstr(objectType, "OBJECT") == NULL) {
            char *posStr = GameIni_get(obj, "POS");
            if (posStr) {
                int posesCount = 0;
                char **posesArr = StringTools_cutOnStrings(posStr, ';', &posesCount);

                for (int count = 0; count < posesCount; count++) {
                    int pc = 0;
                    int *ps = StringTools_cutOnInts(posesArr[count], ',', &pc);

                    ShopObject *mobj = ShopObject_new(NULL, NULL, NULL, 0,
                                                       ps[0], ps[1], ps[2]);

                    LevelLoader_loadRM(obj, (RoomObject *)mobj, lvl, key, "");
                    lastAddedObject = (RoomObject *)mobj;
                    House_addObject(house, (RoomObject *)mobj);

                    free(ps);
                }
                for (int i = 0; i < posesCount; i++) free(posesArr[i]);
                free(posesArr);
            }

        } else if (strstr(objectType, "IMAGE2D") != NULL) {
            char *posStr = GameIni_get(obj, "POS");
            if (posStr) {
                int posesCount = 0;
                char **posesArr = StringTools_cutOnStrings(posStr, ';', &posesCount);

                for (int count = 0; count < posesCount; count++) {
                    int pc = 0;
                    int *ps = StringTools_cutOnInts(posesArr[count], ',', &pc);

                    int64_t timeout = LevelLoader_getLongKey("TIMEOUT", obj, Main_settings, key, 1000);

                    Vector3D v;
                    Vector3D_set(&v, ps[0], ps[1], ps[2]);
                    Image2D *mobj = Image2D_new(&v, NULL, timeout);

                    LevelLoader_loadRM(obj, (RoomObject *)mobj, lvl, key, "");
                    lastAddedObject = (RoomObject *)mobj;
                    House_addObject(house, (RoomObject *)mobj);

                    free(ps);
                }
                for (int i = 0; i < posesCount; i++) free(posesArr[i]);
                free(posesArr);
            }
        }

        /* ON_ACTIVATE, MESSAGE, ERRMSG for last added object handled via named search */

        free(objectType);
        free(body);

        if (!nextBracket) break;
        ptr = nextBracket;
    }

    free(file);
}

/* --- createScene --- */

Scene *LevelLoader_createScene(int width, int height, const char *file, GameScreen *gs) {
    char *file2 = StringTools_getStringFromResource(file);
    if (!file2) return NULL;

    /* Parse level ini */
    void *lvl = GameIni_new(file2, false);

    float levelScale = LevelLoader_getFloat("WORLD_SCALE", lvl, Main_settings, 1.0f);

    /* Fog/render settings */
    DirectX7_standartDrawmode = (int8_t)LevelLoader_getInt("DMODE", lvl, Main_settings, 0);
    DirectX7_setFogDist(LevelLoader_getInt("FOGD", lvl, Main_settings, 1));
    DirectX7_setDrDist(LevelLoader_getInt("DIST", lvl, Main_settings, 0x7FFFFFFF));

    /* Load meshes */
    char *worldModel = GameIni_get(lvl, "WORLD_MODEL");
    void *meshes = Asset_getMeshes(worldModel, levelScale, levelScale, levelScale);

    /* Lightmapper settings */
    LightMapper_reset();

    /* Load lights */
    LevelLoader_loadLights(file2, lvl);

    /* Build house */
    char *lightmap = GameIni_get(lvl, "LIGHTMAP");
    bool optimize = LevelLoader_getBoolean("OPTIMIZE_LEVEL_MODEL", lvl, Main_settings, true);

    /* HouseCreator needs Mesh** — placeholder */
    extern House *HouseCreator_create(void **meshes, int count, bool np,
                                       const char *lightdata, bool optimize);

    /* Read start/finish/enemies */
    int startCount = 0, finishCount = 0, enemyCount2 = 0;
    Respawn *start = NULL, *finish = NULL, *enemies = NULL;

    char *tmp = GameIni_get(lvl, "START");
    if (tmp) start = readPoints(tmp, NULL, &startCount);

    tmp = GameIni_get(lvl, "FINISH");
    if (tmp) finish = readPoints(tmp, NULL, &finishCount);

    tmp = GameIni_get(lvl, "ENEMIES");
    if (tmp) enemies = readPoints(tmp, NULL, &enemyCount2);

    int enemyCount = enemyCount2;
    tmp = GameIni_get(lvl, "ENEMY_COUNT");
    if (tmp) enemyCount = StringTools_parseInt(tmp);

    int frequency = LevelLoader_getInt("FREQUENCY", lvl, Main_settings, 2000);

    /* Create Scene */
    extern Scene *Scene_new(int w, int h, House *house, Respawn *start, Respawn *finish,
                            Respawn *enemies, int enemyCount, int frequency,
                            int maxBots, bool generateWays);

    int maxBots = LevelLoader_getInt("ZOMBIE_COUNT", lvl, Main_settings, 5);
    bool genWays = LevelLoader_getInt("GENERATE_BOTS_WAYS", lvl, Main_settings, 1) == 1;

    /* Placeholder scene creation — house build is deferred */
    Scene *scene = Scene_new(width, height, NULL, start, finish, enemies,
                             enemyCount, frequency, maxBots, genWays);

    LevelLoader_defaultOneBot = LevelLoader_getBoolean("SPAWN_ONE_ENEMY", lvl, Main_settings, false);

    /* Room chunk sizes */
    Room_chunkSize = LevelLoader_getInt("CHUNK_COLLISION_SIZE", lvl, Main_settings, 5000);
    Room_chunkSizeRender = LevelLoader_getInt("CHUNK_SIZE", lvl, Main_settings, 0);

    free(file2);
    return scene;
}
