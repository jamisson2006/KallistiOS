/*
 * player.h — porte fiel de code/AI/Player.java
 */
#ifndef QE_AI_PLAYER_H
#define QE_AI_PLAYER_H

#include "../gameplay/objects/game_object.h"
#include "../math/vector3d.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct Arsenal   Arsenal;
typedef struct Camera    Camera;
typedef struct TPPose    TPPose;
typedef struct ItemList  ItemList;
typedef struct DirectX7  DirectX7;
typedef struct Scene     Scene;
typedef struct House     House;
typedef struct GameScreen GameScreen;
typedef struct HUDInfo   HUDInfo;
typedef struct Weapon    Weapon;

typedef struct Player {
    GameObject  base;

    int         money;
    int         frags;
    int         stepIndex;
    Arsenal    *arsenal;
    bool        damaged;

    bool        zoom;
    float       fov;
    int         stdFov;
    int         zoomFov;
    int64_t     lastZoomAction;
    int         falldist;
    int64_t     lastStep;
    int64_t     lastYCheck;
    int         lastY;
    float       rotateX;
    float       rotateY;

    ItemList   *items;

    int         walkFrame;
    int         attackFrame;
    int         muzzleFrame;
    Camera     *cam;
    TPPose     *scenePose;
} Player;

extern char   **Player_sndWalk;
extern int      Player_sndWalk_n;
extern char    *Player_sndJump;
extern bool     Player_arcadeJumpPhysics;
extern bool     Player_fallDamage;
extern int      Player_walkSpeedStatic;
extern int      Player_radiusOverwrite;
extern int      Player_heightOverwrite;

Player *Player_new(int wG3D, int hG3D, Vector3D *pos, void *hudInfo, TPPose *scenePose);
void    Player_set(Player *self, int wG3D, int hG3D, Vector3D *pos,
                   void *hudInfo, TPPose *scenePose);
void    Player_destroy(Player *self);
void    Player_render(Player *self, DirectX7 *g3d, int x1, int y1, int x2, int y2);
void    Player_update(Player *self, Scene *scene);

TPPose *Player_currentPose(Player *self);
bool    Player_canWalk(Player *self);
bool    Player_canJump(Player *self);
bool    Player_canLookX(Player *self);
bool    Player_canLookY(Player *self);
bool    Player_canAttack(Player *self);
bool    Player_canAttackSight(Player *self);
bool    Player_canAttackNoSight(Player *self);
bool    Player_isSwapStrafeLook(Player *self);
bool    Player_isRotToWalkDir(Player *self);
bool    Player_show2D(Player *self);
float   Player_lookSpeed(Player *self);
int     Player_walkSpeedVal(Player *self);

void    Player_setCamera(Player *self, DirectX7 *g3d);
Camera *Player_getCamera(Player *self);
int     Player_getRenderPart(Player *self, Scene *scene);
int     Player_getEyesHeight(Player *self);

bool    Player_damage(Player *self, GameObject *obj, int dmg);
bool    Player_isDamaged(Player *self);
void    Player_pay(Player *self, int price);
bool    Player_isTimeToRenew(Player *self);

void    Player_fire(Player *self, DirectX7 *g3d);
void    Player_jump(Player *self);
void    Player_updateMatrix(Player *self);

void    Player_rotYn(Player *self, float i);
void    Player_rotXn(Player *self, float i);
void    Player_rotLeft(Player *self);
void    Player_rotRight(Player *self);
void    Player_rotUp(Player *self);
void    Player_rotDown(Player *self);
void    Player_walk(Player *self, int right, int forward);

void    Player_nextWeapon(Player *self);
void    Player_previousWeapon(Player *self);

void   *Player_getHUDInfo(Player *self);
void    Player_activate(Player *self, House *house, Player *player2, GameScreen *gs);
void    Player_copyNewToUsed(Player *self);

#endif
