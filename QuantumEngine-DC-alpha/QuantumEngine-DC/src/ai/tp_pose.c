/*
 * tp_pose.c — porte fiel de code/AI/TPPose.java
 */
#include "tp_pose.h"
#include "player.h"
#include "../math/matrix.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

typedef struct Morphing     Morphing;
typedef struct MultyTexture MultyTexture;
typedef struct Mesh         Mesh;
typedef struct MeshClone    MeshClone;
typedef struct Sprite       Sprite;
typedef struct Vertex       Vertex;
typedef struct Texture      Texture;
typedef struct DirectX7     DirectX7;
typedef struct Camera       Camera;
typedef struct GameIni      GameIni;
typedef struct RenderObject RenderObject;
typedef struct RImg         RImg;

extern bool     Main_isExist(const char *path);
extern void    *GameIni_createGroups(const char *path, char ***out_names, int *out_n);
extern GameIni **GameIni_getGroups(void *obj);
extern char    *GameIni_get(GameIni *ini, const char *key);
extern char    *GameIni_getDef(GameIni *ini, const char *key, const char *def);
extern int      GameIni_getInt(GameIni *ini, const char *key, int def);
extern float    GameIni_getFloat(GameIni *ini, const char *key, float def);
extern int     *GameIni_cutOnInts(const char *str, char sep, int *out_n);
extern char   **GameIni_cutOnStrings(const char *str, char sep1, char sep2, int *out_n);

extern Mesh   **Room_loadMeshes(const char *path, float sx, float sy, float sz, int *out_n);
extern Mesh   **Asset_getMeshes(const char *path, float sx, float sy, float sz, int *out_n);
extern Texture *Asset_getTexture(const char *path);

extern MultyTexture *MultyTexture_new(const char *path, bool repeat);
extern Texture     **MultyTexture_getTextures(MultyTexture *mt, int *out_n);

extern MeshClone *MeshClone_new(Mesh *mesh);
extern Mesh      *MeshClone_copy(MeshClone *mc);
extern void       MeshClone_destroy(MeshClone *mc);

extern int  Mesh_maxX(Mesh *m);
extern int  Mesh_minX(Mesh *m);
extern int  Mesh_maxY(Mesh *m);
extern int  Mesh_minY(Mesh *m);
extern int  Mesh_maxZ(Mesh *m);
extern int  Mesh_minZ(Mesh *m);
extern void Mesh_increaseMeshSz(Mesh *m, int z);

extern short **Morphing_create(Mesh **models, int models_n, int start, int end);
extern Morphing *Morphing_new(short **anim, int anim_n, Mesh *clone);
extern void     Morphing_setFrame(Morphing *m, int frame);
extern void     Morphing_setFrameNI(Morphing *m, int frame);
extern int      Morphing_getFrame(Morphing *m);
extern int      Morphing_getMaxFrame(Morphing *m);
extern Mesh    *Morphing_getMesh(Morphing *m);
extern void     Morphing_setMorphEnabled(Morphing *m, bool enabled);

extern Sprite  *Sprite_new(Texture *tex, int scale);
extern void     Sprite_setMode(Sprite *spr, int mode);
extern void     Sprite_setFog(Sprite *spr, bool fog);
extern int      Sprite_getHeight(Sprite *spr);
extern Vector3D *Sprite_getPos(Sprite *spr);
extern void     Sprite_project(Sprite *spr, Matrix *invCam, DirectX7 *g3d);

extern Vertex  *Vertex_new_ints(int *vals, int n);
extern void     Vertex_transform(Vertex *v, const Matrix *mat);

extern void     DirectX7_transformAndProjectVertices(DirectX7 *g3d, Mesh *mesh, Matrix *fmat);
extern Matrix  *DirectX7_computeFinalMatrix(DirectX7 *g3d, Matrix *mat);
extern Matrix  *DirectX7_getInvCamera(DirectX7 *g3d);
extern void     DirectX7_addMesh(DirectX7 *g3d, Mesh *mesh, int x1, int y1, int x2, int y2, MultyTexture *mt);
extern void     DirectX7_addRenderObject(DirectX7 *g3d, void *obj, int x1, int y1, int x2, int y2);
extern int      DirectX7_standartDrawmode;

extern void     Camera_set(Camera *cam, Matrix *plmat, float rotX, float rotY,
                           bool absolutePos, bool absoluteRot);

extern int      QFPS_frameTime;

extern bool     Texture_isAlphaMixing(Texture *tex);
extern RImg    *Texture_getRImg(Texture *tex);

extern int8_t   StringTools_parseByte(const char *s);

TPPose **TPPose_meshPoses = NULL;
int      TPPose_meshPoses_n = 0;
MultyTexture *TPPose_mt = NULL;
Mesh   **TPPose_models = NULL;
int      TPPose_models_n = 0;
Mesh    *TPPose_clone = NULL;
int8_t  *TPPose_defDrawModes = NULL;
int      TPPose_defDrawModes_n = 0;
bool     TPPose_inited = false;
int      TPPose_radius = 0;
int      TPPose_height = 0;

static Matrix s_mat;
static Matrix s_tmp;

static int8_t *loadModes(const char *tmp, int *out_n) {
    int cut_n = 0;
    char **cut = GameIni_cutOnStrings(tmp, ',', ';', &cut_n);
    int8_t *out = (int8_t *)malloc(cut_n * sizeof(int8_t));
    *out_n = cut_n;

    for (int i = 0; i < cut_n; i++) {
        if (strcmp(cut[i], "std") == 0) out[i] = INT8_MIN;
        else out[i] = StringTools_parseByte(cut[i]);
    }

    return out;
}

static void applyModes(Texture **texs, int texs_n, int8_t *modes, int modes_n) {
    int len = texs_n > modes_n ? modes_n : texs_n;

    for (int i = 0; i < len; i++) {
        extern void Texture_setDrawmode(Texture *t, int8_t dm);
        if (modes[i] == INT8_MIN) Texture_setDrawmode(texs[i], (int8_t)DirectX7_standartDrawmode);
        Texture_setDrawmode(texs[i], modes[i]);
    }
}

static Morphing *makeMorphing(const char *animName, Mesh **models, int models_n,
                              Mesh *clone, GameIni *ini, GameIni *def, bool createIfNo) {
    if (models == NULL || clone == NULL) return NULL;

    int walkStart = 0, walkEnd = 1;
    char *tmp = GameIni_getDef(ini, animName, GameIni_get(def, animName));
    if (tmp != NULL) {
        int cycle_n = 0;
        int *cycle = GameIni_cutOnInts(tmp, '-', &cycle_n);
        walkStart = cycle[0];
        walkEnd = cycle[cycle_n == 1 ? 0 : 1] + 1;
        free(cycle);
    } else if (!createIfNo) return NULL;

    short **anim = Morphing_create(models, models_n, walkStart, walkEnd);
    return Morphing_new(anim, walkEnd - walkStart, clone);
}

void TPPose_init(void) {
    TPPose_inited = true;
    if (!Main_isExist("/thirdperson.txt")) return;

    char **names = NULL;
    int groups_n = 0;
    void *obj = GameIni_createGroups("/thirdperson.txt", &names, &groups_n);
    GameIni **groups = GameIni_getGroups(obj);

    TPPose_meshPoses_n = groups_n - 1;
    TPPose_meshPoses = (TPPose **)calloc(TPPose_meshPoses_n, sizeof(TPPose *));

    GameIni *set = groups[0];
    float scale = GameIni_getFloat(set, "SCALE", 1.0f);

    char *tmp = GameIni_get(set, "MODEL");
    if (tmp != NULL) {
        TPPose_models = Room_loadMeshes(tmp, scale, scale, scale, &TPPose_models_n);
        TPPose_mt = MultyTexture_new(GameIni_get(set, "TEX"), false);

        MeshClone *mc = MeshClone_new(TPPose_models[0]);
        TPPose_clone = MeshClone_copy(mc);
        MeshClone_destroy(mc);

        int sizex = Mesh_maxX(TPPose_clone) - Mesh_minX(TPPose_clone);
        int sizez = Mesh_maxZ(TPPose_clone) - Mesh_minZ(TPPose_clone);
        TPPose_radius = (int)(sqrt((double)(sizex * sizex + sizez * sizez)) / 2.0);
        TPPose_height = Mesh_maxY(TPPose_clone) - Mesh_minY(TPPose_clone);
    }

    tmp = GameIni_get(set, "DRAW_MODES");
    if (tmp != NULL) TPPose_defDrawModes = loadModes(tmp, &TPPose_defDrawModes_n);

    for (int i = 0; i < TPPose_meshPoses_n; i++) {
        TPPose_meshPoses[i] = TPPose_new(names[i + 1], TPPose_models, TPPose_models_n,
                                         TPPose_clone, groups[i + 1], groups[1]);
    }
}

TPPose *TPPose_loadPoseExternal(const char *path) {
    if (!Main_isExist(path)) return NULL;

    char **names = NULL;
    int groups_n = 0;
    void *obj = GameIni_createGroups(path, &names, &groups_n);
    GameIni **groups = GameIni_getGroups(obj);

    return TPPose_new(names[0], TPPose_models, TPPose_models_n,
                      TPPose_clone, groups[0], groups[0]);
}

void TPPose_applyRenderModes(TPPose **poses, int poses_n) {
    if (TPPose_defDrawModes != NULL && TPPose_mt != NULL) {
        int texs_n = 0;
        Texture **texs = MultyTexture_getTextures(TPPose_mt, &texs_n);
        applyModes(texs, texs_n, TPPose_defDrawModes, TPPose_defDrawModes_n);
    }

    for (int i = 0; i < poses_n; i++) {
        MultyTexture *mt = poses[i]->secondMt;
        int8_t *modes = poses[i]->drawModesSecond;
        int modes_n = poses[i]->drawModesSecond_n;

        if (mt != NULL && modes != NULL) {
            int texs_n = 0;
            Texture **texs = MultyTexture_getTextures(mt, &texs_n);
            applyModes(texs, texs_n, modes, modes_n);
        }
    }
}

TPPose *TPPose_new(const char *poseName, Mesh **meshes, int meshes_n,
                   Mesh *clone, GameIni *ini, GameIni *def) {
    TPPose *self = (TPPose *)calloc(1, sizeof(TPPose));
    self->poseName = strdup(poseName);

    self->walk = makeMorphing("WALK", meshes, meshes_n, clone, ini, def, true);
    self->attack = makeMorphing("ATTACK", meshes, meshes_n, clone, ini, def, false);
    self->walkSight = makeMorphing("WALK_SIGHT", meshes, meshes_n, clone, ini, def, false);
    self->attackSight = makeMorphing("ATTACK_SIGHT", meshes, meshes_n, clone, ini, def, false);

    char *tmp = GameIni_getDef(ini, "SECOND_DRAW_MODES", GameIni_get(def, "SECOND_DRAW_MODES"));
    if (tmp != NULL) self->drawModesSecond = loadModes(tmp, &self->drawModesSecond_n);

    Mesh **secondModels = NULL;
    int secondModels_n = 0;
    Mesh *cloneSecond = NULL;
    tmp = GameIni_getDef(ini, "SECOND_MODEL", GameIni_get(def, "SECOND_MODEL"));
    if (tmp != NULL) {
        float scale2 = GameIni_getFloat(ini, "SECOND_SCALE",
                       GameIni_getFloat(def, "SECOND_SCALE", 1.0f));
        secondModels = Asset_getMeshes(tmp, scale2, scale2, scale2, &secondModels_n);
        self->secondMt = MultyTexture_new(
            GameIni_getDef(ini, "SECOND_TEX", GameIni_get(def, "SECOND_TEX")), false);

        MeshClone *mc = MeshClone_new(secondModels[0]);
        cloneSecond = MeshClone_copy(mc);
        MeshClone_destroy(mc);
    }

    self->secondWalk = makeMorphing("SECOND_WALK", secondModels, secondModels_n, cloneSecond, ini, def, true);
    self->secondAttack = makeMorphing("SECOND_ATTACK", secondModels, secondModels_n, cloneSecond, ini, def, false);
    self->secondWalkSight = makeMorphing("SECOND_WALK_SIGHT", secondModels, secondModels_n, cloneSecond, ini, def, false);
    self->secondAttackSight = makeMorphing("SECOND_ATTACK_SIGHT", secondModels, secondModels_n, cloneSecond, ini, def, false);

    if (GameIni_getInt(ini, "MORPH_INTEPOLATE", GameIni_getInt(def, "MORPH_INTEPOLATE", 1)) == 0) {
        if (self->walk != NULL) Morphing_setMorphEnabled(self->walk, false);
        if (self->attack != NULL) Morphing_setMorphEnabled(self->attack, false);
        if (self->walkSight != NULL) Morphing_setMorphEnabled(self->walkSight, false);
        if (self->attackSight != NULL) Morphing_setMorphEnabled(self->attackSight, false);

        if (self->secondWalk != NULL) Morphing_setMorphEnabled(self->secondWalk, false);
        if (self->secondAttack != NULL) Morphing_setMorphEnabled(self->secondAttack, false);
        if (self->secondWalkSight != NULL) Morphing_setMorphEnabled(self->secondWalkSight, false);
        if (self->secondAttackSight != NULL) Morphing_setMorphEnabled(self->secondAttackSight, false);
    }

    self->show3D = GameIni_getInt(ini, "SHOW_3D", GameIni_getInt(def, "SHOW_3D", 1)) == 1;
    self->show2D = GameIni_getInt(ini, "SHOW_2D", GameIni_getInt(def, "SHOW_2D", 0)) == 1;
    self->showSecond = GameIni_getInt(ini, "SECOND_SHOW_3D",
        GameIni_getInt(def, "SECOND_SHOW_3D", self->secondWalk == NULL ? 0 : 1)) == 1;

    self->show3DSight = GameIni_getInt(ini, "SHOW_3D_SIGHT", GameIni_getInt(def, "SHOW_3D_SIGHT", 1)) == 1;
    self->show2DSight = GameIni_getInt(ini, "SHOW_2D_SIGHT", GameIni_getInt(def, "SHOW_2D_SIGHT", 0)) == 1;
    self->showSecondSight = GameIni_getInt(ini, "SECOND_SHOW_3D_SIGHT",
        GameIni_getInt(def, "SECOND_SHOW_3D_SIGHT", self->secondWalk == NULL ? 0 : 1)) == 1;

    self->canWalk = GameIni_getInt(ini, "CAN_WALK", GameIni_getInt(def, "CAN_WALK", 1)) == 1;
    self->canWalkSight = GameIni_getInt(ini, "CAN_WALK_SIGHT", GameIni_getInt(def, "CAN_WALK_SIGHT", 1)) == 1;

    self->canJump = GameIni_getInt(ini, "CAN_JUMP", GameIni_getInt(def, "CAN_JUMP", 1)) == 1;
    self->canJumpSight = GameIni_getInt(ini, "CAN_JUMP_SIGHT", GameIni_getInt(def, "CAN_JUMP_SIGHT", 1)) == 1;

    self->canLookX = GameIni_getInt(ini, "CAN_LOOK_X", GameIni_getInt(def, "CAN_LOOK_X", 1)) == 1;
    self->canLookXSight = GameIni_getInt(ini, "CAN_LOOK_X_SIGHT", GameIni_getInt(def, "CAN_LOOK_X_SIGHT", 1)) == 1;

    self->canLookY = GameIni_getInt(ini, "CAN_LOOK_Y", GameIni_getInt(def, "CAN_LOOK_Y", 1)) == 1;
    self->canLookYSight = GameIni_getInt(ini, "CAN_LOOK_Y_SIGHT", GameIni_getInt(def, "CAN_LOOK_Y_SIGHT", 1)) == 1;

    self->canAttack = GameIni_getInt(ini, "CAN_ATTACK", GameIni_getInt(def, "CAN_ATTACK", 1)) == 1;
    self->canAttackSight = GameIni_getInt(ini, "CAN_ATTACK_SIGHT", GameIni_getInt(def, "CAN_ATTACK_SIGHT", 1)) == 1;

    self->lookSpeed = GameIni_getFloat(ini, "LOOK_SPEED", GameIni_getFloat(def, "LOOK_SPEED", 1.0f));
    self->lookSpeedSight = GameIni_getFloat(ini, "LOOK_SPEED_SIGHT", GameIni_getFloat(def, "LOOK_SPEED_SIGHT", 0.71f));

    self->swapStrafeLook = GameIni_getInt(ini, "SWAP_STRAFE_LOOK", GameIni_getInt(def, "SWAP_STRAFE_LOOK", 0)) == 1;
    self->swapStrafeLookSight = GameIni_getInt(ini, "SWAP_STRAFE_LOOK_SIGHT", GameIni_getInt(def, "SWAP_STRAFE_LOOK_SIGHT", 0)) == 1;

    self->rotToWalkDir = GameIni_getInt(ini, "ROTATE_TO_WALK_DIR", GameIni_getInt(def, "ROTATE_TO_WALK_DIR", 0)) == 1;
    self->rotToWalkDirSight = GameIni_getInt(ini, "ROTATE_TO_WALK_DIR_SIGHT", GameIni_getInt(def, "ROTATE_TO_WALK_DIR_SIGHT", 0)) == 1;

    tmp = GameIni_getDef(ini, "CAM_POS", GameIni_get(def, "CAM_POS"));
    if (tmp != NULL) {
        int pos_n = 0;
        int *pos = GameIni_cutOnInts(tmp, ',', &pos_n);
        self->camX = pos[0]; self->camY = pos[1]; self->camZ = pos[2];
        free(pos);
    }

    self->camRotX = GameIni_getInt(ini, "CAM_ROT_X", GameIni_getInt(def, "CAM_ROT_X", 0));
    self->camRotY = GameIni_getInt(ini, "CAM_ROT_Y", GameIni_getInt(def, "CAM_ROT_Y", 0));
    self->rotModelY = GameIni_getInt(ini, "MODEL_ROT_Y", GameIni_getInt(def, "MODEL_ROT_Y", 0));
    self->camSmoothSteps = GameIni_getInt(ini, "CAM_SMOOTH_STEPS", GameIni_getInt(def, "CAM_SMOOTH_STEPS", 3));

    self->camAbsolutePos = GameIni_getInt(ini, "CAM_ABSOLUTE_POS", GameIni_getInt(def, "CAM_ABSOLUTE_POS", 0)) == 1;
    self->camAbsolutePosSight = GameIni_getInt(ini, "CAM_ABSOLUTE_POS_SIGHT", GameIni_getInt(def, "CAM_ABSOLUTE_POS_SIGHT", 0)) == 1;

    self->camAbsoluteRot = GameIni_getInt(ini, "CAM_ABSOLUTE_ROT", GameIni_getInt(def, "CAM_ABSOLUTE_ROT", 0)) == 1;
    self->camAbsoluteRotSight = GameIni_getInt(ini, "CAM_ABSOLUTE_ROT_SIGHT", GameIni_getInt(def, "CAM_ABSOLUTE_ROT_SIGHT", 0)) == 1;

    tmp = GameIni_getDef(ini, "CAM_POS_SIGHT", GameIni_get(def, "CAM_POS_SIGHT"));
    if (tmp != NULL) {
        int pos_n = 0;
        int *pos = GameIni_cutOnInts(tmp, ',', &pos_n);
        self->camXSight = pos[0]; self->camYSight = pos[1]; self->camZSight = pos[2];
        free(pos);
    }

    self->camRotXSight = GameIni_getInt(ini, "CAM_ROT_X_SIGHT", GameIni_getInt(def, "CAM_ROT_X_SIGHT", 0));
    self->camRotYSight = GameIni_getInt(ini, "CAM_ROT_Y_SIGHT", GameIni_getInt(def, "CAM_ROT_Y_SIGHT", 0));
    self->rotModelYSight = GameIni_getInt(ini, "MODEL_ROT_Y_SIGHT", GameIni_getInt(def, "MODEL_ROT_Y_SIGHT", 0));
    self->camSmoothStepsSight = GameIni_getInt(ini, "CAM_SMOOTH_STEPS_SIGHT", GameIni_getInt(def, "CAM_SMOOTH_STEPS_SIGHT", 2));

    self->rotModelX = GameIni_getInt(ini, "ROT_MODEL_X", GameIni_getInt(def, "ROT_MODEL_X", 0)) == 1;
    self->rotModelXSight = GameIni_getInt(ini, "ROT_MODEL_X_SIGHT", GameIni_getInt(def, "ROT_MODEL_X_SIGHT", 0)) == 1;

    tmp = GameIni_getDef(ini, "MUZZLE_FLASH_POS", GameIni_get(def, "MUZZLE_FLASH_POS"));
    if (tmp != NULL) {
        int vals_n = 0;
        int *vals = GameIni_cutOnInts(tmp, ',', &vals_n);
        self->muzzleFlashPos = Vertex_new_ints(vals, vals_n);
        free(vals);

        Texture *mfTex = Asset_getTexture(
            GameIni_getDef(ini, "MUZZLE_FLASH", GameIni_get(def, "MUZZLE_FLASH")));
        int mfScale = GameIni_getInt(ini, "MUZZLE_FLASH_SCALE",
                      GameIni_getInt(def, "MUZZLE_FLASH_SCALE", 1));
        self->muzzleFlash = Sprite_new(mfTex, mfScale);

        if (Texture_isAlphaMixing(mfTex)) Sprite_setMode(self->muzzleFlash, 3);
        Sprite_setFog(self->muzzleFlash, false);
        self->muzzleFlashTimer = GameIni_getInt(ini, "MUZZLE_FLASH_TIMER",
                                 GameIni_getInt(def, "MUZZLE_FLASH_TIMER", 0));
    }

    self->animationSpeed = GameIni_getInt(ini, "ANIMATION_SPEED", GameIni_getInt(def, "ANIMATION_SPEED", 100));
    self->animationSpeedAttack = GameIni_getInt(ini, "ANIMATION_SPEED_ATTACK",
        GameIni_getInt(def, "ANIMATION_SPEED_ATTACK", self->animationSpeed));

    self->walkSpeed = GameIni_getInt(ini, "WALK_SPEED", GameIni_getInt(def, "WALK_SPEED", INT_MIN));
    self->walkSpeedSight = GameIni_getInt(ini, "WALK_SPEED_SIGHT", GameIni_getInt(def, "WALK_SPEED_SIGHT", INT_MIN));
    if (self->walkSpeedSight == INT_MIN) self->walkSpeedSight = self->walkSpeed;

    return self;
}

bool TPPose_show2D(TPPose *self, Player *player) {
    return player->zoom ? self->show2DSight : self->show2D;
}

bool TPPose_canWalk(TPPose *self, Player *player) {
    return player->zoom ? self->canWalkSight : self->canWalk;
}

bool TPPose_canJump(TPPose *self, Player *player) {
    return player->zoom ? self->canJumpSight : self->canJump;
}

bool TPPose_canLookX(TPPose *self, Player *player) {
    return player->zoom ? self->canLookXSight : self->canLookX;
}

bool TPPose_canLookY(TPPose *self, Player *player) {
    return player->zoom ? self->canLookYSight : self->canLookY;
}

bool TPPose_canAttack(TPPose *self, Player *player) {
    return player->zoom ? self->canAttackSight : self->canAttack;
}

bool TPPose_isSwapStrafeLook(TPPose *self, Player *player) {
    return player->zoom ? self->swapStrafeLookSight : self->swapStrafeLook;
}

bool TPPose_isRotToWalkDir(TPPose *self, Player *player) {
    return player->zoom ? self->rotToWalkDirSight : self->rotToWalkDir;
}

static Morphing *getWalk(TPPose *self, bool zoom) {
    if (self->walkSight == NULL) return self->walk;
    return zoom ? self->walkSight : self->walk;
}

static Morphing *getSecondWalk(TPPose *self, bool zoom) {
    if (self->secondWalkSight == NULL) return self->secondWalk;
    return zoom ? self->secondWalkSight : self->secondWalk;
}

static Morphing *getAttack(TPPose *self, bool zoom) {
    if (self->attackSight == NULL) return self->attack;
    return zoom ? self->attackSight : self->attack;
}

static Morphing *getSecondAttack(TPPose *self, bool zoom) {
    if (self->secondAttackSight == NULL) return self->secondAttack;
    return zoom ? self->secondAttackSight : self->secondAttack;
}

void TPPose_update(TPPose *self, Camera *cam, Player *player) {
    bool zoom = player->zoom;
    extern void Camera_setX(Camera *c, int v);
    extern void Camera_setY(Camera *c, int v);
    extern void Camera_setZ(Camera *c, int v);
    extern void Camera_setRotX(Camera *c, int v);
    extern void Camera_setRotY(Camera *c, int v);
    extern void Camera_setSmoothSteps(Camera *c, int v);

    Camera_setX(cam, zoom ? self->camXSight : self->camX);
    Camera_setY(cam, zoom ? self->camYSight : self->camY);
    Camera_setZ(cam, zoom ? self->camZSight : self->camZ);
    Camera_setRotX(cam, zoom ? self->camRotXSight : self->camRotX);
    Camera_setRotY(cam, zoom ? self->camRotYSight : self->camRotY);
    Camera_setSmoothSteps(cam, zoom ? self->camSmoothStepsSight : self->camSmoothSteps);

    bool absolutePos = zoom ? self->camAbsolutePosSight : self->camAbsolutePos;
    bool absoluteRot = zoom ? self->camAbsoluteRotSight : self->camAbsoluteRot;

    if (!TPPose_canLookX(self, player)) {
        player->rotateX = 0;
        Player_updateMatrix(player);
    }
    Matrix *plmat = Character_getTransform(player->base.character);
    Camera_set(cam, plmat, player->rotateX, player->rotateY, absolutePos, absoluteRot);
}

void TPPose_draw(TPPose *self, Player *player, DirectX7 *g3d,
                 int x1, int y1, int x2, int y2) {
    bool zoom = player->zoom;
    Matrix *plmat = Character_getTransform(player->base.character);

    if (zoom ? self->show3DSight : self->show3D) {
        Matrix_set(&s_mat, plmat);
        Matrix_setRotY(&s_tmp, 180 + (zoom ? self->rotModelYSight : self->rotModelY));
        Matrix_mul(&s_mat, &s_tmp);
        if (!(zoom ? self->rotModelXSight : self->rotModelX)) {
            Matrix_setRotX(&s_tmp, (int)player->rotateX);
            Matrix_mul(&s_mat, &s_tmp);
        }

        Morphing *thisAttack = getAttack(self, zoom);
        Morphing *thisSecondAttack = getSecondAttack(self, zoom);

        Morphing *anim = getWalk(self, zoom);
        Morphing *second = getSecondWalk(self, zoom);

        if (thisAttack != NULL || thisSecondAttack != NULL) {
            Morphing *sec = thisAttack == NULL ? thisSecondAttack : thisAttack;
            int len = Morphing_getMaxFrame(sec);
            if (player->attackFrame <= len) {
                if (thisAttack != NULL) {
                    anim = thisAttack;
                    Morphing_setFrameNI(anim, player->attackFrame);
                }
                if (thisSecondAttack != NULL) {
                    second = thisSecondAttack;
                    Morphing_setFrameNI(second, player->attackFrame);
                }

                player->attackFrame += self->animationSpeedAttack * QFPS_frameTime / 50;
            }
        }

        if (anim != NULL && (anim == self->walk || anim == self->walkSight))
            Morphing_setFrameNI(anim, player->walkFrame);
        if (second != NULL && (second == self->secondWalk || second == self->secondWalkSight))
            Morphing_setFrameNI(second, player->walkFrame);

        if (player->base.character->speed.x != 0 || player->base.character->speed.z != 0) {
            Morphing *w = getWalk(self, zoom);
            player->walkFrame += self->animationSpeed * QFPS_frameTime / 50;
            if (w != NULL && Morphing_getMaxFrame(w) != 0)
                player->walkFrame %= Morphing_getMaxFrame(w);
        }

        if (anim != NULL) {
            Morphing_setFrame(anim, Morphing_getFrame(anim));
            DirectX7_transformAndProjectVertices(g3d, Morphing_getMesh(anim),
                DirectX7_computeFinalMatrix(g3d, &s_mat));
            DirectX7_addMesh(g3d, Morphing_getMesh(anim), x1, y1, x2, y2, TPPose_mt);
            if (TPPose_radius != 0) Mesh_increaseMeshSz(Morphing_getMesh(anim), TPPose_radius * 4);
        }

        if ((zoom ? self->showSecondSight : self->showSecond) && second != NULL) {
            Morphing_setFrame(second, Morphing_getFrame(second));
            DirectX7_transformAndProjectVertices(g3d, Morphing_getMesh(second),
                DirectX7_computeFinalMatrix(g3d, &s_mat));
            DirectX7_addMesh(g3d, Morphing_getMesh(second), x1, y1, x2, y2, self->secondMt);
            if (TPPose_radius != 0) Mesh_increaseMeshSz(Morphing_getMesh(second), TPPose_radius * 4);
        }

        if (self->muzzleFlash != NULL && player->muzzleFrame > 0) {
            Vertex_transform(self->muzzleFlashPos, &s_mat);
            extern int Vertex_getSx(Vertex *v);
            extern int Vertex_getSy(Vertex *v);
            extern int Vertex_getRz(Vertex *v);
            Vector3D *mpos = Sprite_getPos(self->muzzleFlash);
            mpos->x = Vertex_getSx(self->muzzleFlashPos);
            mpos->y = Vertex_getSy(self->muzzleFlashPos) - Sprite_getHeight(self->muzzleFlash) / 2;
            mpos->z = Vertex_getRz(self->muzzleFlashPos);
            Sprite_project(self->muzzleFlash, DirectX7_getInvCamera(g3d), g3d);
            DirectX7_addRenderObject(g3d, self->muzzleFlash, x1, y1, x2, y2);
            extern int *Sprite_getSzPtr(Sprite *s);
            *Sprite_getSzPtr(self->muzzleFlash) += TPPose_radius * 4;
            player->muzzleFrame -= QFPS_frameTime;
        }
    }
}

float TPPose_lookSpeedVal(TPPose *self, Player *player) {
    return player->zoom ? self->lookSpeedSight : self->lookSpeed;
}

int TPPose_walkSpeedVal(TPPose *self, Player *player) {
    int speed = player->zoom ? self->walkSpeedSight : self->walkSpeed;
    return speed == INT_MIN ? Player_walkSpeedStatic : speed;
}
