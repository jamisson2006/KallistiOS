/*
 * weapon_creator.c — porte fiel de code/utils/WeaponCreator.java (Quantum Engine J2ME)
 */
#include "weapon_creator.h"
#include "gameini.h"
#include "asset.h"
#include "stringtools.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* --- externals --- */
/* Weapon (a converter). Cria com todos os campos passados. */
extern Weapon *Weapon_new(const char *image, const char *fire,
                          float kw, float kh,
                          int damage, int delay, int shotTime,
                          int twoHands, int capacity, int reloadTime,
                          long distance, int patronBuy, int index);
extern void Weapon_setShoot(Weapon *w, const char *s);
extern const char *Weapon_shoot(const Weapon *w);
extern void Weapon_setReload(Weapon *w, const char *s);
extern const char *Weapon_reload(const Weapon *w);

extern int  Weapon_newanim(const Weapon *w);
extern void Weapon_setNewanim(Weapon *w, int v);
extern int  Weapon_canShoot(const Weapon *w);
extern void Weapon_setCanShoot(Weapon *w, int v);

extern void Weapon_setMeleeAnim(Weapon *w, int v);
extern void Weapon_setAttackIntensity(Weapon *w, float v);

extern int  Weapon_ignoreSightOnDraw(const Weapon *w);
extern void Weapon_setIgnoreSightOnDraw(Weapon *w, int v);

extern void Weapon_setFileSight(Weapon *w, const char *v);
extern void Weapon_setFileSightWeapon(Weapon *w, const char *v);
extern void Weapon_setFilePatron(Weapon *w, const char *v);
extern void Weapon_setFilePatronLow(Weapon *w, const char *v);

extern void Weapon_setCustomPos(Weapon *w, int v);
extern void Weapon_setCustomPosX(Weapon *w, float v);

extern int  Weapon_centreAlign(const Weapon *w);
extern void Weapon_setCentreAlign(Weapon *w, int v);
extern int  Weapon_debugWeapon(const Weapon *w);
extern void Weapon_setDebugWeapon(Weapon *w, int v);
extern int  Weapon_leftHand(const Weapon *w);
extern void Weapon_setLeftHand(Weapon *w, int v);
extern int  Weapon_shootLight(const Weapon *w);
extern void Weapon_setShootLight(Weapon *w, int v);

extern void Weapon_setStdFov(Weapon *w, int v);
extern void Weapon_setZoomFov(Weapon *w, int v);
extern void Weapon_setHasZoom(Weapon *w, int v);

extern void Weapon_setPlayerPose(Weapon *w, int v);
extern void Weapon_setAmmoPriceFactor(Weapon *w, float v);
extern int  Weapon_ammoBundled(const Weapon *w);
extern void Weapon_setAmmoBundled(Weapon *w, int v);
extern int  Weapon_ammoInShop(const Weapon *w);
extern void Weapon_setAmmoInShop(Weapon *w, int v);
extern int  Weapon_lowPatronAmount(const Weapon *w);
extern void Weapon_setLowPatronAmount(Weapon *w, int v);

/* TPPose */
typedef struct TPPose { const char *poseName; /* ... */ } TPPose;
extern TPPose **TPPose_meshPoses(int *out_n); /* NULL, 0 se ainda nao definido */

/* Main */
extern int  Main_isSounds;
extern int  Main_sounds;

Weapon *WeaponCreator_createWeapon(int index) {
    char **names = NULL;
    GameIni **groups = NULL;
    int count = 0;
    GameIni_createGroups("/weapons.txt", &names, &groups, &count);

    /* Java: Integer.toString(index) */
    char idx[16]; snprintf(idx, sizeof idx, "%d", index);

    Weapon *result = NULL;

    for (int i = 0; i < count; i++) {
        if (strcmp(names[i], idx) != 0) continue;
        GameIni *obj = groups[i];

        float kw = 1.0f;
        if (GameIni_get(obj, "KW") != NULL) kw = IniFile_getFloat(obj, "KW");
        float kh = 1.0f;
        if (GameIni_get(obj, "KH") != NULL) kh = IniFile_getFloat(obj, "KH");

        result = Weapon_new(
            GameIni_get(obj, "IMAGE"),
            GameIni_get(obj, "FIRE"),
            kw, kh,
            IniFile_getInt(obj, "DAMAGE"),
            IniFile_getInt(obj, "DELAY"),
            IniFile_getInt(obj, "SHOTTIME"),
            IniFile_getIntDef(obj, "TWOHANDS", 0) == 1,
            IniFile_getIntDef(obj, "CAPACITY", 1),
            IniFile_getInt(obj, "RELOADTIME"),
            (long) IniFile_getLong(obj, "DISTANCE"),
            IniFile_getIntDef(obj, "PATRONBUY", 1) == 1,
            index);

        Weapon_setShoot(result, GameIni_get(obj, "SHOOT_SOUND"));
        if (Weapon_shoot(result) != NULL && Main_isSounds && Main_sounds > 0)
            Asset_getSound(Weapon_shoot(result));

        Weapon_setReload(result, GameIni_get(obj, "RELOAD_SOUND"));
        if (Weapon_reload(result) != NULL && Main_isSounds && Main_sounds > 0)
            Asset_getSound(Weapon_reload(result));

        Weapon_setNewanim(result,
            IniFile_getIntDef(obj, "NEWFIREANIM", Weapon_newanim(result) ? 1 : 0) == 1);

        Weapon_setCanShoot(result,
            IniFile_getIntDef(obj, "CANSHOOT", Weapon_canShoot(result) ? 1 : 0) == 1);

        const char *tmp = GameIni_get(obj, "MELEEANIM");
        if (tmp != NULL && strcmp(tmp, "1") == 0) {
            Weapon_setMeleeAnim(result, 1);
            Weapon_setAttackIntensity(result, 5.0f);
        }

        Weapon_setIgnoreSightOnDraw(result,
            IniFile_getIntDef(obj, "IGNORE_SIGHT_IMAGE",
                Weapon_ignoreSightOnDraw(result) ? 1 : 0) == 1);

        Weapon_setFileSight(result,       GameIni_get(obj, "SIGHT_IMAGE"));
        Weapon_setFileSightWeapon(result, GameIni_get(obj, "SIGHT_WEAPON_IMAGE"));
        Weapon_setFilePatron(result,      GameIni_get(obj, "PATRON_ICON"));
        Weapon_setFilePatronLow(result,   GameIni_get(obj, "LOW_PATRON_ICON"));

        if (GameIni_get(obj, "X_POS") != NULL) {
            Weapon_setCustomPos(result, 1);
            Weapon_setCustomPosX(result, IniFile_getFloat(obj, "X_POS"));
        }

        Weapon_setCentreAlign(result,
            IniFile_getIntDef(obj, "CENTRE_ALIGHT", Weapon_centreAlign(result) ? 1 : 0) == 1);
        Weapon_setDebugWeapon(result,
            IniFile_getIntDef(obj, "DEBUG_WEAPON", Weapon_debugWeapon(result) ? 1 : 0) == 1);

        Weapon_setLeftHand(result,
            IniFile_getIntDef(obj, "LEFT_HAND", Weapon_leftHand(result) ? 1 : 0) == 1);
        Weapon_setShootLight(result,
            IniFile_getIntDef(obj, "SHOOT_LIGHT", Weapon_shootLight(result) ? 1 : 0) == 1);

        const char *str = GameIni_get(obj, "ZOOM");
        if (str != NULL) {
            int nn;
            int *snds = GameIni_cutOnInts(str, ';', ',', &nn);
            if (nn >= 3) {
                Weapon_setStdFov(result,  snds[0]);
                Weapon_setZoomFov(result, snds[1]);
                Weapon_setHasZoom(result, snds[2] == 1);
            }
            free(snds);
        }

        str = GameIni_get(obj, "PLAYER_POSE");
        int np = 0;
        TPPose **poses = TPPose_meshPoses(&np);
        if (str != NULL && poses != NULL) {
            for (int x = 0; x < np; x++) {
                if (poses[x]->poseName && strcmp(poses[x]->poseName, str) == 0) {
                    Weapon_setPlayerPose(result, x);
                    break;
                }
            }
        }

        str = GameIni_get(obj, "AMMO_PRICE_PERCENTAGE");
        if (str != NULL)
            Weapon_setAmmoPriceFactor(result, StringTools_parseFloat(str) / 100.0f);

        Weapon_setAmmoBundled(result,
            IniFile_getIntDef(obj, "AMMO_BUNDLED", Weapon_ammoBundled(result)));
        Weapon_setAmmoInShop(result,
            IniFile_getIntDef(obj, "AMMO_IN_SHOP", Weapon_ammoInShop(result)));

        Weapon_setLowPatronAmount(result,
            IniFile_getIntDef(obj, "LOW_PATRON_AMOUNT", Weapon_lowPatronAmount(result)));

        break;
    }

    IniFile_freeGroupsResult(names, groups, count);
    return result;
}
