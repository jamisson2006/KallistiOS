/*
 * room_object.h — porte fiel de code/Gameplay/Objects/RoomObject.java (Quantum Engine J2ME)
 * @author Roman Lahin
 *
 * Classe base para todos os objetos interativeis no mapa.
 * 1032 linhas no original Java.
 */
#ifndef QE_GAMEPLAY_MAP_ROOM_OBJECT_H
#define QE_GAMEPLAY_MAP_ROOM_OBJECT_H

#include <stdbool.h>
#include <stdint.h>

/* Forward declarations */
typedef struct House      House;
typedef struct Player     Player;
typedef struct GameScreen GameScreen;
typedef struct Scene      Scene;
typedef struct DirectX7   DirectX7;
typedef struct Vector3D   Vector3D;
typedef struct Character  Character;
typedef struct Matrix     Matrix;

/* ------------------------------------------------------------------ */
/* RoomObject — struct base                                            */
/* ------------------------------------------------------------------ */
typedef struct RoomObject {
    /* --- flags basicos --- */
    bool activable;
    bool clickable;
    bool singleUse;
    bool destroyOnUse;

    /* --- strings --- */
    char *name;
    char *desc;
    char *sound;
    char *message;
    char *errMessage;

    /* --- need array (condicao para ativar) --- */
    char **need;
    int    needCount;

    /* --- additional array (script executado ao ativar) --- */
    char **additional;
    int    additionalCount;

    /* --- tempos e delays --- */
    int64_t radius;
    int64_t messageTimeOut;
    int64_t errMessageTimeOut;
    int64_t messageDelay;
    int64_t errMessageDelay;

    /* --- tipos de mensagem (0=dialog, 1=timed, 2=paused) --- */
    int messageType;
    int errMessageType;

    /* --- estado --- */
    bool    near;
    bool    activated;
    bool    visible;
    bool    alwaysActivate;
    bool    activateOnlyOne;
    bool    reloadDestroy;
    int64_t lastActivate;
    int64_t timeToReset;

    /* --- point/display --- */
    bool hideWhenUnusable;
    bool needToPoint;
    bool squarePoint;
    bool dynamicPoint;
    int  pointRadius;
    int  pointHeight;
    Vector3D *pointOffset;

    /* --- spawner --- */
    int spawnerId;

    /* --- rayCast --- */
    bool ignoreWeaponRayCast;

    /* --- character (contem transform com posicao, part, etc.) --- */
    Character *character;

} RoomObject;

/* ------------------------------------------------------------------ */
/* Funcoes de posicao                                                  */
/* ------------------------------------------------------------------ */
int  RoomObject_getPosX(RoomObject *self);
int  RoomObject_getPosY(RoomObject *self);
int  RoomObject_getPosZ(RoomObject *self);

/* part — armazenado no character */
int  RoomObject_getPart(RoomObject *self);
void RoomObject_setPart(RoomObject *self, int part, House *house);
void RoomObject_setNewPart(RoomObject *self, int part);

/* ------------------------------------------------------------------ */
/* Ativacao e verificacao                                              */
/* ------------------------------------------------------------------ */
void RoomObject_activate(RoomObject *self, Player *player, House *house, GameScreen *gs);
bool RoomObject_check(RoomObject *self, DirectX7 *g3d, int yy);

/* ------------------------------------------------------------------ */
/* Sistema de scripting                                                */
/* ------------------------------------------------------------------ */
void    RoomObject_give(char **script, int scriptCount, Player *player, House *house, GameScreen *gs);
int64_t RoomObject_readVar(const char *expression, Player *player, House *house, GameScreen *gs);
bool    RoomObject_readBoolean(const char *expression, Player *player, House *house, GameScreen *gs);
char   *RoomObject_readString(const char *expression, Player *player, House *house, GameScreen *gs);
void    RoomObject_getSetValue(const char *property, RoomObject *target, Player *player, House *house, GameScreen *gs);

/* ------------------------------------------------------------------ */
/* Coleta / condicoes                                                  */
/* ------------------------------------------------------------------ */
bool RoomObject_isAllCollected(RoomObject *self, Player *player, House *house, GameScreen *gs);
bool RoomObject_containsCollected(const char *s, Player *player, House *house, GameScreen *gs);
bool RoomObject_containsSimple(const char *name, const char **vec, int vecCount);

/* ------------------------------------------------------------------ */
/* Debug draw                                                          */
/* ------------------------------------------------------------------ */
void RoomObject_drawDebug(RoomObject *self, DirectX7 *g3d);
void RoomObject_drawDebugSquare(RoomObject *self, DirectX7 *g3d);

#endif
