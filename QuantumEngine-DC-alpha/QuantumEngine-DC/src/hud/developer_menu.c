/*
 * developer_menu.c — porte fiel de code/HUD/DeveloperMenu.java
 */
#include "developer_menu.h"
#include "menu.h"
#include "pause_screen.h"
#include "video_player.h"
#include "base/font.h"
#include "base/item_list.h"
#include "../utils/main.h"
#include "../utils/inifile.h"
#include "../utils/canvas/main_canvas.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int DeveloperMenu_debugMode              = 0;
int DeveloperMenu_drawLevelPoints        = 1;
int DeveloperMenu_drawPortals            = 0;
int DeveloperMenu_renderPolygonsOverwrite= 0;
int DeveloperMenu_godMode                = 1;
int DeveloperMenu_showShootCollision     = 0;
int DeveloperMenu_showFps                = 1;
int DeveloperMenu_showRam                = 1;
int DeveloperMenu_showRoomID             = 0;
int DeveloperMenu_showPlayerPos          = 0;
int DeveloperMenu_fly                    = 0;

extern void Graphics_setColor(Graphics *g, int c);
extern void Graphics_fillRect(Graphics *g, int x, int y, int w, int h);

static int is_pause_screen_(MyCanvas *mc) { (void) mc; return 0; }
static int is_menu_(MyCanvas *mc)         { (void) mc; return 0; }

static void setItems(DeveloperMenu *self) {
    int n; char **items = Selectable_getItems(&self->base, &n);
    for (int i = 0; i < n; i++) { free(items[i]); items[i] = NULL; }
    int i = 0;
    #define QE_STR_BOOL(b) ((b) ? "true" : "false")
    char buf[128];
    snprintf(buf, sizeof buf, "Debug: %s", QE_STR_BOOL(DeveloperMenu_debugMode));         items[i++] = strdup(buf);
    items[i++] = strdup("Open all levels");
    snprintf(buf, sizeof buf, "Can select level: %s", QE_STR_BOOL(Main_canSelectLevel));   items[i++] = strdup(buf);
    snprintf(buf, sizeof buf, "Show QFPS: %s", QE_STR_BOOL(DeveloperMenu_showFps));        items[i++] = strdup(buf);
    snprintf(buf, sizeof buf, "Show RAM: %s", QE_STR_BOOL(DeveloperMenu_showRam));         items[i++] = strdup(buf);
    snprintf(buf, sizeof buf, "Show room ID: %s", QE_STR_BOOL(DeveloperMenu_showRoomID));  items[i++] = strdup(buf);
    snprintf(buf, sizeof buf, "Draw portals: %s", QE_STR_BOOL(DeveloperMenu_drawPortals)); items[i++] = strdup(buf);
    snprintf(buf, sizeof buf, "Show player pos: %s", QE_STR_BOOL(DeveloperMenu_showPlayerPos)); items[i++] = strdup(buf);
    snprintf(buf, sizeof buf, "God mode: %s", QE_STR_BOOL(DeveloperMenu_godMode));         items[i++] = strdup(buf);
    snprintf(buf, sizeof buf, "Show level objects: %s", QE_STR_BOOL(DeveloperMenu_drawLevelPoints)); items[i++] = strdup(buf);
    snprintf(buf, sizeof buf, "Show shoot collision: %s", QE_STR_BOOL(DeveloperMenu_showShootCollision)); items[i++] = strdup(buf);
    snprintf(buf, sizeof buf, "Render Pixels Overwrite: %s", QE_STR_BOOL(DeveloperMenu_renderPolygonsOverwrite)); items[i++] = strdup(buf);
    snprintf(buf, sizeof buf, "Fly: %s", DeveloperMenu_fly == 0 ? "Off" : DeveloperMenu_fly == 1 ? "Fly" : "Noclip"); items[i++] = strdup(buf);
    items[i++] = strdup("Play video!!! :D");
}

static void DeveloperMenu_paint(MyCanvas *mc, Graphics *g) {
    DeveloperMenu *self = (DeveloperMenu*) mc;
    Graphics_setColor(g, 0);
    Graphics_fillRect(g, 0, 0, MyCanvas_getWidth(mc), MyCanvas_getHeight(mc));
    /* background delegado ao caller */
    if (is_pause_screen_(self->menu)) {
        ItemList_drawBck(self->base.list, g, MyCanvas_getWidth(mc) / 8, Font_height(Main_getFont()),
                         MyCanvas_getWidth(mc) * 6 / 8, MyCanvas_getHeight(mc) - Font_height(Main_getFont()) * 2);
        Main_drawBckDialog(g, MyCanvas_getHeight(mc) - Font_height(Main_getFont()), MyCanvas_getHeight(mc));
    }
    ItemList_draw(self->base.list, g, MyCanvas_getWidth(mc) / 8, Font_height(Main_getFont()),
                  MyCanvas_getWidth(mc) * 6 / 8, MyCanvas_getHeight(mc) - Font_height(Main_getFont()) * 2);
    GUIScreen_drawSoftKeys(&self->base.base, g);
}

void DeveloperMenu_setFly(DeveloperMenu *self, int fl) {
    DeveloperMenu_fly = fl;
    if (is_pause_screen_(self->menu)) {
        extern void  PauseScreen_setPlayerFly(PauseScreen *ps, int fly);
        PauseScreen_setPlayerFly((PauseScreen*) self->menu, fl > 0);
    }
}

static void adjust_dm(DeveloperMenu *self, int dir) {
    int var1 = Selectable_itemIndex(&self->base);
    int id = 0;
    id++; if (var1 == id - 1) {
        DeveloperMenu_debugMode = dir > 0;
        if (is_menu_(self->menu))         Menu_reloadText((Menu*) self->menu);
        if (is_pause_screen_(self->menu)) PauseScreen_reloadText((PauseScreen*) self->menu);
    }
    id++;
    id++; if (var1 == id - 1) { Main_canSelectLevel = dir > 0; if (is_menu_(self->menu)) Menu_reloadText((Menu*) self->menu); }
    id++; if (var1 == id - 1) DeveloperMenu_showFps                = dir > 0;
    id++; if (var1 == id - 1) DeveloperMenu_showRam                = dir > 0;
    id++; if (var1 == id - 1) DeveloperMenu_showRoomID             = dir > 0;
    id++; if (var1 == id - 1) DeveloperMenu_drawPortals            = dir > 0;
    id++; if (var1 == id - 1) DeveloperMenu_showPlayerPos          = dir > 0;
    id++; if (var1 == id - 1) DeveloperMenu_godMode                = dir > 0;
    id++; if (var1 == id - 1) DeveloperMenu_drawLevelPoints        = dir > 0;
    id++; if (var1 == id - 1) DeveloperMenu_showShootCollision     = dir > 0;
    id++; if (var1 == id - 1) DeveloperMenu_renderPolygonsOverwrite= dir > 0;
    id++; if (var1 == id - 1) DeveloperMenu_setFly(self,
        dir > 0 ? (DeveloperMenu_fly==0?1:DeveloperMenu_fly==1?2:0) : (DeveloperMenu_fly==0?2:DeveloperMenu_fly==1?0:1));
    setItems(self);
    MyCanvas_repaint(&self->base.base.base);
}
static void DeveloperMenu_onKey4(GUIScreen *gs) { adjust_dm((DeveloperMenu*) gs, -1); }
static void DeveloperMenu_onKey6(GUIScreen *gs) { adjust_dm((DeveloperMenu*) gs, +1); }
static void DeveloperMenu_onKey5(GUIScreen *gs) {
    DeveloperMenu *self = (DeveloperMenu*) gs;
    int var1 = Selectable_itemIndex(&self->base);
    int id = 0;
    int repaint = 1;
    id++; if (var1 == id - 1) {
        DeveloperMenu_debugMode = !DeveloperMenu_debugMode;
        if (is_menu_(self->menu))         Menu_reloadText((Menu*) self->menu);
        if (is_pause_screen_(self->menu)) PauseScreen_reloadText((PauseScreen*) self->menu);
    }
    id++; if (var1 == id - 1) { Main_setAvailableLevelCount(Main_lastLevel); if (is_menu_(self->menu)) Menu_reloadText((Menu*) self->menu); }
    id++; if (var1 == id - 1) { Main_canSelectLevel ^= 1; if (is_menu_(self->menu)) Menu_reloadText((Menu*) self->menu); }
    id++; if (var1 == id - 1) DeveloperMenu_showFps                ^= 1;
    id++; if (var1 == id - 1) DeveloperMenu_showRam                ^= 1;
    id++; if (var1 == id - 1) DeveloperMenu_showRoomID             ^= 1;
    id++; if (var1 == id - 1) DeveloperMenu_drawPortals            ^= 1;
    id++; if (var1 == id - 1) DeveloperMenu_showPlayerPos          ^= 1;
    id++; if (var1 == id - 1) DeveloperMenu_godMode                ^= 1;
    id++; if (var1 == id - 1) DeveloperMenu_drawLevelPoints        ^= 1;
    id++; if (var1 == id - 1) DeveloperMenu_showShootCollision     ^= 1;
    id++; if (var1 == id - 1) DeveloperMenu_renderPolygonsOverwrite^= 1;
    id++; if (var1 == id - 1) DeveloperMenu_setFly(self, DeveloperMenu_fly==0?1:DeveloperMenu_fly==1?2:0);
    id++; if (var1 == id - 1) { repaint = 0; VideoPlayer_new(self->main, "/video.3gp", (MyCanvas*) self); }
    setItems(self);
    if (repaint) MyCanvas_repaint(&gs->base);
}
static void DeveloperMenu_onLeftSoftKey(GUIScreen *gs) { DeveloperMenu_onKey6(gs); }
static void DeveloperMenu_onRightSoftKey(GUIScreen *gs) {
    DeveloperMenu *self = (DeveloperMenu*) gs;
    Main_setCurrent(self->menu);
}

static const GUIScreenVTable g_vt = {
    {
        DeveloperMenu_paint,
        GUIScreen_default_keyPressed, GUIScreen_default_keyRepeated, GUIScreen_default_keyReleased,
        GUIScreen_default_pointerPressed, MyCanvas_default_mouseScrollDown, MyCanvas_default_mouseScrollUp,
        MyCanvas_default_pointerReleased, GUIScreen_default_pointerDragged, MyCanvas_default_pointerClicked,
        MyCanvas_default_sizeChanged, MyCanvas_default_showNotify, MyCanvas_default_hideNotify
    },
    DeveloperMenu_onLeftSoftKey, DeveloperMenu_onRightSoftKey,
    DeveloperMenu_onKey5, DeveloperMenu_onKey4, DeveloperMenu_onKey6,
    Selectable_onKey2, Selectable_onKey8,
    NULL, NULL, NULL, NULL, NULL, NULL
};

DeveloperMenu *DeveloperMenu_new(Main *main, MyCanvas *menu, void *background) {
    DeveloperMenu *d = (DeveloperMenu*) calloc(1, sizeof(DeveloperMenu));
    GUIScreen_init(&d->base.base, &g_vt);
    d->main = main; d->menu = menu; d->background = background;
    IniFile *lang = Main_getGameText();
    char **items = (char**) calloc(14, sizeof(char*));
    Selectable_set(&d->base, Main_getFont(), items, 14, NULL, IniFile_get(lang, "BACK"));
    setItems(d);
    d->backgroundH = MyCanvas_getHeight(&d->base.base.base) / 2 - MyCanvas_getHeight(&d->base.base.base) * Main_getDisplaySize() / 200;
    d->h           = MyCanvas_getHeight(&d->base.base.base) * Main_getDisplaySize() / 100;
    return d;
}
void DeveloperMenu_free(DeveloperMenu *d) { if (!d) return; Selectable_destroy(&d->base); free(d); }
