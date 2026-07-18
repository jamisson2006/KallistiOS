/*
 * tp_pose.h — porte fiel de code/AI/TPPose.java
 */
#ifndef QE_AI_TP_POSE_H
#define QE_AI_TP_POSE_H

#include <stdbool.h>
#include <stdint.h>
#include "../math/matrix.h"

typedef struct Morphing     Morphing;
typedef struct MultyTexture MultyTexture;
typedef struct Mesh         Mesh;
typedef struct Sprite       Sprite;
typedef struct Vertex       Vertex;
typedef struct Texture      Texture;
typedef struct DirectX7     DirectX7;
typedef struct Camera       Camera;
typedef struct GameIni      GameIni;
typedef struct Player       Player;

typedef struct TPPose {
    char       *poseName;
    Morphing   *walk;
    Morphing   *attack;
    Morphing   *secondWalk;
    Morphing   *secondAttack;
    Morphing   *walkSight;
    Morphing   *attackSight;
    Morphing   *secondWalkSight;
    Morphing   *secondAttackSight;
    MultyTexture *secondMt;
    int8_t     *drawModesSecond;
    int         drawModesSecond_n;
    Sprite     *muzzleFlash;
    Vertex     *muzzleFlashPos;
    int         muzzleFlashTimer;
    int         animationSpeedAttack;
    int         animationSpeed;

    bool show3D, show2D, show3DSight, show2DSight, showSecond, showSecondSight;
    bool canWalk, canWalkSight, canJump, canJumpSight;
    bool canLookX, canLookXSight, canLookY, canLookYSight;
    bool canAttack, canAttackSight;
    bool swapStrafeLook, swapStrafeLookSight;
    bool rotToWalkDir, rotToWalkDirSight;
    bool camAbsolutePos, camAbsolutePosSight;
    bool camAbsoluteRot, camAbsoluteRotSight;

    int camX, camY, camZ, camRotX, camRotY;
    int camXSight, camYSight, camZSight, camRotXSight, camRotYSight;
    int camSmoothSteps, camSmoothStepsSight;

    bool rotModelX, rotModelXSight;
    int  rotModelY, rotModelYSight;

    float lookSpeed, lookSpeedSight;
    int   walkSpeed, walkSpeedSight;
} TPPose;

extern TPPose **TPPose_meshPoses;
extern int      TPPose_meshPoses_n;
extern MultyTexture *TPPose_mt;
extern Mesh   **TPPose_models;
extern int      TPPose_models_n;
extern Mesh    *TPPose_clone;
extern int8_t  *TPPose_defDrawModes;
extern int      TPPose_defDrawModes_n;
extern bool     TPPose_inited;
extern int      TPPose_radius;
extern int      TPPose_height;

void    TPPose_init(void);
TPPose *TPPose_loadPoseExternal(const char *path);
void    TPPose_applyRenderModes(TPPose **poses, int poses_n);

TPPose *TPPose_new(const char *poseName, Mesh **meshes, int meshes_n,
                   Mesh *clone, GameIni *ini, GameIni *def);

bool    TPPose_show2D(TPPose *self, Player *player);
bool    TPPose_canWalk(TPPose *self, Player *player);
bool    TPPose_canJump(TPPose *self, Player *player);
bool    TPPose_canLookX(TPPose *self, Player *player);
bool    TPPose_canLookY(TPPose *self, Player *player);
bool    TPPose_canAttack(TPPose *self, Player *player);
bool    TPPose_isSwapStrafeLook(TPPose *self, Player *player);
bool    TPPose_isRotToWalkDir(TPPose *self, Player *player);

void    TPPose_update(TPPose *self, Camera *cam, Player *player);
void    TPPose_draw(TPPose *self, Player *player, DirectX7 *g3d,
                    int x1, int y1, int x2, int y2);

float   TPPose_lookSpeedVal(TPPose *self, Player *player);
int     TPPose_walkSpeedVal(TPPose *self, Player *player);

#endif
