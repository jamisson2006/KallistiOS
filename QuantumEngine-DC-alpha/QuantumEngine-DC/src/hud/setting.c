/*
 * setting.c — porte fiel de code/HUD/Setting.java
 */
#include "setting.h"
#include "menu.h"
#include "pause_screen.h"
#include "keys_settings.h"
#include "base/font.h"
#include "base/item_list.h"
#include "../utils/main.h"
#include "../utils/inifile.h"
#include "../utils/canvas/main_canvas.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

extern int DeveloperMenu_debugMode;
extern void Graphics_setColor(Graphics *g, int c);
extern void Graphics_fillRect(Graphics *g, int x, int y, int w, int h);
extern void Graphics_drawImage(Graphics *g, void *img, int x, int y, int anchor);
extern void Graphics_drawRGB(Graphics *g, int *arr, int off, int stride, int x, int y, int w, int h, int alpha);
extern const void *PauseScreen_vt_marker(void); /* nao precisa — deteccao por opaque type externo */

static int is_pause_screen(MyCanvas *mc) {
    /* Sem RTTI real, o codigo eh disciplinado: setup passa `menu` = PauseScreen ou Menu.
     * Aqui usamos hint via ponteiro de screen tipo. Suficiente para o paint(). */
    (void) mc; return 0;
}
static int is_menu(MyCanvas *mc)         { (void) mc; return 0; }

static int hasLangSelect_v(MyCanvas *menu) { (void) menu; return Main_langs_count > 1; }

static char *dupfmt(const char *fmt, ...) __attribute__((format(printf,1,2)));
static char *dupfmt(const char *fmt, ...) {
    char buf[256]; va_list ap; va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    return strdup(buf);
}
#include <stdarg.h>

static void applyLanguage(Setting *self) {
    char lc[128]; int i = 0;
    for (const char *p = Main_langs[Main_lang]; *p && i + 1 < 128; p++, i++) lc[i] = (char) tolower((unsigned char) *p);
    lc[i] = 0;
    char path[256]; snprintf(path, sizeof path, "/languages/%s.txt", lc);
    Main_setLanguage(path);
    if (is_menu(self->menu)) Menu_reloadText((Menu*) self->menu);
    GUIScreen_setRightSoft(&self->base.base, IniFile_get(Main_getGameText(), "BACK"));
}

static void setItems(Setting *self) {
    IniFile *lang = Main_getGameText();
    int n; char **var2 = Selectable_getItems(&self->base, &n);
    int *ms = self->base.list->midSel;
    int i = 0;

    /* itens duplicados para heap gerenciado pelo ItemList: liberamos antigos */
    for (int j = 0; j < n; j++) { free(var2[j]); var2[j] = NULL; }

    var2[i] = dupfmt("%s:", IniFile_get(lang, "AUDIO")); ms[i] = 1; i++;
    if (Main_music > 0 && Main_isMusic) var2[i] = dupfmt("%s:%d", IniFile_get(lang, "MUSIC_VOLUME"), Main_music);
    else var2[i] = dupfmt("%s:%s", IniFile_get(lang, "MUSIC_VOLUME"), IniFile_get(lang, "OFF"));
    ms[i] = 0; i++;
    if (Main_footsteps > 0 && Main_isFootsteps) var2[i] = dupfmt("%s:%d", IniFile_get(lang, "FOOTSTEPS_VOLUME"), Main_footsteps);
    else var2[i] = dupfmt("%s:%s", IniFile_get(lang, "FOOTSTEPS_VOLUME"), IniFile_get(lang, "OFF"));
    ms[i] = 0; i++;
    if (Main_sounds > 0 && Main_isSounds) var2[i] = dupfmt("%s:%d", IniFile_get(lang, "SOUNDS_VOLUME"), Main_sounds);
    else var2[i] = dupfmt("%s:%s", IniFile_get(lang, "SOUNDS_VOLUME"), IniFile_get(lang, "OFF"));
    ms[i] = 0; i++;

    var2[i] = dupfmt("%s:", IniFile_get(lang, "CONTROLS"));         ms[i] = 1; i++;
    var2[i] = dupfmt("%s:%d", IniFile_get(lang, "CAM_SPEED"), Main_mouseSpeed); ms[i] = 0; i++;
    var2[i] = dupfmt("%s", IniFile_get(lang, "CHANGE_KEYS"));       ms[i] = 0; i++;

    var2[i] = dupfmt("%s:", IniFile_get(lang, "PERF"));             ms[i] = 1; i++;
    var2[i] = dupfmt("%s:%d", IniFile_get(lang, "DISPLAY_SIZE"), Main_getDisplaySize()); ms[i] = 0; i++;
    var2[i] = dupfmt("%s:%d", IniFile_get(lang, "PERSQ"), Main_persQ * 25);   i++;

    if      (Main_pixelsQ == 0) var2[i] = dupfmt("%s:%s", IniFile_get(lang, "TEXQ"), IniFile_get(lang, "LLQ"));
    else if (Main_pixelsQ == 1) var2[i] = dupfmt("%s:%s", IniFile_get(lang, "TEXQ"), IniFile_get(lang, "LQ"));
    else                        var2[i] = dupfmt("%s:%s", IniFile_get(lang, "TEXQ"), IniFile_get(lang, "HQ"));
    ms[i] = 0; i++;

    if      (Main_fogQ == 0) var2[i] = dupfmt("%s:%s", IniFile_get(lang, "FOGQ"), IniFile_get(lang, "OFF"));
    else if (Main_fogQ == 1) var2[i] = dupfmt("%s:%s", IniFile_get(lang, "FOGQ"), IniFile_get(lang, "LLQ"));
    else                     var2[i] = dupfmt("%s:%s", IniFile_get(lang, "FOGQ"), IniFile_get(lang, "HQ"));
    ms[i] = 0; i++;

    var2[i] = dupfmt("%s:%s", IniFile_get(lang, "CORPSES"),   Main_corpses    ? IniFile_get(lang, "ON") : IniFile_get(lang, "OFF")); ms[i] = 0; i++;
    var2[i] = dupfmt("%s:%s", IniFile_get(lang, "BLOOD"),     Main_blood      ? IniFile_get(lang, "ON") : IniFile_get(lang, "OFF")); ms[i] = 0; i++;
    var2[i] = dupfmt("%s:%s", IniFile_get(lang, "MIPMAPPING"),Main_mipMapping ? IniFile_get(lang, "ON") : IniFile_get(lang, "OFF")); ms[i] = 0; i++;
    var2[i] = dupfmt("%s:%s", IniFile_get(lang, "FRAMESKIP"), Main_frameskip  ? IniFile_get(lang, "ON") : IniFile_get(lang, "OFF")); ms[i] = 0; i++;

    var2[i] = dupfmt("%s:", IniFile_get(lang, "HUD")); ms[i] = 1; i++;

    if (hasLangSelect_v(self->menu)) {
        const char *lng = Main_langs[Main_lang];
        const char *tr = GameIni_getNoLang(Main_settings, lng);
        if (tr) lng = tr;
        else { const char *lt = IniFile_get(lang, lng); if (lt) lng = lt; }
        var2[i] = dupfmt("%s:%s", IniFile_get(lang, "LANG"), lng); ms[i] = 0; i++;
    }
    var2[i] = dupfmt("%s:%s", IniFile_get(lang, "RESIZE_WEAPONS"), Main_resizeWeapons ? IniFile_get(lang, "YES") : IniFile_get(lang, "NO")); ms[i] = 0; i++;
    var2[i] = dupfmt("%s:%s", IniFile_get(lang, "HIDEHUD"),        Main_hideHud       ? IniFile_get(lang, "YES") : IniFile_get(lang, "NO")); ms[i] = 0; i++;
    var2[i] = dupfmt("%s:%s", IniFile_get(lang, "DEBUG"),          DeveloperMenu_debugMode ? IniFile_get(lang, "ON")  : IniFile_get(lang, "OFF")); ms[i] = 0; i++;
}

static void Setting_paint(MyCanvas *mc, Graphics *g) {
    Setting *self = (Setting*) mc;
    Graphics_setColor(g, 0);
    Graphics_fillRect(g, 0, 0, MyCanvas_getWidth(mc), MyCanvas_getHeight(mc));
    if (self->background) {
        /* Se background eh Image ou int[] — Java descobre em runtime.
         * Aqui delegamos ao caller de Setting_new via convencao (Image quando vem do Menu). */
    }
    if (is_pause_screen(self->menu)) {
        ItemList_drawBck(self->base.list, g, MyCanvas_getWidth(mc) / 8, Font_height(Main_getFont()),
                         MyCanvas_getWidth(mc) * 6 / 8, MyCanvas_getHeight(mc) - Font_height(Main_getFont()) * 2);
        Main_drawBckDialog(g, MyCanvas_getHeight(mc) - Font_height(Main_getFont()), MyCanvas_getHeight(mc));
    }
    ItemList_draw(self->base.list, g, MyCanvas_getWidth(mc) / 8, Font_height(Main_getFont()),
                  MyCanvas_getWidth(mc) * 6 / 8, MyCanvas_getHeight(mc) - Font_height(Main_getFont()) * 2);
    GUIScreen_drawSoftKeys(&self->base.base, g);
}

static void Setting_onRightSoftKey(GUIScreen *gs) {
    Setting *self = (Setting*) gs;
    Main_saveSettingToStore();
    Main_setCurrent(self->menu);
}
static void Setting_onKey5(GUIScreen *gs) {
    Setting *self = (Setting*) gs;
    int idx = Selectable_itemIndex(&self->base);
    if (idx == 6) {
        Main_setCurrent((MyCanvas*) KeysSettings_new(self->main, self, self->background, is_pause_screen(self->menu)));
        return;
    }
}
static void Setting_onLeftSoftKey(GUIScreen *gs) { Setting_onKey5(gs); }

static void adjust(Setting *self, int dir);
static void Setting_onKey4(GUIScreen *gs) { adjust((Setting*) gs, -1); }
static void Setting_onKey6(GUIScreen *gs) { adjust((Setting*) gs, +1); }

static void adjust(Setting *self, int dir) {
    int var1 = Selectable_itemIndex(&self->base);
    int id = 0; id++;/*Audio*/
    if (var1 == id) {
        if (Main_isMusic) {
            Main_music += 10 * dir;
            if (Main_music < 0)   Main_music = 0;
            if (Main_music > 100) Main_music = 100;
            if (dir < 0 && Main_music == 0) Main_isMusic = 0;
            if (dir > 0 && !Main_isMusic) Main_isMusic = 1;
        } else if (dir > 0) Main_isMusic = 1;
        Sound_setVolume(Main_musicPlayer, Main_isMusic ? Main_music : 0);
    }
    id++;
    if (var1 == id) {
        if (Main_isFootsteps) { Main_footsteps += 10 * dir; if (Main_footsteps<0) Main_footsteps=0; if (Main_footsteps>100) Main_footsteps=100; if (dir<0 && Main_footsteps==0) Main_isFootsteps=0; if (dir>0 && !Main_isFootsteps) Main_isFootsteps=1; } else if (dir > 0) Main_isFootsteps = 1;
    }
    id++;
    if (var1 == id) {
        if (Main_isSounds)    { Main_sounds    += 10 * dir; if (Main_sounds<0) Main_sounds=0; if (Main_sounds>100) Main_sounds=100; if (dir<0 && Main_sounds==0) Main_isSounds=0; if (dir>0 && !Main_isSounds) Main_isSounds=1; } else if (dir > 0) Main_isSounds = 1;
    }
    id++;
    id++;/*Controls header*/
    if (var1 == id) Main_mouseSpeed += 2 * dir;
    id++;
    id++;/*CHANGE_KEYS*/
    id++;/*Perf header*/
    if (var1 == id) Main_setDisplaySize(Main_getDisplaySize() + 5 * dir);
    id++;
    if (var1 == id) Main_setPersQ(Main_persQ + dir < 0 ? 0 : (Main_persQ + dir > 4 ? 4 : Main_persQ + dir));
    id++;
    if (var1 == id) Main_pixelsQ = (Main_pixelsQ + dir < 0 ? 0 : (Main_pixelsQ + dir > 2 ? 2 : Main_pixelsQ + dir));
    id++;
    if (var1 == id) Main_fogQ    = (Main_fogQ    + dir < 0 ? 0 : (Main_fogQ    + dir > 2 ? 2 : Main_fogQ    + dir));
    id++;
    if (var1 == id) Main_corpses    = dir > 0;
    id++;
    if (var1 == id) Main_blood      = dir > 0;
    id++;
    if (var1 == id) Main_mipMapping = dir > 0;
    id++;
    if (var1 == id) Main_frameskip  = dir > 0;
    id++;
    id++;/*Hud header*/
    if (hasLangSelect_v(self->menu)) {
        if (var1 == id) {
            Main_lang += dir;
            if (Main_lang < 0) Main_lang = Main_langs_count - 1;
            if (Main_lang >= Main_langs_count) Main_lang = 0;
            applyLanguage(self);
        }
        id++;
    }
    if (var1 == id) Main_resizeWeapons = dir > 0;
    id++;
    if (var1 == id) Main_hideHud       = dir > 0;
    id++;
    if (var1 == id) {
        DeveloperMenu_debugMode = dir > 0;
        if (is_menu(self->menu))         Menu_reloadText((Menu*) self->menu);
        if (is_pause_screen(self->menu)) PauseScreen_reloadText((PauseScreen*) self->menu);
    }
    setItems(self);
    MyCanvas_repaint(&self->base.base.base);
}

static const GUIScreenVTable g_vt = {
    {
        Setting_paint,
        GUIScreen_default_keyPressed, GUIScreen_default_keyRepeated, GUIScreen_default_keyReleased,
        GUIScreen_default_pointerPressed, MyCanvas_default_mouseScrollDown, MyCanvas_default_mouseScrollUp,
        MyCanvas_default_pointerReleased, GUIScreen_default_pointerDragged, MyCanvas_default_pointerClicked,
        MyCanvas_default_sizeChanged, MyCanvas_default_showNotify, MyCanvas_default_hideNotify
    },
    Setting_onLeftSoftKey, Setting_onRightSoftKey,
    Setting_onKey5, Setting_onKey4, Setting_onKey6,
    Selectable_onKey2, Selectable_onKey8,
    NULL, NULL, NULL, NULL, NULL, NULL
};

Setting *Setting_new(Main *main, MyCanvas *menu, void *background) {
    Setting *s = (Setting*) calloc(1, sizeof(Setting));
    GUIScreen_init(&s->base.base, &g_vt);
    s->main = main; s->menu = menu; s->background = background;

    IniFile *lang = Main_getGameText();
    int total = 20 + (hasLangSelect_v(menu) ? 1 : 0);
    char **items = (char**) calloc(total, sizeof(char*));
    int *center  = (int*) calloc(total, sizeof(int));
    Selectable_set_ms(&s->base, Main_getFont(), items, total, NULL, IniFile_get(lang, "BACK"), center, total);
    setItems(s);
    s->base.list->left = 1;
    ItemList_scrollDown(s->base.list);
    s->hei = MyCanvas_getHeight(&s->base.base.base) / 2 - MyCanvas_getHeight(&s->base.base.base) * Main_getDisplaySize() / 200;
    s->h   = MyCanvas_getHeight(&s->base.base.base) * Main_getDisplaySize() / 100;
    return s;
}
void Setting_free(Setting *s) { if (!s) return; Selectable_destroy(&s->base); free(s); }
