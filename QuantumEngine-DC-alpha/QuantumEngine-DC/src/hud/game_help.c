/*
 * game_help.c — porte fiel de code/HUD/GameHelp.java
 */
#include "game_help.h"
#include "menu.h"
#include "title_screen.h"
#include "base/font.h"
#include "base/text_view.h"
#include "../utils/main.h"
#include "../utils/image_resize.h"
#include "../utils/inifile.h"
#include "../utils/canvas/main_canvas.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int   GameHelp_loadSave = 0;
int   GameHelp_loadpos  = 1;
short GameHelp_toMove   = 0;

/* Player.usedPoints (Vector estatico Gameplay/AI) */
extern void *Player_usedPoints_v(void);
extern void  Player_toAddOnStart_addElement(const char *s);
extern int   RoomObject_containsSimple(void *vec, const char *s);

extern Image *ImageResize_createImage_sc(Image *img, float sx, float sy);
extern Image *ImageResize_createImage_wh(Image *img, int w, int h);
extern Image *Image_createImage(const char *file);
extern int    Image_getWidth (Image *img);
extern int    Image_getHeight(Image *img);
extern void   Graphics_setColor(Graphics *g, int c);
extern void   Graphics_fillRect(Graphics *g, int x, int y, int w, int h);
extern void   Graphics_drawImage(Graphics *g, Image *img, int x, int y, int anchor);
extern void   Graphics_translate(Graphics *g, int x, int y);
extern void   Graphics_setClip  (Graphics *g, int x, int y, int w, int h);

static inline int qe_max(int a, int b) { return a > b ? a : b; }
static inline int qe_min(int a, int b) { return a < b ? a : b; }

void GameHelp_init(GameHelp *self, Main *main, Menu *menu, int levelNumberz, int levelEnd,
                    void *hudInfo, int code, int showLoad) {
    self->main = main; self->menu = menu;
    GameHelp_loadSave = 0; GameHelp_loadpos = 1;
    self->levelNumber = levelNumberz;
    self->hudInfo = hudInfo;
    self->stateCode = code;
    self->showLoad = showLoad;
    self->levelEnd = levelEnd;
    GUIScreen_setFont(&self->base, Main_getFont());
    GUIScreen_setLeftSoft(&self->base, IniFile_get(Main_getGameText(), "NEXT"));

    const char *prefix = levelEnd ? "LEVEL_END_" : "GAME_HELP_";
    char add[16] = "";
    if (self->stateCode > 1) snprintf(add, sizeof add, "_%d", self->stateCode);

    char buf[128];
    snprintf(buf, sizeof buf, "%sSCREEN_%d%s", prefix, levelNumberz, add);
    Player_toAddOnStart_addElement(buf);

    self->x = MyCanvas_getWidth(&self->base.base) / 15;
    self->y = Font_height(Main_getFont());
    GameHelp_toMove = 0;

    const char *txt = NULL;
    if (levelNumberz != 1) {
        char k[128]; snprintf(k, sizeof k, "%sTEXT_%d%s", prefix, levelNumberz, add);
        txt = IniFile_get(Main_getGameText(), k);
    } else {
        char k[128]; snprintf(k, sizeof k, "%sTEXT", prefix);
        txt = IniFile_get(Main_getGameText(), k);
        if (!txt) { snprintf(k, sizeof k, "%sTEXT_1%s", prefix, add); txt = IniFile_get(Main_getGameText(), k); }
    }
    if (!txt) txt = "";

    char k1[128], k2[128];
    snprintf(k1, sizeof k1, "%sPREVIEW_%d%s",    prefix, levelNumberz, add);
    snprintf(k2, sizeof k2, "%sBACKGROUND_%d%s", prefix, levelNumberz, add);
    const char *back1 = IniFile_get(Main_getGameText(), k1);
    const char *back2 = IniFile_get(Main_getGameText(), k2);

    int textHeight = MyCanvas_getHeight(&self->base.base) - self->y * 2;
    self->text = TextView_new(txt, MyCanvas_getWidth(&self->base.base) - self->x * 2, textHeight, Main_getFont());
    TextView_setCenter(self->text, 1);

    if (back1) {
        int mn = qe_min(MyCanvas_getWidth(&self->base.base), MyCanvas_getHeight(&self->base.base));
        float sc = mn / 240.0f;
        self->background = ImageResize_createImage_sc(Image_createImage(back1), sc, sc);
        int offset = Image_getHeight(self->background) + Font_height(Main_getFont()) * 2;
        self->y = offset + Font_height(Main_getFont());
        TextView_setHeight(self->text, MyCanvas_getHeight(&self->base.base) - offset - Font_height(Main_getFont()) * 2);
        self->preview = 1;
    } else if (back2) {
        self->background = ImageResize_createImage_wh(Image_createImage(back2), MyCanvas_getWidth(&self->base.base), MyCanvas_getHeight(&self->base.base));
    } else if (menu && menu->background) {
        self->background = menu->background;
    } else if (!menu && !self->background) {
        self->background = ImageResize_createImage_wh(Image_createImage(Main_background), MyCanvas_getWidth(&self->base.base), MyCanvas_getHeight(&self->base.base));
    }

    if (TextView_getTextHeight(self->text) < TextView_getHeight(self->text) && !self->preview) {
        TextView_setY(self->text, qe_max(0, (TextView_getHeight(self->text) - TextView_getTextHeight(self->text)) / 2));
    }
}

static void GameHelp_paint(MyCanvas *mc, Graphics *g) {
    GameHelp *self = (GameHelp*) mc;
    Graphics_setColor(g, 0);
    Graphics_fillRect(g, 0, 0, MyCanvas_getWidth(mc), MyCanvas_getHeight(mc));

    if (self->base.pressUp   && GameHelp_toMove == 0 && !self->base.base.pointerPressed) GameHelp_toMove += Font_height(Main_getFont());
    if (self->base.pressDown && GameHelp_toMove == 0 && !self->base.base.pointerPressed) GameHelp_toMove -= Font_height(Main_getFont());
    if (GameHelp_toMove != 0 && !self->base.base.pointerPressed && TextView_getTextHeight(self->text) > TextView_getHeight(self->text)) {
        int step = qe_max(qe_min(3, GameHelp_toMove), -3);
        TextView_move(self->text, step);
        GameHelp_toMove -= step;
    }

    if (self->background && self->preview) {
        Graphics_setColor(g, 0xffffff);
        int posX = (MyCanvas_getWidth(mc) - Image_getWidth(self->background)) / 2;
        Graphics_drawImage(g, self->background, posX, (self->y - Image_getHeight(self->background)) / 2, 0);
    } else if (self->background) {
        Graphics_setColor(g, 0xffffff);
        Graphics_drawImage(g, self->background, (MyCanvas_getWidth(mc) - Image_getWidth(self->background)) / 2, (MyCanvas_getHeight(mc) - Image_getHeight(self->background)) / 2, 0);
    }
    TextView_paint(self->text, g, self->x, self->y);
    Graphics_translate(g, 0, 0);
    Graphics_setClip(g, 0, 0, MyCanvas_getWidth(mc), MyCanvas_getHeight(mc));
    GUIScreen_drawSoftKeys(&self->base, g);
    if (!self->base.base.pointerPressed) MyCanvas_repaint(mc);
}

static void GameHelp_onKey5(GUIScreen *gs) {
    GameHelp *self = (GameHelp*) gs;
    if (self->levelEnd) {
        if (GameHelp_needToShow(self->levelNumber, self->stateCode + 1, 1)) {
            GameHelp_init(self, self->main, self->menu, self->levelNumber, 1, self->hudInfo, self->stateCode + 1, self->showLoad);
        } else {
            self->background = NULL;
            GUIScreen_destroy(&self->base);
            if (Main_isLastLevel(self->levelNumber)) {
                MyCanvas *scr = TitleScreen_hasTitleScreen()
                    ? (MyCanvas*) TitleScreen_new(self->main, NULL)
                    : (MyCanvas*) Menu_new(self->main);
                Main_setCurrent(scr);
            } else {
                Main_loadLevel(GameHelp_loadSave, GameHelp_loadpos, self->levelNumber + 1, self->hudInfo, self->menu, 1, self->showLoad);
            }
        }
    } else {
        self->background = NULL;
        GUIScreen_destroy(&self->base);
        Main_loadLevel(GameHelp_loadSave, GameHelp_loadpos, self->levelNumber, self->hudInfo, self->menu, self->stateCode + 1, self->showLoad);
    }
}
static void GameHelp_onLeftSoftKey(GUIScreen *gs) { GameHelp_onKey5(gs); }
static void GameHelp_pointerPressed(MyCanvas *mc, int x, int y) {
    GameHelp *self = (GameHelp*) mc;
    GUIScreen_default_pointerPressed(mc, x, y);
    self->y0 = y;
}
static void GameHelp_pointerReleased(MyCanvas *mc, int x, int y) {
    MyCanvas_default_pointerReleased(mc, x, y);
    MyCanvas_repaint(mc);
}
static void GameHelp_pointerDragged(MyCanvas *mc, int x, int y) {
    GameHelp *self = (GameHelp*) mc; (void) x;
    int dy = y - self->y0; self->y0 = y;
    GameHelp_toMove = 0;
    MyCanvas_serviceRepaints(mc);
    TextView_move(self->text, dy);
    MyCanvas_repaint(mc);
}

static const GUIScreenVTable g_vt = {
    {
        GameHelp_paint,
        GUIScreen_default_keyPressed, GUIScreen_default_keyRepeated, GUIScreen_default_keyReleased,
        GameHelp_pointerPressed, MyCanvas_default_mouseScrollDown, MyCanvas_default_mouseScrollUp,
        GameHelp_pointerReleased, GameHelp_pointerDragged, MyCanvas_default_pointerClicked,
        MyCanvas_default_sizeChanged, MyCanvas_default_showNotify, MyCanvas_default_hideNotify
    },
    GameHelp_onLeftSoftKey, NULL,
    GameHelp_onKey5, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

GameHelp *GameHelp_new(Main *main, Menu *menu, int lvl, int end, void *hi, int hs, int sl) {
    GameHelp *h = (GameHelp*) calloc(1, sizeof(GameHelp));
    GUIScreen_init(&h->base, &g_vt);
    GameHelp_init(h, main, menu, lvl, end, hi, hs, sl);
    return h;
}
void GameHelp_free(GameHelp *h) { if (!h) return; GUIScreen_destroy(&h->base); if (h->text) TextView_free(h->text); free(h); }

int GameHelp_needToShow(int levelNumber, int helpState, int levelEnd) {
    char add[16] = "";
    if (helpState > 1) snprintf(add, sizeof add, "_%d", helpState);
    const char *prefix = levelEnd ? "LEVEL_END_" : "GAME_HELP_";

    char k1[128], k2[128], k3[128], k4[128], k5[128], k6[128];
    snprintf(k1, sizeof k1, "%sTEXT_%d%s",       prefix, levelNumber, add);
    snprintf(k2, sizeof k2, "%sTEXT",            prefix);
    snprintf(k3, sizeof k3, "%sPREVIEW_%d%s",    prefix, levelNumber, add);
    snprintf(k4, sizeof k4, "%sBACKGROUND_%d%s", prefix, levelNumber, add);
    snprintf(k5, sizeof k5, "%sSCREEN_%d%s",     prefix, levelNumber, add);
    snprintf(k6, sizeof k6, "%sVIEW_ONCLE_%d%s", prefix, levelNumber, add);

    int dontNeedToShow =
        (IniFile_get(Main_getGameText(), k1) == NULL
         && (levelNumber != 1 || add[0] != 0 || IniFile_get(Main_getGameText(), k2) == NULL)
         && IniFile_get(Main_getGameText(), k3) == NULL
         && IniFile_get(Main_getGameText(), k4) == NULL)
        || (RoomObject_containsSimple(Player_usedPoints_v(), k5)
            && IniFile_getIntDef(Main_getGameText(), k6, 0) == 1);
    return !dontNeedToShow;
}
