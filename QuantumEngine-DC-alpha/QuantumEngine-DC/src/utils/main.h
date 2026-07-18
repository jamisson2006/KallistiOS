/*
 * main.h — porte fiel de code/utils/Main.java (Quantum Engine J2ME)
 *
 * Main eh o "app object" do J2ME (MIDlet). Ele guarda todo o estado global
 * de configuracao do engine (idiomas, icones, quality, save-name...). Aqui
 * reproduzimos como variaveis globais com o prefixo Main_ + funcoes soltas.
 */
#ifndef QE_UTILS_MAIN_H
#define QE_UTILS_MAIN_H

#include <stdint.h>
#include "inifile.h"
#include "gameini.h"
#include "sound.h"

/* Forward decls das classes externas usadas em assinaturas. */
typedef struct Font        Font;
typedef struct Image       Image;
typedef struct Graphics    Graphics;
typedef struct Player      Player;
typedef struct Scene       Scene;
typedef struct MyCanvas    MyCanvas;
typedef struct MainCanvas  MainCanvas;
typedef struct Menu        Menu;
typedef struct Displayable Displayable;

/* --- Estado publico (Java: public static ...) --- */

extern int         Main_lastLevel;
extern IniFile    *Main_gameText;

extern const char *Main_Blood;      /* "/blood.png" */
extern float       Main_bloodscale, Main_splinterscale;
extern int         Main_fullScreenSight;

extern int         Main_corpses, Main_hideHud, Main_blood, Main_frameskip, Main_mipMapping;
extern int         Main_displaySize;
extern int         Main_floorOffsetSZ;

extern int         Main_lang;
extern int         Main_q;
extern const char *Main_life_icon, *Main_hand_icon, *Main_money_icon, *Main_patron_icon;
extern const char *Main_sight_icon, *Main_skull_icon, *Main_background_logo, *Main_background;
extern const char *Main_shop_patron, *Main_shop_medkit, *Main_low_life_icon;
extern const char *Main_low_patron_icon, *Main_patron_no_weapon_icon;

extern char      **Main_splash;
extern int         Main_splash_count;
extern char      **Main_stepSound;
extern int         Main_stepSound_count;
extern const char *Main_jumpSound;
extern const char *Main_menuMusic;

extern int         Main_canSave, Main_canSelectLevel, Main_levelSelectorLoadData;
extern int         Main_pauseScreenSave, Main_saveOnQuit;
extern int         Main_hasZoom;
extern int         Main_stdFov, Main_zoomFov;

extern char      **Main_langs;
extern int         Main_langs_count;
extern Image      *Main_bcks, *Main_bcks2;

extern int         Main_updateOnlyNear, Main_updateOnlyNearPhysics;
extern GameIni    *Main_settings;
extern int         Main_playerHasInventory;

extern int         Main_hidesight;
extern int         Main_fogQ;      /* 0-off 1 - lq 2 - hq */
extern int         Main_pixelsQ;   /* 0-4xmax 1 - 2xmax 2 - 1xmax */
extern int         Main_persQ;
extern int         Main_availableLevel;
extern int         Main_forceLQFog;
extern int         Main_originalSight;
extern int         Main_originalUseIcon;

extern int         Main_music, Main_sounds, Main_footsteps, Main_mouseSpeed;
extern int         Main_isMusic, Main_isSounds, Main_isFootsteps;
extern Sound      *Main_musicPlayer;

extern int         Main_verticalShopScroll;

extern MainCanvas *Main_mainCanvas;
extern const int   Main_s60Optimization;
extern int         Main_levelCounter;
extern int         Main_resizeWeapons;
extern int         Main_symbian;

/* --- API do MIDlet convertida em funcoes --- */

void Main_startApp(void);   /* Java: startApp() */
void Main_pauseApp(void);   /* Java: pauseApp() */
void Main_destroyApp(int unconditional);

int  Main_isExist(const char *file);

void Main_init(void);

void Main_setCurrent(MyCanvas *canvas);          /* estatico Java */
void Main_setCurrentRepaint(MyCanvas *canvas);
void Main_resetCanvas(void);
void Main_setCanvas(Displayable *disp);

void Main_setLanguage(const char *file);
Font   *Main_getFont(void);
IniFile*Main_getGameText(void);

int  Main_isFrameskip(void);
int  Main_isCorpses(void);
int  Main_isMipMapping(void);
int  Main_isBlood(void);

void Main_setDisplaySize(int size);
int  Main_getDisplaySize(void);

int  Main_getAvailableLevelCount(void);
void Main_setAvailableLevelCount(int i);
void Main_addAvailableLevel(int level);
int  Main_isLastLevel(int level);

void Main_saveSettingToStore(void);

void Main_drawBck(Graphics *g, int beginy, int endy);
void Main_drawBckDialog(Graphics *g, int beginy, int endy);

/* Save / Load */
void Main_saveGame(int levelNum, Player *player, Scene *scene);
void Main_saveObjects(int levelNum, Player *player, Scene *scene);
void Main_loadGame(Player *player, int w, int h, Scene *scene);
void Main_loadObjects(Player *player, int w, int h, Scene *scene, int levelNumber);
void Main_loadPosition(Player *player);

/* Carrega proximo nivel — hudInfo/menu podem ser NULL. */
void Main_loadLevel(int loadSave, int loadPos, int levelNumber,
                    void *hudInfo, Menu *menu, int helpState, int showLoad);

void Main_removeSave(void);
int  Main_getContinueLevel(void);
int  Main_hasSave(void);

void Main_setPersQ(int pq);

#endif
