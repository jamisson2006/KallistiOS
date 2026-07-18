/*
 * main.c — porte fiel de code/utils/Main.java (Quantum Engine J2ME)
 *
 * Este arquivo mantem a mesma logica do original: leitura de setting.txt,
 * defaults, saves via RmsUtils, drawBck/drawBckDialog, loadLevel, saveGame.
 *
 * Muitas classes referenciadas (Player, Scene, Weapon, GameHelp, Menu, ...)
 * ainda serao convertidas em zips posteriores. Aqui elas aparecem como
 * declaracoes extern — o binario final so linkara depois desses batches
 * chegarem.
 */
#include "main.h"
#include "gameini.h"
#include "inifile.h"
#include "stringtools.h"
#include "rms_utils.h"
#include "sound.h"
#include "weapon_creator.h"
#include "canvas/main_canvas.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

/* ============================================================
 * Externals (modulos ainda por converter)
 * ============================================================ */

/* Splinter (AI/misc) */
extern void   *Splinter_texture;    /* na verdade um Texture* */
extern void    Splinter_cache(void);
extern void   *Blood_blood;         /* Texture* (AI/misc/Blood) */

/* Texture */
extern void *Texture_createTexture(const char *file);

/* Image (J2ME lcdui) */
extern Image *Image_createImage(const char *file);
extern int    Image_getHeight(Image *img);
extern int    Image_getWidth(Image *img);
extern void   Graphics_drawRegion(Graphics *g, Image *img,
                                  int x_src, int y_src, int w, int h,
                                  int transform, int x_dst, int y_dst, int anchor);

/* Font (HUD/Base/Font) */
extern Font *Font_new(const char *file);

/* GameKeyboard (HUD/Base/GameKeyboard) */
extern int  *GameKeyboard_keyCodes;
extern int   GameKeyboard_keyCodes_len;
extern int  *GameKeyboard_hasKeyCodes; /* 1/0 array */

/* Shop */
extern void  Shop_initShop(void);
extern int   Shop_weaponCount;

/* InventoryScreen */
extern int   InventoryScreen_proportionalInventory;
/* ItemsEngine */
extern void  ItemsEngine_init(void);

/* Player */
extern int    Player_money(Player *p);
extern void   Player_setMoney(Player *p, int m);
extern int    Player_getHp(Player *p);
extern void   Player_setHp(Player *p, int hp);
extern float  Player_fov(Player *p);
extern void   Player_setFov(Player *p, float f);
extern int    Player_stdFov(Player *p);
extern void   Player_setStdFov(Player *p, int f);
extern int    Player_zoomFov(Player *p);
extern void   Player_setZoomFov(Player *p, int f);
extern int    Player_zoom(Player *p);
extern void   Player_setZoom(Player *p, int z);
extern void  *Player_getCharacter(Player *p);
extern void  *Player_arsenal(Player *p);
extern float  Player_rotateY(Player *p);
extern float  Player_rotateX(Player *p);
extern void   Player_rotYn(Player *p, float v);
extern void   Player_rotXn(Player *p, float v);
extern void   Player_updateMatrix(Player *p);
extern void  *Player_items(Player *p);
extern void   PlayerItems_writeSave(void *items, void *dos);
extern void   PlayerItems_loadSave(void *items, void *dis);
extern void  *Player_usedPoints(void);           /* Vector estatico */
extern void   Vector_removeAllElements(void *v);
extern int    Vector_size(void *v);
extern void  *Vector_elementAt(void *v, int i);
extern void   Vector_addElement(void *v, void *elem);

/* Character / Vector3D / Matrix */
extern void  *Character_getTransform(void *ch);
extern void  *Character_getSpeed(void *ch);
extern int    Matrix_m03(void *m); extern int Matrix_m13(void *m); extern int Matrix_m23(void *m);
extern void   Matrix_setIdentity(void *m);
extern void   Matrix_setPosition(void *m, int x, int y, int z);
extern void   Matrix_set(void *m, int a,int b,int c,int d, int e,int f,int g,int h, int i,int j,int k,int l);
extern int    Vector3D_x(void *v); extern int Vector3D_y(void *v); extern int Vector3D_z(void *v);
extern void   Vector3D_set(void *v, int x, int y, int z);
extern void  *Vector3D_new(int x, int y, int z);

/* Scene */
extern void  *Scene_getG3D(Scene *scene); /* DirectX7* */
extern int    G3D_flashlightEnabled(void *g3d);
extern void   G3D_setFlashlightEnabled(void *g3d, int on);
extern void  *Scene_rmsBots(Scene *scene);
extern void  *Scene_rmsObjects(Scene *scene);
extern void  *Scene_getItemBags(Scene *scene);
extern void  *Scene_getHouse(Scene *scene);
extern void   House_addObject(void *h, void *obj);
extern int   *Scene_rmsBotsKilled(Scene *scene);
extern void   Scene_setRmsBotsKilled(Scene *scene, int *arr, int n);
extern int   *Scene_rmsObjectsDestroyed(Scene *scene);
extern void   Scene_setRmsObjectsDestroyed(Scene *scene, int *arr, int n);

/* Weapon */
extern void *Weapon_new_placeholder(void); /* nao usado — placeholder p/ tipo */
extern void  Weapon_createSprite(void *w, int w_, int h_);
extern void *Weapon_magazine(void *w);
extern void  Magazine_set(void *m, int ammo, int rounds);
extern int   Magazine_ammo(void *m);
extern int   Magazine_rounds(void *m);
extern void *Arsenal_weapons(void *arsenal, int *n);
extern int   Arsenal_current(void *arsenal);
extern void  Arsenal_setCurrent(void *arsenal, int c);
extern void  Arsenal_setWeapons(void *arsenal, void **weapons, int n);
extern void  Arsenal_destroy(void *arsenal);

/* NPC / NPCSpawner / RoomObject / ItemsBag */
extern int   NPC_isDead(void *obj);
extern int   NPCSpawner_deathCheck(void *obj);
extern int   RoomObject_isNPC(void *obj);
extern int   RoomObject_isNPCSpawner(void *obj);
extern int   RoomObject_activated(void *obj);
extern void *ItemsBag_new(void *pos_v3);
extern void  ItemsBag_writeSave(void *bag, void *dos);
extern void  ItemsBag_loadSave(void *bag, void *dis);

/* HUD / Menu / Screens */
extern int   DeveloperMenu_debugMode;
extern int   GameHelp_needToShow(int levelNumber, int helpState, int fromDeath);
extern void *GameHelp_new(void *main, Menu *menu, int levelNumber, int fromDeath,
                          void *hudInfo, int helpState, int showLoad);
extern int   GameHelp_loadSave;
extern int   GameHelp_loadpos;
extern void *LoadingScreen_new(void *main, int levelNumber, int loadSave, int loadPos, int showLoad);
extern void  Menu_destroy(Menu *menu);

/* GameScreen (para pauseApp e loadLevel) */
extern int   MyCanvas_isGameScreen(MyCanvas *c);
extern void  GameScreen_openPause(MyCanvas *gs);

/* ============================================================
 * Estado publico (Java: public static ...)
 * ============================================================ */

int         Main_lastLevel = 0;
IniFile    *Main_gameText  = NULL;

const char *Main_Blood         = "/blood.png";
float       Main_bloodscale    = 0.0f, Main_splinterscale = 0.0f;
int         Main_fullScreenSight = 0;

static const char *Main_savename = NULL;
int         Main_corpses = 0, Main_hideHud = 0, Main_blood = 0;
int         Main_frameskip = 0, Main_mipMapping = 1;
int         Main_displaySize = 80;

int         Main_floorOffsetSZ = 0;

int         Main_lang = -1;
int         Main_q    = 24;
const char *Main_life_icon=NULL, *Main_hand_icon=NULL, *Main_money_icon=NULL;
const char *Main_patron_icon=NULL, *Main_sight_icon=NULL, *Main_skull_icon=NULL;
const char *Main_background_logo=NULL, *Main_background=NULL;
const char *Main_shop_patron=NULL, *Main_shop_medkit=NULL, *Main_low_life_icon=NULL;
const char *Main_low_patron_icon=NULL, *Main_patron_no_weapon_icon=NULL;

char      **Main_splash = NULL; int Main_splash_count = 0;
char      **Main_stepSound = NULL; int Main_stepSound_count = 0;
const char *Main_jumpSound = NULL;
const char *Main_menuMusic = NULL;

int         Main_canSave = 1, Main_canSelectLevel = 0, Main_levelSelectorLoadData = 0;
int         Main_pauseScreenSave = 0, Main_saveOnQuit = 0;
int         Main_hasZoom = 1;
int         Main_stdFov = 74, Main_zoomFov = 44;

char      **Main_langs = NULL; int Main_langs_count = 0;
Image      *Main_bcks = NULL, *Main_bcks2 = NULL;

int         Main_updateOnlyNear = 0, Main_updateOnlyNearPhysics = 0;
GameIni    *Main_settings = NULL;
int         Main_playerHasInventory = 0;

int         Main_hidesight = 0;
int         Main_fogQ = 2, Main_pixelsQ = 2, Main_persQ = 2;
int         Main_availableLevel = 1;
int         Main_forceLQFog = 0, Main_originalSight = 0, Main_originalUseIcon = 0;

int         Main_music = 100, Main_sounds = 100, Main_footsteps = 100, Main_mouseSpeed = 66;
int         Main_isMusic = 1, Main_isSounds = 1, Main_isFootsteps = 1;
Sound      *Main_musicPlayer = NULL;

int         Main_verticalShopScroll = 0;

MainCanvas *Main_mainCanvas = NULL;
const int   Main_s60Optimization = 0;
int         Main_levelCounter = 1;
int         Main_resizeWeapons = 0;
int         Main_symbian = 0;

static Font *g_font = NULL;
static int   g_run  = 0;

/* ============================================================
 * DataOutputStream / DataInputStream — big-endian, fiel ao Java
 * ============================================================ */

typedef struct { uint8_t *buf; size_t len, cap; } qe_bao;
typedef struct { const uint8_t *buf; size_t len, pos; } qe_bai;

static void qe_bao_init(qe_bao *b) { b->buf = NULL; b->len = 0; b->cap = 0; }
static void qe_bao_free(qe_bao *b) { free(b->buf); }
static void qe_bao_grow(qe_bao *b, size_t n) {
    if (b->len + n <= b->cap) return;
    size_t nc = b->cap ? b->cap * 2 : 64;
    while (nc < b->len + n) nc *= 2;
    b->buf = (uint8_t*) realloc(b->buf, nc);
    b->cap = nc;
}
static void qe_writeInt(qe_bao *b, int v) {
    qe_bao_grow(b, 4);
    b->buf[b->len++] = (uint8_t)((v >> 24) & 0xff);
    b->buf[b->len++] = (uint8_t)((v >> 16) & 0xff);
    b->buf[b->len++] = (uint8_t)((v >> 8)  & 0xff);
    b->buf[b->len++] = (uint8_t)( v        & 0xff);
}
static void qe_writeShort(qe_bao *b, int v) {
    qe_bao_grow(b, 2);
    b->buf[b->len++] = (uint8_t)((v >> 8) & 0xff);
    b->buf[b->len++] = (uint8_t)( v       & 0xff);
}
static void qe_writeBoolean(qe_bao *b, int v) {
    qe_bao_grow(b, 1);
    b->buf[b->len++] = v ? 1 : 0;
}
static void qe_writeUTF(qe_bao *b, const char *s) {
    size_t l = strlen(s);
    qe_writeShort(b, (int) l);
    qe_bao_grow(b, l);
    memcpy(b->buf + b->len, s, l);
    b->len += l;
}

static int  qe_readInt(qe_bai *r) {
    if (r->pos + 4 > r->len) return 0;
    int v = ((int)r->buf[r->pos] << 24) | ((int)r->buf[r->pos+1] << 16)
          | ((int)r->buf[r->pos+2] << 8) | (int)r->buf[r->pos+3];
    r->pos += 4; return v;
}
static int  qe_readShort(qe_bai *r) {
    if (r->pos + 2 > r->len) return 0;
    int v = ((int)r->buf[r->pos] << 8) | (int)r->buf[r->pos+1];
    r->pos += 2; return v;
}
static int  qe_readBoolean(qe_bai *r) {
    if (r->pos + 1 > r->len) return 0;
    return r->buf[r->pos++] != 0;
}
static char *qe_readUTF(qe_bai *r) {
    int l = qe_readShort(r);
    if (l < 0 || r->pos + l > r->len) return strdup("");
    char *s = (char*) malloc(l + 1);
    memcpy(s, r->buf + r->pos, l);
    s[l] = 0;
    r->pos += l;
    return s;
}

/* ============================================================
 * Implementacao
 * ============================================================ */

Font    *Main_getFont(void)     { return g_font; }
IniFile *Main_getGameText(void) { return Main_gameText; }
int      Main_isFrameskip(void) { return Main_frameskip; }
int      Main_isCorpses(void)   { return Main_corpses; }
int      Main_isMipMapping(void){ return Main_mipMapping; }
int      Main_isBlood(void)     { return Main_blood; }

void Main_setDisplaySize(int size) {
    if (size < 50)  size = 50;
    if (size > 100) size = 100;
    Main_displaySize = size;
}
int Main_getDisplaySize(void) { return Main_displaySize; }

int Main_getAvailableLevelCount(void) {
    return Main_availableLevel < Main_lastLevel ? Main_availableLevel : Main_lastLevel;
}

void Main_setAvailableLevelCount(int i) {
    Main_availableLevel = i;
    Main_saveSettingToStore();
}

void Main_addAvailableLevel(int level) {
    if (level + 1 > Main_availableLevel && level + 1 <= Main_lastLevel) {
        Main_availableLevel = level + 1;
        Main_saveSettingToStore();
    }
}

int Main_isLastLevel(int level) { return level >= Main_lastLevel; }

void Main_setPersQ(int pq) {
    Main_persQ = pq;
    Main_q = 12;
    if (Main_persQ <= 2) Main_q = 24;
}

/* --- isExist: usa resource path fiel --- */
int Main_isExist(const char *file) {
    char path[512];
    if (file[0] == '/') snprintf(path, sizeof path, "/rd%s", file);
    else                snprintf(path, sizeof path, "/rd/%s", file);
    FILE *fp = fopen(path, "rb");
    if (!fp) return 0;
    fclose(fp);
    return 1;
}

static void Main_updateLevelCount(void) {
    char name[64];
    for (Main_lastLevel = 0; ; Main_lastLevel++) {
        snprintf(name, sizeof name, "/level%d.txt", Main_lastLevel + 1);
        if (!Main_isExist(name)) break;
    }
}

/* --- defaultSettings (fiel; sem microedition.platform, definimos symbian=false) --- */
static void Main_defaultSettings(void) {
    Main_music = 100;
    Main_sounds = 100;
    Main_footsteps = 100;
    Main_displaySize = 80;
    Main_availableLevel = 1;
    Main_persQ  = 2;
    Main_q      = 24;
    Main_pixelsQ = 2;
    Main_fogQ    = 2;

    Main_corpses = 0;
    Main_blood   = 0;
    Main_mipMapping = 1;
    Main_lang    = -1;
    Main_mouseSpeed = 66;
    Main_hideHud = 0;
    Main_resizeWeapons =
        (IniFile_getIntDef(Main_settings, "RESIZE_WEAPONS", 0) == 1);

    Main_symbian = 0; /* Dreamcast — nao Symbian */

    /* Java: frameskip = !symbian; e depois pstros/win/mac/etc => setPersQ(3) */
    Main_frameskip = 1;
    Main_setPersQ(3);

    Main_updateLevelCount();
}

/* --- setLanguage --- */
void Main_setLanguage(const char *file) {
    if (Main_gameText) IniFile_free(Main_gameText);
    Main_gameText = IniFile_createFromResource(file);
    g_font = Font_new(IniFile_get(Main_gameText, "FONT"));
}

/* --- init: carrega settings, save-state e primeira tela --- */

void Main_init(void) {
    g_run = 1;

    Main_settings = GameIni_createFromResourceKeys("/setting.txt", 1);

    Main_savename = GameIni_getNoLangDef(Main_settings, "SAVE_NAME", "ZOMBIE");
    DeveloperMenu_debugMode =
        (IniFile_getIntDef(Main_settings, "DEBUG", 0) == 1);

    Main_splinterscale = IniFile_getFloatDef(Main_settings, "SPLINTER_SCALE", 1.0f);
    Main_bloodscale    = IniFile_getFloatDef(Main_settings, "BLOOD_SCALE",   1.0f);

    Main_canSave              = (IniFile_getIntDef(Main_settings, "CANSAVE", 1) == 1);
    Main_saveOnQuit           = (IniFile_getIntDef(Main_settings, "SAVESTATE", 0) == 1);
    Main_canSelectLevel       = (IniFile_getIntDef(Main_settings, "CANSELECTLEVEL", 1) == 1);
    Main_levelSelectorLoadData= (IniFile_getIntDef(Main_settings, "LEVELSELECTOR_LOAD_DATA", 1) == 1);
    Main_pauseScreenSave      = (IniFile_getIntDef(Main_settings, "PAUSE_SCREEN_SAVE", 0) == 1);

    const char *tmp = GameIni_getNoLang(Main_settings, "SHOPSCROLL");
    if (tmp != NULL) Main_verticalShopScroll = (strcmp(tmp, "VERTICAL") == 0);

    Main_hidesight = (IniFile_getIntDef(Main_settings, "HIDESIGHT", 0) == 1);
    Main_menuMusic = GameIni_getNoLang(Main_settings, "MENU_MUSIC");

    Splinter_texture = Texture_createTexture(
        GameIni_getNoLangDef(Main_settings, "SPLINTER_SPRITE", "/splinter.png"));
    Splinter_cache();

    const char *bloodTexture =
        GameIni_getNoLangDef(Main_settings, "BLOOD_SPRITE", "/blood.png");
    if (bloodTexture != NULL) Blood_blood = Texture_createTexture(bloodTexture);

    tmp = GameIni_getNoLang(Main_settings, "TEXT_BCK");
    if (tmp != NULL) {
        Main_bcks = Image_createImage(tmp);
        if (!Main_bcks) fprintf(stderr, "No text background\n");
    }

    tmp = GameIni_getNoLang(Main_settings, "DIALOG_BCK");
    if (tmp != NULL) {
        Main_bcks2 = Image_createImage(tmp);
        if (!Main_bcks2) fprintf(stderr, "No dialog background\n");
    }

    Main_life_icon                = GameIni_getNoLangDef(Main_settings, "LIFE_ICON", "/life.png");
    Main_low_life_icon            = GameIni_getNoLangDef(Main_settings, "LOW_LIFE_ICON", NULL);
    Main_money_icon               = GameIni_getNoLangDef(Main_settings, "MONEY_ICON", "/money.png");
    Main_patron_icon              = GameIni_getNoLangDef(Main_settings, "PATRON_ICON", "/patron.png");
    Main_low_patron_icon          = GameIni_getNoLangDef(Main_settings, "LOW_PATRON_ICON", "/patron_low.png");
    Main_patron_no_weapon_icon    = GameIni_getNoLangDef(Main_settings, "PATRON_NO_WEAPON_ICON", "/patron_no_weapon.png");
    Main_skull_icon               = GameIni_getNoLangDef(Main_settings, "SKULL_ICON", "/skull.png");
    Main_hand_icon                = GameIni_getNoLangDef(Main_settings, "HAND_ICON", "/hand.png");
    Main_background_logo          = GameIni_getNoLangDef(Main_settings, "BACKGROUND_LOGO", "/background.png");
    Main_background               = GameIni_getNoLangDef(Main_settings, "BACKGROUND", "/background2.png");
    Main_shop_patron              = GameIni_getNoLangDef(Main_settings, "SHOP_PATRON_ICON", "/icon_patron.png");
    Main_shop_medkit              = GameIni_getNoLangDef(Main_settings, "SHOP_MEDKIT_ICON", "/icon_medicine_chest.png");

    if (Main_splash) StringTools_freeStrings(Main_splash, Main_splash_count);
    Main_splash = GameIni_cutOnStrings(
        GameIni_getNoLangDef(Main_settings, "SPLASH", "/splash.png"), ',', ';', &Main_splash_count);

    Main_levelCounter = (IniFile_getIntDef(Main_settings, "COUNT_LEVELS", 0) == 1);

    Main_playerHasInventory = (IniFile_getIntDef(Main_settings, "PLAYER_HAS_INVENTORY", 0) == 1);
    InventoryScreen_proportionalInventory = (IniFile_getIntDef(Main_settings, "INVENTORY_PROPORTIONAL", 1) == 1);
    ItemsEngine_init();

    /* Le save de configuracao */
    int loaded_defaults = 0;
    if (RmsUtils_hasStore(Main_savename)) {
        size_t sz;
        void  *data = RmsUtils_openStore(Main_savename, &sz);
        if (data && sz > 0) {
            qe_bai r = { (const uint8_t*) data, sz, 0 };

            Main_music           = qe_readInt(&r);
            Main_sounds          = qe_readInt(&r);
            Main_footsteps       = qe_readInt(&r);
            Main_displaySize     = qe_readInt(&r);
            Main_availableLevel  = qe_readInt(&r);
            Main_setPersQ(qe_readInt(&r));
            Main_fogQ            = qe_readInt(&r);
            Main_pixelsQ         = qe_readInt(&r);
            Main_frameskip       = qe_readBoolean(&r);
            Main_corpses         = qe_readBoolean(&r);
            Main_blood           = qe_readBoolean(&r);
            Main_mipMapping      = qe_readBoolean(&r);
            DeveloperMenu_debugMode = qe_readBoolean(&r);
            Main_lang            = qe_readInt(&r);
            Main_mouseSpeed      = qe_readInt(&r);
            Main_hideHud         = qe_readBoolean(&r);
            Main_resizeWeapons   = qe_readBoolean(&r);
            Main_lastLevel       = qe_readInt(&r);

            /* keycodes: pode faltar em versoes antigas — Java tinha try/catch. */
            int codes = qe_readInt(&r);
            if (codes > 0 && codes < 256) {
                free(GameKeyboard_keyCodes);
                free(GameKeyboard_hasKeyCodes);
                GameKeyboard_keyCodes    = (int*) calloc(codes, sizeof(int));
                GameKeyboard_hasKeyCodes = (int*) calloc(codes, sizeof(int));
                GameKeyboard_keyCodes_len = codes;
                for (int i = 0; i < codes; i++) {
                    if (qe_readBoolean(&r)) {
                        GameKeyboard_hasKeyCodes[i] = 1;
                        GameKeyboard_keyCodes[i]    = qe_readInt(&r);
                    }
                }
            } else {
                GameKeyboard_keyCodes = NULL;
            }
            free(data);

            char lvl_next[64], lvl_this[64];
            snprintf(lvl_next, sizeof lvl_next, "/level%d.txt", Main_lastLevel + 1);
            snprintf(lvl_this, sizeof lvl_this, "/level%d.txt", Main_lastLevel);
            if (Main_isExist(lvl_next) || !Main_isExist(lvl_this))
                Main_updateLevelCount();
        } else {
            loaded_defaults = 1;
        }
    } else {
        loaded_defaults = 1;
    }
    if (loaded_defaults) Main_defaultSettings();

    tmp = GameIni_getNoLang(Main_settings, "OPEN_LEVELS");
    if (tmp != NULL) {
        if (strcmp(tmp, "ALL") == 0) Main_availableLevel = Main_lastLevel;
        else {
            int p = StringTools_parseInt(tmp);
            if (p > Main_availableLevel) Main_availableLevel = p;
        }
    }

    char *langsList = StringTools_getStringFromResource("/languages/languages.txt");
    if (langsList == NULL) {
        Main_langs = (char**) malloc(sizeof(char*) * 1);
        Main_langs[0] = strdup("ENGLISH");
        Main_langs_count = 1;
    } else {
        Main_langs = StringTools_cutOnStrings(langsList, ',', &Main_langs_count);
        free(langsList);
    }

    /* "/languages/" + langs[lang == -1 ? 0 : lang].toLowerCase() + ".txt" */
    int li = (Main_lang == -1) ? 0 : Main_lang;
    if (li < 0 || li >= Main_langs_count) li = 0;
    char lang_lc[128];
    size_t k;
    for (k = 0; Main_langs[li][k] && k + 1 < sizeof lang_lc; k++)
        lang_lc[k] = (char) tolower((unsigned char) Main_langs[li][k]);
    lang_lc[k] = 0;

    char path[256];
    snprintf(path, sizeof path, "/languages/%s.txt", lang_lc);
    Main_setLanguage(path);

    Shop_initShop();

    Main_mainCanvas = MainCanvas_new(NULL);
    /* Java: setCurrent(new SplashScreen(this)); — fica a cargo do
     * modulo Screens (splash.c) chamar Main_setCurrent(...) apos criar. */
}

void Main_startApp(void) { if (!g_run) Main_init(); }

void Main_pauseApp(void) {
    if (!Main_mainCanvas) return;
    MyCanvas *scr = MainCanvas_getScreen(Main_mainCanvas);
    if (scr && MyCanvas_isGameScreen(scr))
        GameScreen_openPause(scr);
}

void Main_destroyApp(int unconditional) { (void) unconditional; }

void Main_setCurrent(MyCanvas *canvas) {
    if (Main_mainCanvas) MainCanvas_setScreen(Main_mainCanvas, canvas);
}
void Main_setCurrentRepaint(MyCanvas *canvas) {
    Main_setCurrent(canvas);
    if (canvas) MyCanvas_repaint(canvas);
}
void Main_resetCanvas(void) {
    /* Java: Display.getDisplay(this).setCurrent(mainCanvas). Sem-op no port. */
}
void Main_setCanvas(Displayable *disp) { (void) disp; }

/* --- drawBck / drawBckDialog (Java: static) --- */

static void qe_drawBckImpl(Graphics *g, int beginy, int endy, Image *bcks) {
    if (bcks == NULL) return;
    int imgh = Image_getHeight(bcks);
    int imgw = Image_getWidth(bcks);

    int iy = 0;
    for (int y = beginy - 1; y > beginy - imgh; y--) {
        Graphics_drawRegion(g, bcks, 0, imgh - 2 - iy, imgw, 1, 0, 0, y, 0);
        iy++;
    }

    for (int y = beginy; y < endy; y++) {
        Graphics_drawRegion(g, bcks, 0, imgh - 1, imgw, 1, 0, 0, y, 0);
    }

    iy = imgh - 1;
    for (int y = endy; y < endy + imgh; y++) {
        Graphics_drawRegion(g, bcks, 0, iy, imgw, 1, 0, 0, y, 0);
        iy--;
    }
}

void Main_drawBck(Graphics *g, int beginy, int endy)       { qe_drawBckImpl(g, beginy, endy, Main_bcks); }
void Main_drawBckDialog(Graphics *g, int beginy, int endy) { qe_drawBckImpl(g, beginy, endy, Main_bcks2); }

/* ---- saveSettingToStore ---- */

void Main_saveSettingToStore(void) {
    qe_bao b; qe_bao_init(&b);

    qe_writeInt(&b, Main_music);
    qe_writeInt(&b, Main_sounds);
    qe_writeInt(&b, Main_footsteps);
    qe_writeInt(&b, Main_displaySize);
    qe_writeInt(&b, Main_availableLevel);
    qe_writeInt(&b, Main_persQ);
    qe_writeInt(&b, Main_fogQ);
    qe_writeInt(&b, Main_pixelsQ);
    qe_writeBoolean(&b, Main_frameskip);
    qe_writeBoolean(&b, Main_corpses);
    qe_writeBoolean(&b, Main_blood);
    qe_writeBoolean(&b, Main_mipMapping);
    qe_writeBoolean(&b, DeveloperMenu_debugMode);
    qe_writeInt(&b, Main_lang);
    qe_writeInt(&b, Main_mouseSpeed);
    qe_writeBoolean(&b, Main_hideHud);
    qe_writeBoolean(&b, Main_resizeWeapons);
    qe_writeInt(&b, Main_lastLevel);

    qe_writeInt(&b, GameKeyboard_keyCodes_len);
    for (int i = 0; i < GameKeyboard_keyCodes_len; i++) {
        if (GameKeyboard_hasKeyCodes[i]) {
            qe_writeBoolean(&b, 1);
            qe_writeInt(&b, GameKeyboard_keyCodes[i]);
        } else {
            qe_writeBoolean(&b, 0);
        }
    }

    RmsUtils_removeStore(Main_savename);
    RmsUtils_saveStore(Main_savename, b.buf, b.len);
    qe_bao_free(&b);
}

/* ---- saveGame ---- */
void Main_saveGame(int levelNum, Player *player, Scene *scene) {
    if (!Main_canSave) return;

    char savname[128];
    snprintf(savname, sizeof savname, "%s_Player", Main_savename);

    qe_bao b; qe_bao_init(&b);
    qe_writeInt(&b, -3); /* versao */
    qe_writeInt(&b, levelNum + 1);

    void *transform = Character_getTransform(Player_getCharacter(player));
    qe_writeInt(&b, Matrix_m03(transform));
    qe_writeInt(&b, Matrix_m13(transform));
    qe_writeInt(&b, Matrix_m23(transform));
    Player_updateMatrix(player);
    qe_writeInt(&b, ((int) Player_rotateY(player)) << 12);
    qe_writeInt(&b, ((int) Player_rotateX(player)) << 12);

    void *sp = Character_getSpeed(Player_getCharacter(player));
    qe_writeInt(&b, Vector3D_x(sp));
    qe_writeInt(&b, Vector3D_y(sp));
    qe_writeInt(&b, Vector3D_z(sp));

    qe_writeInt(&b, Player_money(player));
    qe_writeInt(&b, Player_getHp(player));
    qe_writeInt(&b, (int) Player_fov(player));
    qe_writeInt(&b, Player_stdFov(player));
    qe_writeInt(&b, Player_zoomFov(player));
    qe_writeBoolean(&b, Player_zoom(player));
    qe_writeBoolean(&b, G3D_flashlightEnabled(Scene_getG3D(scene)));

    void *arsenal = Player_arsenal(player);
    int weap_n;
    void **weaps = (void**) Arsenal_weapons(arsenal, &weap_n);
    int weapons2 = 0;
    for (int i = 0; i < weap_n; i++) if (weaps && weaps[i]) weapons2++;

    fprintf(stderr, "write weapons count\n");
    qe_writeInt(&b, weapons2);
    fprintf(stderr, "write weapon current\n");
    qe_writeInt(&b, Arsenal_current(arsenal));

    for (int i = 0; i < weap_n; i++) {
        void *weapon = weaps ? weaps[i] : NULL;
        if (weapon != NULL) {
            qe_writeInt(&b, i);
            void *mag = Weapon_magazine(weapon);
            qe_writeShort(&b, Magazine_ammo(mag));
            qe_writeShort(&b, Magazine_rounds(mag));
        }
    }

    fprintf(stderr, "write keys\n");
    void *usedPoints = Player_usedPoints();
    if (usedPoints != NULL) {
        int el = Vector_size(usedPoints);
        qe_writeInt(&b, el);
        for (int i = 0; i < el; i++) {
            qe_writeUTF(&b, (const char*) Vector_elementAt(usedPoints, i));
        }
    } else {
        qe_writeInt(&b, 0);
    }

    if (Main_playerHasInventory)
        PlayerItems_writeSave(Player_items(player), &b);

    fprintf(stderr, "Game saved\n");

    RmsUtils_removeStore(savname);
    RmsUtils_saveStore(savname, b.buf, b.len);
    qe_bao_free(&b);
}

/* ---- saveObjects ---- */
void Main_saveObjects(int levelNum, Player *player, Scene *scene) {
    (void) player;
    if (!Main_canSave) return;

    char savname[128];
    snprintf(savname, sizeof savname, "%s_lvl%d", Main_savename, levelNum);

    void *rmsBots = Scene_rmsBots(scene);
    int saveBots = (rmsBots != NULL) && (Vector_size(rmsBots) > 0);
    void *bags = Scene_getItemBags(scene);
    void *rmsObjs = Scene_rmsObjects(scene);
    int bags_n = bags ? Vector_size(bags) : 0;
    int objs_n = rmsObjs ? Vector_size(rmsObjs) : 0;

    if (!saveBots && bags_n == 0 && objs_n == 0) return;

    qe_bao b; qe_bao_init(&b);

    if (saveBots) {
        int n = Vector_size(rmsBots);
        qe_writeInt(&b, n);
        for (int i = 0; i < n; i++) {
            void *obj = Vector_elementAt(rmsBots, i);
            if (RoomObject_isNPC(obj))              qe_writeBoolean(&b, NPC_isDead(obj));
            else if (RoomObject_isNPCSpawner(obj))  qe_writeBoolean(&b, NPCSpawner_deathCheck(obj));
            else                                    qe_writeBoolean(&b, 0);
        }
    } else {
        qe_writeInt(&b, 0);
    }

    qe_writeInt(&b, bags_n);
    for (int i = 0; i < bags_n; i++) {
        void *obj = Vector_elementAt(bags, i);
        ItemsBag_writeSave(obj, &b);
    }

    qe_writeInt(&b, objs_n);
    for (int i = 0; i < objs_n; i++) {
        void *obj = Vector_elementAt(rmsObjs, i);
        qe_writeBoolean(&b, RoomObject_activated(obj));
    }

    fprintf(stderr, "Game saved\n");

    RmsUtils_removeStore(savname);
    RmsUtils_saveStore(savname, b.buf, b.len);
    qe_bao_free(&b);
}

/* ---- loadGame ---- */
void Main_loadGame(Player *player, int w, int h, Scene *scene) {
    if (!Main_canSave) return;

    char savname[128];
    snprintf(savname, sizeof savname, "%s_Player", Main_savename);

    if (!RmsUtils_hasStore(savname)) return;
    size_t sz;
    void *data = RmsUtils_openStore(savname, &sz);
    if (!data) { fprintf(stderr, "Error reading save file\n"); return; }

    qe_bai r = { (const uint8_t*) data, sz, 0 };

    int version = qe_readInt(&r);
    if (version < 0) qe_readInt(&r); /* skip level number */

    if (version >= 0) {
        for (int i = 0; i < 15; i++) qe_readInt(&r); /* pula 15 ints antigos */
    } else if (version <= -1) {
        for (int i = 0; i < 8; i++)  qe_readInt(&r);
    }

    Player_setMoney(player, qe_readInt(&r));
    Player_setHp(player,    qe_readInt(&r));
    Player_setFov(player,   (float) qe_readInt(&r));
    Player_setStdFov(player, qe_readInt(&r));
    Player_setZoomFov(player, qe_readInt(&r));
    Player_setZoom(player,  qe_readBoolean(&r));
    if (version <= -3) G3D_setFlashlightEnabled(Scene_getG3D(scene), qe_readBoolean(&r));

    int weapc   = qe_readInt(&r);
    int current = qe_readInt(&r);

    void *arsenal = Player_arsenal(player);
    Arsenal_destroy(arsenal);
    Arsenal_setCurrent(arsenal, current);

    void **weapons = (void**) calloc(Shop_weaponCount > 0 ? Shop_weaponCount : 1, sizeof(void*));

    for (int i = 0; i < weapc; i++) {
        int tw = qe_readInt(&r);
        if (tw >= 0 && tw < Shop_weaponCount) {
            weapons[tw] = WeaponCreator_createWeapon(tw);
            Weapon_createSprite(weapons[tw], w, h * Main_displaySize / 100);
            int ammo   = qe_readShort(&r);
            int rounds = qe_readShort(&r);
            Magazine_set(Weapon_magazine(weapons[tw]), ammo, rounds);
        } else {
            qe_readShort(&r); qe_readShort(&r);
        }
    }
    Arsenal_setWeapons(arsenal, weapons, Shop_weaponCount);
    Vector_removeAllElements(Player_usedPoints());

    int objs = qe_readInt(&r);
    for (int i = 0; i < objs; i++) {
        char *s = qe_readUTF(&r);
        Vector_addElement(Player_usedPoints(), s);
    }

    if (version <= -2) {
        if (Main_playerHasInventory) PlayerItems_loadSave(Player_items(player), &r);
    }

    free(data);
}

/* ---- loadObjects ---- */
void Main_loadObjects(Player *player, int w, int h, Scene *scene, int levelNumber) {
    (void) player; (void) w; (void) h;
    if (!Main_canSave) return;

    char savname[128];
    snprintf(savname, sizeof savname, "%s_lvl%d", Main_savename, levelNumber);

    if (!RmsUtils_hasStore(savname)) return;
    size_t sz;
    void *data = RmsUtils_openStore(savname, &sz);
    if (!data) { fprintf(stderr, "Error reading objects file\n"); return; }
    qe_bai r = { (const uint8_t*) data, sz, 0 };

    int savedBots = qe_readInt(&r);
    if (savedBots == 0) {
        Scene_setRmsBotsKilled(scene, NULL, 0);
    } else {
        int *arr = (int*) malloc(sizeof(int) * savedBots);
        for (int i = 0; i < savedBots; i++) arr[i] = qe_readBoolean(&r);
        Scene_setRmsBotsKilled(scene, arr, savedBots);
    }

    int savedBags = qe_readInt(&r);
    for (int i = 0; i < savedBags; i++) {
        void *bag = ItemsBag_new(Vector3D_new(0, 0, 0));
        ItemsBag_loadSave(bag, &r);
        House_addObject(Scene_getHouse(scene), bag);
    }

    int savedObjs = qe_readInt(&r);
    if (savedObjs == 0) {
        Scene_setRmsObjectsDestroyed(scene, NULL, 0);
    } else {
        int *arr = (int*) malloc(sizeof(int) * savedObjs);
        for (int i = 0; i < savedObjs; i++) arr[i] = qe_readBoolean(&r);
        Scene_setRmsObjectsDestroyed(scene, arr, savedObjs);
    }

    free(data);
}

/* ---- loadPosition ---- */
void Main_loadPosition(Player *player) {
    if (!Main_canSave) return;

    char savname[128];
    snprintf(savname, sizeof savname, "%s_Player", Main_savename);
    if (!RmsUtils_hasStore(savname)) return;

    size_t sz;
    void *data = RmsUtils_openStore(savname, &sz);
    if (!data) { fprintf(stderr, "Error reading player position save\n"); return; }
    qe_bai r = { (const uint8_t*) data, sz, 0 };

    int version = qe_readInt(&r);
    if (version < 0) qe_readInt(&r);

    void *ch = Player_getCharacter(player);

    if (version >= 0) {
        int a=qe_readInt(&r)*0, b=qe_readInt(&r)*0, c=qe_readInt(&r)*0;
        int d=qe_readInt(&r);
        int e=qe_readInt(&r)*0, f=qe_readInt(&r)*0, g=qe_readInt(&r)*0;
        int hh=qe_readInt(&r);
        int i=qe_readInt(&r)*0, j=qe_readInt(&r)*0, k=qe_readInt(&r)*0;
        int l=qe_readInt(&r);
        Matrix_set(Character_getTransform(ch), a,b,c,d, e,f,g,hh, i,j,k,l);

        int sx=qe_readInt(&r), sy=qe_readInt(&r), sz=qe_readInt(&r);
        Vector3D_set(Character_getSpeed(ch), sx, sy, sz);
        Player_updateMatrix(player);

    } else if (version <= -1) {
        Matrix_setIdentity(Character_getTransform(ch));
        int x = qe_readInt(&r), y = qe_readInt(&r), z = qe_readInt(&r);
        Matrix_setPosition(Character_getTransform(ch), x, y, z);

        Player_rotYn(player, qe_readInt(&r) / 4096.0f);
        Player_rotXn(player, qe_readInt(&r) / 4096.0f);

        int sx=qe_readInt(&r), sy=qe_readInt(&r), sz=qe_readInt(&r);
        Vector3D_set(Character_getSpeed(ch), sx, sy, sz);
        Player_updateMatrix(player);
    }

    free(data);
}

/* ---- loadLevel ---- */
void Main_loadLevel(int loadSave, int loadPos, int levelNumber,
                    void *hudInfo, Menu *menu, int helpState, int showLoad) {
    /* System.gc(); Thread.sleep(5L); — no port DC, sleep 5ms opcional. */

    if (!GameHelp_needToShow(levelNumber, helpState, 0)) {
        if (menu != NULL) Menu_destroy(menu);
        void *ls = LoadingScreen_new(NULL, levelNumber, loadSave, loadPos, showLoad);
        Main_setCurrent((MyCanvas*) ls);
    } else {
        void *gh = GameHelp_new(NULL, menu, levelNumber, 0, hudInfo, helpState, showLoad);
        GameHelp_loadSave = loadSave;
        GameHelp_loadpos  = loadPos;
        Main_setCurrent((MyCanvas*) gh);
    }
}

/* ---- removeSave / continueLevel / hasSave ---- */

void Main_removeSave(void) {
    if (!Main_canSave) return;

    char savname[128];
    snprintf(savname, sizeof savname, "%s_Player", Main_savename);
    RmsUtils_removeStore(savname);

    for (int i = 0; i < Main_lastLevel; i++) {
        snprintf(savname, sizeof savname, "%s_lvl%d", Main_savename, i);
        RmsUtils_removeStore(savname);
    }
}

int Main_getContinueLevel(void) {
    if (!Main_canSave) return 1;
    char savname[128]; snprintf(savname, sizeof savname, "%s_Player", Main_savename);
    if (!RmsUtils_hasStore(savname)) return 1;

    size_t sz;
    void *data = RmsUtils_openStore(savname, &sz);
    if (!data) { fprintf(stderr, "Error getting continue level number\n"); return 1; }
    qe_bai r = { (const uint8_t*) data, sz, 0 };
    int lvl = qe_readInt(&r);
    if (lvl < 0) lvl = qe_readInt(&r);
    free(data);
    return lvl - 1;
}

int Main_hasSave(void) {
    if (!Main_canSave) return 0;
    char savname[128]; snprintf(savname, sizeof savname, "%s_Player", Main_savename);
    if (!RmsUtils_hasStore(savname)) return 0;

    size_t sz;
    void *data = RmsUtils_openStore(savname, &sz);
    if (!data) return 0;
    qe_bai r = { (const uint8_t*) data, sz, 0 };
    int lvl = qe_readInt(&r);
    if (lvl < 0) lvl = qe_readInt(&r);
    free(data);
    return lvl != 0;
}
