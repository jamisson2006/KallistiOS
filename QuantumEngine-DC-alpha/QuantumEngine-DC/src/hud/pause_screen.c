/*
 * pause_screen.c — porte fiel de code/HUD/PauseScreen.java
 */
#include "pause_screen.h"
#include "menu.h"
#include "setting.h"
#include "developer_menu.h"
#include "loading_screen.h"
#include "base/font.h"
#include "base/item_list.h"
#include "../utils/main.h"
#include "../utils/inifile.h"
#include "../utils/canvas/main_canvas.h"

#include <stdlib.h>
#include <string.h>

extern int DeveloperMenu_debugMode;
extern void Graphics_setColor(Graphics *g, int c);
extern void Graphics_fillRect(Graphics *g, int x, int y, int w, int h);
extern void Graphics_drawRGB (Graphics *g, int *arr, int off, int stride, int x, int y, int w, int h, int alpha);
extern void Main_notifyDestroyed(Main *m);

extern int   GameScreen_levelNumber(GameScreen *gs);
extern void *GameScreen_player(GameScreen *gs);
extern void *GameScreen_scene(GameScreen *gs);
extern int   Player_getHp_v(void *player);
extern void  GameScreen_destroy_gs(GameScreen *gs);
extern void  GameScreen_init(GameScreen *gs);
extern void  GameScreen_destroyMusic(GameScreen *gs);
extern void  GameScreen_resize(GameScreen *gs);
extern void  GameScreen_setPaused(GameScreen *gs, int p);
extern void  GameScreen_startMus(GameScreen *gs, long long mt);
extern void  GameScreen_setMusTime(GameScreen *gs, long long mt);
extern void *Scene_getG3D_ps(void *scene);
extern void  G3D_resize(void *g3d, int w, int h);
extern void  GameScreen_start_ps(GameScreen *gs);

static int items_count(void) {
    return (Main_pauseScreenSave ? 6 : 5) + (DeveloperMenu_debugMode ? 1 : 0);
}

void PauseScreen_reloadText(PauseScreen *self) {
    IniFile *text = Main_getGameText();
    int total = items_count();
    char **items = (char**) calloc(total, sizeof(char*));
    int i = 0;
    items[i++] = strdup(IniFile_get(text, "CONTINUE"));
    items[i++] = strdup(IniFile_get(text, "RELOAD"));
    if (Main_pauseScreenSave) items[i++] = strdup(IniFile_get(text, "SAVE_GAME"));
    items[i++] = strdup(IniFile_get(text, "OPTIONS"));
    items[i++] = strdup(IniFile_get(text, "MENU"));
    items[i++] = strdup(IniFile_get(text, "EXIT"));
    if (DeveloperMenu_debugMode) items[i++] = strdup("Developer Menu");
    Selectable_set(&self->base, Main_getFont(), items, total, IniFile_get(text, "SELECT"), IniFile_get(text, "BACK"));
}

static void PauseScreen_paint(MyCanvas *mc, Graphics *g);
static void PauseScreen_onLeftSoftKey(GUIScreen *gs);
static void PauseScreen_onRightSoftKey(GUIScreen *gs);
static void PauseScreen_onKey5(GUIScreen *gs) { PauseScreen_onLeftSoftKey(gs); }

static const GUIScreenVTable g_vt = {
    {
        PauseScreen_paint,
        GUIScreen_default_keyPressed, GUIScreen_default_keyRepeated, GUIScreen_default_keyReleased,
        GUIScreen_default_pointerPressed, MyCanvas_default_mouseScrollDown, MyCanvas_default_mouseScrollUp,
        MyCanvas_default_pointerReleased, GUIScreen_default_pointerDragged, MyCanvas_default_pointerClicked,
        MyCanvas_default_sizeChanged, MyCanvas_default_showNotify, MyCanvas_default_hideNotify
    },
    PauseScreen_onLeftSoftKey, PauseScreen_onRightSoftKey,
    PauseScreen_onKey5, NULL, NULL,
    Selectable_onKey2, Selectable_onKey8, NULL, NULL, NULL, NULL, NULL, NULL
};

PauseScreen *PauseScreen_new(Main *main, GameScreen *gs, int *background, long long time) {
    PauseScreen *p = (PauseScreen*) calloc(1, sizeof(PauseScreen));
    GUIScreen_init(&p->base.base, &g_vt);
    p->main = main; p->gameScreen = gs; p->background = background;
    p->mus = Main_isMusic; p->lastIndex = 0; p->mt = time; p->state = 0;
    p->saveGame = 1;
    PauseScreen_reloadText(p);
    p->height3D    = MyCanvas_getHeight(&p->base.base.base) / 2 - MyCanvas_getHeight(&p->base.base.base) * Main_getDisplaySize() / 200;
    p->backgroundH = MyCanvas_getHeight(&p->base.base.base) * Main_getDisplaySize() / 100;
    MyCanvas_repaint(&p->base.base.base);
    return p;
}
void PauseScreen_free(PauseScreen *p) { if (!p) return; Selectable_destroy(&p->base); free(p); }

static void PauseScreen_paint(MyCanvas *mc, Graphics *g) {
    PauseScreen *self = (PauseScreen*) mc;
    IniFile *text = Main_getGameText();
    Graphics_setColor(g, 0);
    if (self->state == 0) self->lastIndex = ItemList_getIndex(self->base.list);
    Graphics_fillRect(g, 0, 0, MyCanvas_getWidth(mc), MyCanvas_getHeight(mc));

    if (self->background) Graphics_drawRGB(g, self->background, 0, MyCanvas_getWidth(mc), 0, self->height3D, MyCanvas_getWidth(mc), self->backgroundH, 0);

    Graphics_setColor(g, 0xffffff);
    Font *font = Main_getFont();
    Main_drawBckDialog(g, MyCanvas_getHeight(mc) - Font_height(font), MyCanvas_getHeight(mc));

    if (self->state == 0) {
        GUIScreen_setSoftKeysNames(&self->base.base, IniFile_get(text, "SELECT"), IniFile_get(text, "BACK"));
        ItemList_drawBck(self->base.list, g, 0, Font_height(font), MyCanvas_getWidth(mc), MyCanvas_getHeight(mc) - Font_height(font) * 2);
        Selectable_paint(mc, g);
        int y = (MyCanvas_getHeight(mc) - Font_height(font) * 2 - ItemList_getHeight(self->base.list)) / 2 + Font_height(font);
        y /= 3;
        Main_drawBckDialog(g, y - Font_height(font) / 2, y + Font_height(font) / 2);
        Font_drawString(font, g, IniFile_get(text, "PAUSE"), MyCanvas_getWidth(mc) / 2, y, 3);
    } else if (self->state == 1 || self->state == 2) {
        int y = MyCanvas_getHeight(mc) / 2;
        Main_drawBckDialog(g, y - Font_height(font) / 2, y + Font_height(font) / 2);
        Font_drawString(font, g, IniFile_get(text, self->state == 1 ? "CONFIRM" : "SAVE_GAME?"), MyCanvas_getWidth(mc) / 2, y, 3);
        GUIScreen_setSoftKeysNames(&self->base.base, IniFile_get(text, "YES"), IniFile_get(text, "NO"));
        GUIScreen_drawSoftKeys(&self->base.base, g);
    }
}

static void closeGS(PauseScreen *self);
static void checkVariant(PauseScreen *self);

static void PauseScreen_onRightSoftKey(GUIScreen *gs) {
    PauseScreen *self = (PauseScreen*) gs;
    if (self->state == 0) {
        Selectable_setItemIndex(&self->base, 0);
        checkVariant(self);
    } else if (self->state == 1) {
        ItemList_setIndex(self->base.list, self->lastIndex);
        self->state = 0; MyCanvas_repaint((MyCanvas*) gs);
    } else if (self->state == 2) {
        ItemList_setIndex(self->base.list, self->lastIndex);
        self->saveGame = 0; closeGS(self);
    }
}

static void PauseScreen_onLeftSoftKey(GUIScreen *gs) {
    PauseScreen *self = (PauseScreen*) gs;
    int itemsN; Selectable_getItems(&self->base, &itemsN);
    int idx = Selectable_itemIndex(&self->base);
    if (self->state == 0 && (idx < (Main_pauseScreenSave ? 4 : 3)
        || (idx == itemsN - 1 && DeveloperMenu_debugMode)) && idx != 1
        && (idx != 2 || !Main_pauseScreenSave)) {
        checkVariant(self);
    } else {
        if (self->state == 2) {
            ItemList_setIndex(self->base.list, self->lastIndex);
            closeGS(self);
        } else if (self->state == 1) {
            ItemList_setIndex(self->base.list, self->lastIndex);
            if (!Main_saveOnQuit || idx == 1 || (idx == 2 && Main_pauseScreenSave)) closeGS(self);
            else { self->state = 2; MyCanvas_repaint((MyCanvas*) gs); }
        } else {
            self->state = 1; MyCanvas_repaint((MyCanvas*) gs);
        }
    }
}

static void closeGS(PauseScreen *self) {
    int index = Selectable_itemIndex(&self->base);
    if (index == (Main_pauseScreenSave ? 4 : 3)) {
        if (Main_saveOnQuit && self->saveGame && Player_getHp_v(GameScreen_player(self->gameScreen)) > 0) {
            Main_saveGame(GameScreen_levelNumber(self->gameScreen), GameScreen_player(self->gameScreen), GameScreen_scene(self->gameScreen));
            Main_saveObjects(GameScreen_levelNumber(self->gameScreen), GameScreen_player(self->gameScreen), GameScreen_scene(self->gameScreen));
        }
        GameScreen_destroy_gs(self->gameScreen); self->gameScreen = NULL; self->background = NULL;
        Main_setCurrent((MyCanvas*) Menu_new(self->main));
    } else if (index == (Main_pauseScreenSave ? 5 : 4)) {
        if (Main_saveOnQuit && self->saveGame && Player_getHp_v(GameScreen_player(self->gameScreen)) > 0) {
            Main_saveGame(GameScreen_levelNumber(self->gameScreen), GameScreen_player(self->gameScreen), GameScreen_scene(self->gameScreen));
            Main_saveObjects(GameScreen_levelNumber(self->gameScreen), GameScreen_player(self->gameScreen), GameScreen_scene(self->gameScreen));
        }
        Main_saveOnQuit = 0;
        GameScreen_destroy_gs(self->gameScreen); self->gameScreen = NULL; self->background = NULL;
        Main_notifyDestroyed(self->main);
    } else if (index == 1) {
        self->background = NULL;
        int hasSave = Main_hasSave();
        int lvl = hasSave ? Main_getContinueLevel() : GameScreen_levelNumber(self->gameScreen);
        GameScreen_destroy_gs(self->gameScreen); self->gameScreen = NULL;
        Main_setCurrent((MyCanvas*) LoadingScreen_new(self->main, lvl, hasSave, 1, 1));
    } else if (index == 2 && Main_pauseScreenSave) {
        if (Player_getHp_v(GameScreen_player(self->gameScreen)) > 0) {
            Main_saveGame(GameScreen_levelNumber(self->gameScreen), GameScreen_player(self->gameScreen), GameScreen_scene(self->gameScreen));
            Main_saveObjects(GameScreen_levelNumber(self->gameScreen), GameScreen_player(self->gameScreen), GameScreen_scene(self->gameScreen));
        }
        self->state = 0; MyCanvas_repaint((MyCanvas*) &self->base.base.base);
    }
}

static void checkVariant(PauseScreen *self) {
    int item = Selectable_itemIndex(&self->base);
    int itemsN; Selectable_getItems(&self->base, &itemsN);
    if (item == 0) {
        if (Main_isMusic != self->mus) {
            if (Main_isMusic) { GameScreen_setMusTime(self->gameScreen, 0); self->mt = 0; GameScreen_init(self->gameScreen); }
            else GameScreen_destroyMusic(self->gameScreen);
        }
        int newH3D = MyCanvas_getHeight(&self->base.base.base) / 2 - MyCanvas_getHeight(&self->base.base.base) * Main_getDisplaySize() / 200;
        if (self->height3D != newH3D) {
            GameScreen_resize(self->gameScreen);
            G3D_resize(Scene_getG3D_ps(GameScreen_scene(self->gameScreen)),
                       MyCanvas_getWidth(&self->base.base.base),
                       MyCanvas_getHeight(&self->base.base.base) * Main_getDisplaySize() / 100);
        }
        GameScreen_setPaused(self->gameScreen, 0);
        Main_setCurrent((MyCanvas*) self->gameScreen);
        GameScreen_start_ps(self->gameScreen);
        if (Main_isMusic) GameScreen_startMus(self->gameScreen, self->mt);
        self->background = NULL; self->gameScreen = NULL;
    }
    if (item == (Main_pauseScreenSave ? 3 : 2)) Main_setCurrent((MyCanvas*) Setting_new(self->main, self, self->background));
    if (item == itemsN - 1 && DeveloperMenu_debugMode) Main_setCurrent((MyCanvas*) DeveloperMenu_new(self->main, self, self->background));
}
