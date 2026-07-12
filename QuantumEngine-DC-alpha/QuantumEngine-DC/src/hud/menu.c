/*
 * menu.c — porte fiel de code/HUD/Menu.java
 */
#include "menu.h"
#include "help.h"
#include "setting.h"
#include "level_selection.h"
#include "benchmark.h"
#include "developer_menu.h"
#include "base/font.h"
#include "base/item_list.h"
#include "../utils/main.h"
#include "../utils/image_resize.h"
#include "../utils/inifile.h"
#include "../utils/canvas/main_canvas.h"

#include <stdlib.h>
#include <string.h>

/* extern helpers */
extern Image *Image_createImage(const char *file);
extern Image *ImageResize_createImage_wh(Image *img, int w, int h);
extern void   Graphics_drawImage(Graphics *g, Image *img, int x, int y, int anchor);
extern void   Main_notifyDestroyed(Main *m);
extern void   Main_platformRequest(Main *m, const char *url);

int Menu_w = 240;
int Menu_h = 320;
int Menu_hasSave = 0;

static void Menu_paint(MyCanvas *mc, Graphics *g);
static void Menu_onKey5(GUIScreen *gs);
static void Menu_onLeftSoftKey(GUIScreen *gs) { Menu_onKey5(gs); }

static const GUIScreenVTable g_vt = {
    {
        Menu_paint,
        GUIScreen_default_keyPressed,
        GUIScreen_default_keyRepeated,
        GUIScreen_default_keyReleased,
        GUIScreen_default_pointerPressed,
        MyCanvas_default_mouseScrollDown,
        MyCanvas_default_mouseScrollUp,
        MyCanvas_default_pointerReleased,
        GUIScreen_default_pointerDragged,
        MyCanvas_default_pointerClicked,
        MyCanvas_default_sizeChanged,
        MyCanvas_default_showNotify,
        MyCanvas_default_hideNotify
    },
    Menu_onLeftSoftKey, NULL, Menu_onKey5,
    NULL, NULL,
    Selectable_onKey2, Selectable_onKey8,
    NULL, NULL, NULL, NULL, NULL, NULL
};

Menu *Menu_new(Main *main) {
    Menu *m = (Menu*) calloc(1, sizeof(Menu));
    GUIScreen_init(&m->base.base, &g_vt);
    Menu_hasSave = Main_hasSave();
    m->main = main;
    Menu_w = MyCanvas_getWidth(&m->base.base.base);
    Menu_h = MyCanvas_getHeight(&m->base.base.base);

    m->background     = ImageResize_createImage_wh(Image_createImage(Main_background), Menu_w, Menu_h);
    m->backgroundMain = m->background;
    if (Main_background_logo != NULL && strcmp(Main_background_logo, Main_background) != 0) {
        m->backgroundMain = ImageResize_createImage_wh(Image_createImage(Main_background_logo), Menu_w, Menu_h);
    }

    if (Main_isMusic && Main_menuMusic != NULL) {
        Sound_loadFile(Main_musicPlayer, Main_menuMusic);
        Sound_setVolume(Main_musicPlayer, Main_music);
        Sound_start(Main_musicPlayer);
    }

    Menu_reloadText(m);
    return m;
}

void Menu_reloadText(Menu *self) {
    self->newGameIndex = self->continueIndex = self->levelSelectIndex =
        self->helpIndex = self->optionsIndex = self->exitIndex = -1;

    IniFile *lang = Main_getGameText();
    int cap = 16, cnt = 0;
    char **items = (char**) malloc(sizeof(char*) * cap);
    #define QE_ADD(s) do { if (cnt >= cap) { cap *= 2; items = (char**) realloc(items, sizeof(char*) * cap); } items[cnt++] = strdup(s); } while (0)

    int levels = Main_getAvailableLevelCount();
    if (!Menu_hasSave) {
        if (levels > 1 && Main_canSelectLevel) { QE_ADD(IniFile_get(lang, "SELECT_LEVEL")); self->levelSelectIndex = 0; }
        else { QE_ADD(IniFile_get(lang, "NEW_GAME")); self->newGameIndex = 0; }
    } else {
        QE_ADD(IniFile_get(lang, "CONTINUE")); self->continueIndex = 0;
        self->newGameIndex = 1;
        if (levels > 1 && Main_canSelectLevel) {
            QE_ADD(IniFile_get(lang, "SELECT_LEVEL"));
            self->levelSelectIndex = 1;
            self->newGameIndex = 2;
        }
        QE_ADD(IniFile_get(lang, "NEW_GAME"));
    }
    QE_ADD(IniFile_get(lang, "HELP"));    self->helpIndex    = cnt - 1;
    QE_ADD(IniFile_get(lang, "OPTIONS")); self->optionsIndex = cnt - 1;
    QE_ADD(IniFile_get(lang, "EXIT"));    self->exitIndex    = cnt - 1;

    extern int DeveloperMenu_debugMode;
    if (DeveloperMenu_debugMode) {
        QE_ADD("Benchmark"); QE_ADD("AE-Mods"); QE_ADD("Developer menu");
    }

    Selectable_set(&self->base, Main_getFont(), items, cnt, IniFile_get(lang, "SELECT"), NULL);
    #undef QE_ADD
}

void Menu_destroy(Menu *self) {
    Selectable_destroy(&self->base);
    Sound_destroy(Main_musicPlayer);
    self->background = self->backgroundMain = NULL;
}

void Menu_free(Menu *self) {
    Menu_destroy(self);
    free(self);
}

static void Menu_paint(MyCanvas *mc, Graphics *g) {
    Menu *self = (Menu*) mc;
    Menu_drawBackgroundLogo(self, g);
    Selectable_paint(mc, g);
}

void Menu_drawBackgroundLogo(Menu *self, Graphics *g) { Graphics_drawImage(g, self->backgroundMain, 0, 0, 0); }
void Menu_drawBackground    (Menu *self, Graphics *g) { Graphics_drawImage(g, self->background,     0, 0, 0); }

static void Menu_onKey5(GUIScreen *gs) {
    Menu *self = (Menu*) gs;
    int index = Selectable_itemIndex(&self->base);
    int items_n; char **items = Selectable_getItems(&self->base, &items_n);
    extern int DeveloperMenu_debugMode;

    if      (index == self->newGameIndex)      { Main_removeSave(); Main_loadLevel(0, 0, 1, NULL, self, 1, 1); }
    else if (index == self->continueIndex)     { int ln = Main_getContinueLevel(); Main_loadLevel(1, 1, ln, NULL, self, 1, 1); }
    else if (index == self->levelSelectIndex)  { Main_setCurrent((MyCanvas*) LevelSelection_new(self->main, self)); }
    else if (index == self->helpIndex)         { Main_setCurrent((MyCanvas*) Help_new(self->main, self)); }
    else if (index == self->optionsIndex)      { Main_setCurrent((MyCanvas*) Setting_new(self->main, self, self->background)); }
    else if (index == self->exitIndex)         { Main_notifyDestroyed(self->main); }
    else if (index == items_n - 3 && DeveloperMenu_debugMode) { Main_setCurrent((MyCanvas*) Benchmark_new(self->main, self)); }
    else if (index == items_n - 2 && DeveloperMenu_debugMode) { Main_platformRequest(self->main, "http://ae-mods.ru"); }
    else if (index == items_n - 1 && DeveloperMenu_debugMode) { Main_setCurrent((MyCanvas*) DeveloperMenu_new(self->main, self, self->background)); }

    (void) items;
    MyCanvas_repaint(&gs->base);
}
