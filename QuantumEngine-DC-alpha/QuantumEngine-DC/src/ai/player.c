/*
 * player.c — porte fiel de code/AI/Player.java
 */
#include "player.h"
#include "tp_pose.h"
#include "zombie.h"
#include "big_zombie.h"
#include "npc.h"
#include "../math/matrix.h"
#include <stdlib.h>
#include <math.h>
#include <limits.h>

typedef struct Arsenal    Arsenal;
typedef struct Camera     Camera;
typedef struct ItemList   ItemList;
typedef struct DirectX7   DirectX7;
typedef struct Scene      Scene;
typedef struct House      House;
typedef struct GameScreen GameScreen;
typedef struct HUDInfo    HUDInfo;
typedef struct Weapon     Weapon;
typedef struct Sprite     Sprite;
typedef struct Matrix     Matrix;
typedef struct RenderObject RenderObject;
typedef struct Sound      Sound;

extern Arsenal   *Arsenal_new(int wG3D, int hG3D);
extern void       Arsenal_destroy(Arsenal *a);
extern int        Arsenal_getCurrent(Arsenal *a);
extern void       Arsenal_setCurrent(Arsenal *a, int c);
extern Weapon    *Arsenal_currentWeapon(Arsenal *a);
extern Weapon   **Arsenal_getWeapons(Arsenal *a, int *out_n);
extern void       Arsenal_next(Arsenal *a);
extern void       Arsenal_previous(Arsenal *a);

extern Weapon    *WeaponCreator_createWeapon(int id);
extern void       Weapon_reset(Weapon *w);
extern int        Weapon_getAmmo(Weapon *w);
extern int        Weapon_getRounds(Weapon *w);
extern void       Weapon_setAmmo(Weapon *w, int ammo);
extern void       Weapon_createSprite(Weapon *w, int wG3D, int hG3D);
extern void       Weapon_renderSplinter(Weapon *w, DirectX7 *g3d);
extern GameObject *Weapon_update(Weapon *w, House *house, Player *player);
extern bool       Weapon_fire(Weapon *w, DirectX7 *g3d);
extern void       Weapon_enableShake(Weapon *w);
extern int        Weapon_getPlayerPose(Weapon *w);

extern Camera    *Camera_new(void);
extern void       Camera_set(Camera *cam, Matrix *plmat, float rotX, float rotY,
                             bool absolutePos, bool absoluteRot);
extern void       Camera_calcPart(Camera *cam, House *house);
extern int        Camera_getPart(Camera *cam);
extern void       Camera_setPart(Camera *cam, int part);
extern Matrix    *Camera_getCamera(Camera *cam);
extern float      Camera_getCurrentRotY(Camera *cam);

extern ItemList  *ItemList_new(void);

extern HUDInfo   *HUDInfo_new(int money, int *ammos, int ammos_n);

extern void       DirectX7_setCamera(DirectX7 *g3d, Matrix *mat);

extern Matrix    *Character_getTransform(Character *ch);
extern void       Character_reset(Character *ch);
extern void       Character_setFly(Character *ch, bool fly);
extern void       Character_moveZ(Character *ch, int amount);
extern void       Character_moveX(Character *ch, int amount);
extern void       Character_moveFree(Character *ch, Vector3D *dir);
extern void       Character_jump2(Character *ch, int height, float speed);
extern void       Character_jumpArcade(Character *ch, int height, float speed);
extern void       Character_set(Character *ch, int radius, int height);
extern int        Character_getRadius(Character *ch);
extern int        Character_getHeight(Character *ch);
extern int        Character_getPart(Character *ch);

extern void       GameObject_setHp(GameObject *self, int hp);
extern int        GameObject_getHp(GameObject *self);
extern void       GameObject_setCharacterSize2(GameObject *self, int radius, int height);
extern void       GameObject_setCharacterSize(GameObject *self, int height);
extern bool       GameObject_isDead(GameObject *self);
extern int        GameObject_getPart(GameObject *self);
extern int        GameObject_getFrame(GameObject *self);
extern int        GameObject_getPosY(GameObject *self);

extern House     *Scene_getHouse(Scene *scene);
extern void     **House_getRooms(House *house);
extern char      *Room_getStepSound(void *room);
extern char      *Room_getJumpSound(void *room);
extern void       Scene_runScript(Scene *scene, void *args);

extern bool       RoomObject_containsSimple(const char *name, const char **vec, int vecCount);

extern int64_t    qe_current_ms(void);

extern Sound     *Asset_getSound(const char *path);
extern void       Sound_setVolume(Sound *snd, int vol);
extern void       Sound_start(Sound *snd);
extern void       Sound_startAt(Sound *snd, int pos);
extern int        Sound_getState(Sound *snd);

extern int        Main_stdFov;
extern int        Main_zoomFov;
extern char     **Main_stepSound;
extern int        Main_stepSound_n;
extern char      *Main_jumpSound;
extern bool       Main_isFootsteps;
extern int        Main_footsteps;
extern int        Main_mouseSpeed;

extern int        DeveloperMenu_fly;
extern bool       DeveloperMenu_debugMode;
extern bool       DeveloperMenu_godMode;

extern int        QFPS_frameTime;

char   **Player_sndWalk = NULL;
int      Player_sndWalk_n = 0;
char    *Player_sndJump = NULL;
bool     Player_arcadeJumpPhysics = false;
bool     Player_fallDamage = true;
int      Player_walkSpeedStatic = 150;
int      Player_radiusOverwrite = -1;
int      Player_heightOverwrite = 0;

Player *Player_new(int wG3D, int hG3D, Vector3D *pos, void *hudInfo, TPPose *scenePose) {
    Player *self = (Player *)calloc(1, sizeof(Player));
    Player_set(self, wG3D, hG3D, pos, hudInfo, scenePose);
    return self;
}

void Player_set(Player *self, int wG3D, int hG3D, Vector3D *pos,
                void *hudInfo, TPPose *scenePose) {
    self->base.base.name = "PLAYER";
    Character_reset(self->base.character);
    Matrix_setPosition(Character_getTransform(self->base.character), 0, 0, 0);
    if (pos != NULL)
        Matrix_setPosition(Character_getTransform(self->base.character), pos->x, pos->y, pos->z);
    GameObject_setHp(&self->base, 100);
    self->frags = 0;
    GameObject_setCharacterSize2(&self->base, 400, 1503);
    self->falldist = 0;
    self->money = 0;
    self->rotateX = 0; self->rotateY = 0;

    self->arsenal = Arsenal_new(wG3D, hG3D);
    if (hudInfo != NULL) {
        HUDInfo *hi = (HUDInfo *)hudInfo;
        extern int HUDInfo_getMoney(HUDInfo *h);
        extern int *HUDInfo_getAmmo(HUDInfo *h, int *out_n);
        self->money = HUDInfo_getMoney(hi);
        int ammos_n = 0;
        int *ammos = HUDInfo_getAmmo(hi, &ammos_n);
        int weapons_n = 0;
        Weapon **weapons = Arsenal_getWeapons(self->arsenal, &weapons_n);

        for (int i = 0; i < weapons_n; i++) {
            if (weapons[i] != NULL) Weapon_reset(weapons[i]);
            if (ammos[i] == -1) weapons[i] = NULL;
            else {
                weapons[i] = WeaponCreator_createWeapon(i);
                Weapon_setAmmo(weapons[i], ammos[i]);
            }
        }
        if (Arsenal_getCurrent(self->arsenal) != -1)
            Weapon_createSprite(Arsenal_currentWeapon(self->arsenal), wG3D, hG3D);
    }

    self->items = ItemList_new();
    Character_setFly(self->base.character, DeveloperMenu_fly > 0);
    self->walkFrame = 0;
    self->attackFrame = INT_MAX;
    self->muzzleFrame = 0;
    self->scenePose = scenePose;

    if (Player_currentPose(self) != NULL) {
        self->cam = Camera_new();
        if (TPPose_meshPoses != NULL) TPPose_applyRenderModes(TPPose_meshPoses, TPPose_meshPoses_n);
        if (scenePose != NULL) {
            TPPose *arr[1] = { scenePose };
            TPPose_applyRenderModes(arr, 1);
        }
        if (TPPose_radius != 0) GameObject_setCharacterSize2(&self->base, TPPose_radius, TPPose_height);
    }

    if (Player_radiusOverwrite != -1)
        Character_set(self->base.character, Player_radiusOverwrite, Character_getHeight(self->base.character));
    if (Player_heightOverwrite != 0)
        Character_set(self->base.character, Character_getRadius(self->base.character), Player_heightOverwrite);
}

void Player_destroy(Player *self) {
    self->money = 0;
}

void Player_render(Player *self, DirectX7 *g3d, int x1, int y1, int x2, int y2) {
    if (Main_stepSound != NULL && Main_isFootsteps && Main_footsteps != 0) {
        if ((self->base.character->speed.x != 0 || self->base.character->speed.z != 0) &&
            self->base.character->onFloor == true &&
            qe_current_ms() - self->lastStep > 450 &&
            !self->base.character->fly) {

            char **soundz = Main_stepSound;
            int soundz_n = Main_stepSound_n;
            if (Player_sndWalk != NULL) { soundz = Player_sndWalk; soundz_n = Player_sndWalk_n; }
            if (soundz != NULL && self->stepIndex >= soundz_n) self->stepIndex = 0;

            if (soundz != NULL) {
                Sound *snd = Asset_getSound(soundz[self->stepIndex]);
                if (snd != NULL) {
                    Sound_setVolume(snd, Main_footsteps);
                    Sound_startAt(snd, 0);
                    self->stepIndex++;
                }
            }
            self->lastStep = qe_current_ms();
        }
    }

    if (Arsenal_currentWeapon(self->arsenal) != NULL)
        Weapon_renderSplinter(Arsenal_currentWeapon(self->arsenal), g3d);

    if (Player_currentPose(self) != NULL) {
        TPPose_draw(Player_currentPose(self), self, g3d, x1, y1, x2, y2);
    }
}

void Player_nextWeapon(Player *self) {
    int old = Arsenal_getCurrent(self->arsenal);
    Arsenal_next(self->arsenal);
    if (old != Arsenal_getCurrent(self->arsenal)) {
        self->attackFrame = INT_MAX;
        self->muzzleFrame = 0;
    }
}

void Player_previousWeapon(Player *self) {
    int old = Arsenal_getCurrent(self->arsenal);
    Arsenal_previous(self->arsenal);
    if (old != Arsenal_getCurrent(self->arsenal)) {
        self->attackFrame = INT_MAX;
        self->muzzleFrame = 0;
    }
}

void Player_update(Player *self, Scene *scene) {
    GameObject_update(&self->base);

    House *house = Scene_getHouse(scene);
    void **rooms = House_getRooms(house);
    int part = GameObject_getPart(&self->base);
    Player_sndWalk = NULL; Player_sndWalk_n = 0;
    Player_sndJump = NULL;

    char *stepSnd = Room_getStepSound(rooms[part]);
    if (stepSnd != NULL) { Player_sndWalk = &stepSnd; Player_sndWalk_n = 1; }
    Player_sndJump = Room_getJumpSound(rooms[part]);

    if (Player_fallDamage) {
        if (self->base.character->onFloor == false && qe_current_ms() - self->lastYCheck < 1000) {
            self->falldist += self->lastY - GameObject_getPosY(&self->base);
        }
        if (self->falldist > 7000 && self->base.character->onFloor == true) {
            self->damaged = true;
            Player_damage(self, NULL, (self->falldist - 7000) / 40);
        }
        if (self->falldist > 11000 && self->base.character->onFloor == false) {
            self->damaged = true;
            Player_damage(self, NULL, 100);
        }
        self->lastYCheck = qe_current_ms();
        self->lastY = GameObject_getPosY(&self->base);
        if (self->base.character->onFloor || self->base.character->fly) self->falldist = 0;
    }

    if (Arsenal_getCurrent(self->arsenal) != -1) {
        if (Arsenal_currentWeapon(self->arsenal) != NULL) {
            GameObject *obj = Weapon_update(Arsenal_currentWeapon(self->arsenal), house, self);
            if (obj != NULL) {
                Zombie *z = (Zombie *)obj;
                BigZombie *bz = (BigZombie *)obj;
                NPC *npc = (NPC *)obj;
                (void)z; (void)bz; (void)npc;
            }
        }
    }

    if (self->cam != NULL) {
        if (Player_currentPose(self) != NULL)
            TPPose_update(Player_currentPose(self), self->cam, self);
        Camera_calcPart(self->cam, house);
        if (Camera_getPart(self->cam) == -1 && GameObject_getPart(&self->base) != -1)
            Camera_setPart(self->cam, GameObject_getPart(&self->base));
    }
}

TPPose *Player_currentPose(Player *self) {
    if (self->scenePose != NULL) return self->scenePose;
    if (TPPose_meshPoses == NULL) return NULL;
    Weapon *wp = Arsenal_currentWeapon(self->arsenal);
    if (wp == NULL) return TPPose_meshPoses[0];
    return TPPose_meshPoses[Weapon_getPlayerPose(wp)];
}

bool Player_canWalk(Player *self) {
    TPPose *tp = Player_currentPose(self);
    if (tp == NULL) return true;
    return TPPose_canWalk(tp, self);
}

bool Player_canJump(Player *self) {
    TPPose *tp = Player_currentPose(self);
    if (tp == NULL) return true;
    return TPPose_canJump(tp, self);
}

bool Player_canLookX(Player *self) {
    TPPose *tp = Player_currentPose(self);
    if (tp == NULL) return true;
    return TPPose_canLookX(tp, self);
}

bool Player_canLookY(Player *self) {
    TPPose *tp = Player_currentPose(self);
    if (tp == NULL) return true;
    return TPPose_canLookY(tp, self);
}

bool Player_canAttack(Player *self) {
    TPPose *tp = Player_currentPose(self);
    if (tp == NULL) return true;
    return TPPose_canAttack(tp, self);
}

bool Player_canAttackSight(Player *self) {
    TPPose *tp = Player_currentPose(self);
    if (tp == NULL) return true;
    return tp->canAttackSight;
}

bool Player_canAttackNoSight(Player *self) {
    TPPose *tp = Player_currentPose(self);
    if (tp == NULL) return true;
    return tp->canAttack;
}

bool Player_isSwapStrafeLook(Player *self) {
    TPPose *tp = Player_currentPose(self);
    if (tp == NULL) return false;
    return TPPose_isSwapStrafeLook(tp, self);
}

bool Player_isRotToWalkDir(Player *self) {
    TPPose *tp = Player_currentPose(self);
    if (tp == NULL) return false;
    return TPPose_isRotToWalkDir(tp, self);
}

bool Player_show2D(Player *self) {
    TPPose *tp = Player_currentPose(self);
    if (tp == NULL) return true;
    return TPPose_show2D(tp, self);
}

float Player_lookSpeed(Player *self) {
    TPPose *tp = Player_currentPose(self);
    if (tp == NULL) return (self->zoom ? 0.71f : 1.0f);
    return TPPose_lookSpeedVal(tp, self);
}

int Player_walkSpeedVal(Player *self) {
    TPPose *tp = Player_currentPose(self);
    if (tp == NULL) return Player_walkSpeedStatic;
    return TPPose_walkSpeedVal(tp, self);
}

void Player_setCamera(Player *self, DirectX7 *g3d) {
    Matrix *playerMat = Character_getTransform(self->base.character);
    int playerHeight = Player_getEyesHeight(self);
    if (self->cam == NULL) {
        playerMat->m13 += playerHeight;
        DirectX7_setCamera(g3d, playerMat);
        playerMat->m13 -= playerHeight;
    } else {
        DirectX7_setCamera(g3d, Camera_getCamera(self->cam));
    }
}

Camera *Player_getCamera(Player *self) {
    return self->cam;
}

int Player_getRenderPart(Player *self, Scene *scene) {
    if (self->cam == NULL) {
        return GameObject_getPart(&self->base);
    } else {
        return Camera_getPart(self->cam);
    }
}

int Player_getEyesHeight(Player *self) {
    int height = 1503;

    if (GameObject_isDead(&self->base)) {
        float div = 0.4f * (float)GameObject_getFrame(&self->base);
        if (div < 1.0f) div = 1.0f;
        height = (int)(height / div);
        if (height < Character_getRadius(self->base.character))
            height = Character_getRadius(self->base.character);
    }

    return height;
}

bool Player_damage(Player *self, GameObject *obj, int dmg) {
    if (DeveloperMenu_debugMode && dmg > 0 && DeveloperMenu_godMode) return true;
    if (dmg > 0) self->damaged = true;
    if (dmg < 0 && GameObject_getHp(&self->base) - dmg > 100) {
        GameObject_damage(&self->base, -(100 - GameObject_getHp(&self->base)));
        return true;
    }

    GameObject_damage(&self->base, dmg);
    return true;
}

bool Player_isDamaged(Player *self) {
    bool tmp = self->damaged;
    self->damaged = false;
    return tmp;
}

void Player_pay(Player *self, int price) {
    self->money -= price;
}

bool Player_isTimeToRenew(Player *self) {
    return GameObject_isDead(&self->base) && (qe_current_ms() - self->base.DeathTime) > 3000;
}

void Player_fire(Player *self, DirectX7 *g3d) {
    if (Arsenal_getCurrent(self->arsenal) != -1 && Player_canAttack(self)) {
        if (Weapon_fire(Arsenal_currentWeapon(self->arsenal), g3d)) {
            self->attackFrame = 0;
            TPPose *tp = Player_currentPose(self);
            if (tp != NULL) self->muzzleFrame = tp->muzzleFlashTimer;
        }
    }
}

void Player_jump(Player *self) {
    if (!Player_canJump(self)) return;
    if (!Player_arcadeJumpPhysics)
        Character_jump2(self->base.character, 150, 1.2f);
    else
        Character_jumpArcade(self->base.character, 150, 1.2f);

    if (self->base.character->onFloor && Main_isFootsteps && Main_footsteps != 0) {
        char *snd = Main_jumpSound;
        if (Player_sndJump != NULL) snd = Player_sndJump;
        if (snd != NULL) {
            Sound *s = Asset_getSound(snd);
            if (s != NULL && Sound_getState(s) != 400) {
                Sound_setVolume(s, Main_footsteps);
                Sound_start(s);
            }
        }
    }
}

void Player_updateMatrix(Player *self) {
    int x = self->base.character->transform.m03;
    int y = self->base.character->transform.m13;
    int z = self->base.character->transform.m23;

    if (self->rotateX < -80) self->rotateX = -80;
    if (self->rotateX > 80) self->rotateX = 80;

    while (self->rotateY > 360) self->rotateY -= 360;
    while (self->rotateY < 0) self->rotateY += 360;

    Matrix_setIdentity(&self->base.character->transform);
    Matrix_setRotX(&self->base.character->transform, (int)self->rotateX);
    Matrix_setPosition(&self->base.character->transform, x, y, z);
    Matrix_rotY(&self->base.character->transform, (int)self->rotateY);
}

void Player_rotYn(Player *self, float i) {
    if (!Player_canLookY(self)) return;
    self->rotateY += i;
    Player_updateMatrix(self);
}

void Player_rotXn(Player *self, float i) {
    if (!Player_canLookX(self)) return;
    self->rotateX += i;
    Player_updateMatrix(self);
}

void Player_rotLeft(Player *self) {
    if (!Player_canLookY(self)) return;
    Player_rotYn(self, (7.0f * Player_lookSpeed(self) * Main_mouseSpeed / 50.0f) * QFPS_frameTime / 50.0f);
}

void Player_rotRight(Player *self) {
    if (!Player_canLookY(self)) return;
    Player_rotYn(self, -(7.0f * Player_lookSpeed(self) * Main_mouseSpeed / 50.0f) * QFPS_frameTime / 50.0f);
}

void Player_rotUp(Player *self) {
    if (!Player_canLookX(self)) return;
    Player_rotXn(self, (7.0f * Player_lookSpeed(self) * Main_mouseSpeed / 100.0f) * QFPS_frameTime / 50.0f);
}

void Player_rotDown(Player *self) {
    if (!Player_canLookX(self)) return;
    Player_rotXn(self, -(7.0f * Player_lookSpeed(self) * Main_mouseSpeed / 100.0f) * QFPS_frameTime / 50.0f);
}

void Player_walk(Player *self, int right, int forward) {
    if (!Player_canWalk(self)) return;

    int ws = Player_walkSpeedVal(self);
    if (self->base.character->fly) ws = ws * 7 / 3;

    if (self->cam != NULL && Player_isRotToWalkDir(self)) {
        float modelRot = Camera_getCurrentRotY(self->cam);
        if (forward < 0) modelRot += 180;
        modelRot -= right * 90 / (forward != 0 ? (forward > 0 ? 2 : -2) : 1);

        Vector3D dir;
        dir.x = (int)(-sin(modelRot * 3.14159265358979 / 180.0) * ws);
        dir.y = 0;
        dir.z = (int)(-cos(modelRot * 3.14159265358979 / 180.0) * ws);

        if (self->base.character->fly || self->base.character->onFloor) {
            self->rotateY = modelRot;
            Player_updateMatrix(self);
            Character_moveFree(self->base.character, &dir);
        }
    } else {
        Character_moveZ(self->base.character, -ws * forward);
        Character_moveX(self->base.character, ws * right);
    }

    if ((forward != 0 || right != 0) && Arsenal_currentWeapon(self->arsenal) != NULL)
        Weapon_enableShake(Arsenal_currentWeapon(self->arsenal));
}

void *Player_getHUDInfo(Player *self) {
    int weapons_n = 0;
    Weapon **weapons = Arsenal_getWeapons(self->arsenal, &weapons_n);
    int *ammos = (int *)malloc(weapons_n * sizeof(int));

    for (int i = 0; i < weapons_n; i++) {
        if (weapons[i] != NULL)
            ammos[i] = Weapon_getAmmo(weapons[i]) + Weapon_getRounds(weapons[i]);
        else
            ammos[i] = -1;
    }

    HUDInfo *hi = HUDInfo_new(self->money, ammos, weapons_n);
    free(ammos);
    return hi;
}

void Player_activate(Player *self, House *house, Player *player2, GameScreen *gs) {
    (void)self; (void)house; (void)player2; (void)gs;
}

void Player_copyNewToUsed(Player *self) {
    (void)self;
    extern void   **Player_toAddOnStart;
    extern int      Player_toAddOnStart_n;
    extern void   **Player_usedPoints;
    extern int      Player_usedPoints_n;
    extern int      Player_usedPoints_cap;

    if (Player_toAddOnStart_n == 0) return;

    while (Player_toAddOnStart_n > 0) {
        const char *elem = (const char *)Player_toAddOnStart[0];
        if (!RoomObject_containsSimple(elem, (const char **)Player_usedPoints, Player_usedPoints_n)) {
            if (Player_usedPoints_n >= Player_usedPoints_cap) {
                Player_usedPoints_cap = Player_usedPoints_cap == 0 ? 8 : Player_usedPoints_cap * 2;
                Player_usedPoints = (void **)realloc(Player_usedPoints,
                    Player_usedPoints_cap * sizeof(void *));
            }
            Player_usedPoints[Player_usedPoints_n++] = Player_toAddOnStart[0];
        }
        for (int i = 0; i < Player_toAddOnStart_n - 1; i++)
            Player_toAddOnStart[i] = Player_toAddOnStart[i + 1];
        Player_toAddOnStart_n--;
    }
}
