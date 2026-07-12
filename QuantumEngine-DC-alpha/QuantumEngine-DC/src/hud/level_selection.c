/*
 * level_selection.c — porte fiel de code/HUD/LevelSelection.java
 */
#include "level_selection.h"
#include "menu.h"
#include "base/font.h"
#include "base/item_list.h"
#include "../utils/main.h"
#include "../utils/inifile.h"
#include "../utils/canvas/main_canvas.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void LevelSelection_paint(MyCanvas *mc, Graphics *g) {
    LevelSelection *self = (LevelSelection*) mc;
    Menu_drawBackground(self->menu, g);
    Selectable_paint(mc, g);
}
static void LevelSelection_onRightSoftKey(GUIScreen *gs) {
    LevelSelection *self = (LevelSelection*) gs;
    Main_setCurrent((MyCanvas*) self->menu);
}
static void LevelSelection_onKey5(GUIScreen *gs) {
    LevelSelection *self = (LevelSelection*) gs;
    int levelNumber = Selectable_itemIndex(&self->base) + 1;
    Selectable_destroy(&self->base);
    Main_loadLevel(Main_levelSelectorLoadData, 0, levelNumber, NULL, self->menu, 1, 1);
}
static void LevelSelection_onLeftSoftKey(GUIScreen *gs) { LevelSelection_onKey5(gs); }

static const GUIScreenVTable g_vt = {
    {
        LevelSelection_paint,
        GUIScreen_default_keyPressed, GUIScreen_default_keyRepeated, GUIScreen_default_keyReleased,
        GUIScreen_default_pointerPressed, MyCanvas_default_mouseScrollDown, MyCanvas_default_mouseScrollUp,
        MyCanvas_default_pointerReleased, GUIScreen_default_pointerDragged, MyCanvas_default_pointerClicked,
        MyCanvas_default_sizeChanged, MyCanvas_default_showNotify, MyCanvas_default_hideNotify
    },
    LevelSelection_onLeftSoftKey, LevelSelection_onRightSoftKey,
    LevelSelection_onKey5, NULL, NULL,
    Selectable_onKey2, Selectable_onKey8,
    NULL, NULL, NULL, NULL, NULL, NULL
};

LevelSelection *LevelSelection_new(Main *main, Menu *menu) {
    LevelSelection *ls = (LevelSelection*) calloc(1, sizeof(LevelSelection));
    GUIScreen_init(&ls->base.base, &g_vt);
    ls->main = main; ls->menu = menu;

    IniFile *g = Main_getGameText();
    const char *lv = IniFile_get(g, "LEVEL");
    int n = Main_getAvailableLevelCount();
    char **items = (char**) calloc(n, sizeof(char*));
    for (int i = 0; i < n; i++) {
        char key[32]; snprintf(key, sizeof key, "LEVEL_%d", i + 1);
        const char *nm = IniFile_get(g, key);
        char buf[128];
        char prefix[16] = "";
        if (Main_levelCounter) snprintf(prefix, sizeof prefix, "%d.", i + 1);
        if (nm) snprintf(buf, sizeof buf, "%s%s", prefix, nm);
        else    snprintf(buf, sizeof buf, "%s %d", lv, i + 1);
        items[i] = strdup(buf);
    }
    Selectable_set(&ls->base, Main_getFont(), items, n, IniFile_get(g, "SELECT"), IniFile_get(g, "BACK"));
    if (Menu_hasSave) Selectable_setItemIndex(&ls->base, Main_getContinueLevel() - 1);
    return ls;
}
LevelSelection *LevelSelection_new_hud(Main *main, Menu *menu, void *hudInfo) { (void)hudInfo; return LevelSelection_new(main, menu); }
void LevelSelection_free(LevelSelection *ls) { if (!ls) return; Selectable_destroy(&ls->base); free(ls); }
