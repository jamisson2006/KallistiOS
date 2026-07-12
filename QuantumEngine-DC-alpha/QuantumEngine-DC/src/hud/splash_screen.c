/*
 * splash_screen.c — porte fiel de code/HUD/SplashScreen.java
 */
#include "splash_screen.h"
#include "menu.h"
#include "base/font.h"
#include "base/item_list.h"
#include "../utils/main.h"
#include "../utils/image_resize.h"
#include "../utils/inifile.h"
#include "../utils/canvas/main_canvas.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _arch_dreamcast
#include <arch/timer.h>
static inline long long qe_now_ms(void) { return (long long) timer_ms_gettime64(); }
#else
#include <sys/time.h>
static inline long long qe_now_ms(void) {
    struct timeval tv; gettimeofday(&tv,0);
    return (long long) tv.tv_sec * 1000LL + tv.tv_usec / 1000;
}
#endif

extern Image *Image_createImage(const char *file);
extern Image *ImageResize_createImage_wh(Image *img, int w, int h);
extern Image *ImageResize_createImage_sc(Image *img, float sx, float sy);
extern int    Image_getHeight(Image *img);
extern int    Image_getWidth (Image *img);
extern void   Graphics_setColor(Graphics *g, int c);
extern void   Graphics_fillRect(Graphics *g, int x, int y, int w, int h);
extern void   Graphics_drawImage(Graphics *g, Image *img, int x, int y, int anchor);

static void SplashScreen_paint(MyCanvas *mc, Graphics *g);
static void SplashScreen_onKey2(GUIScreen *gs);
static void SplashScreen_onKey8(GUIScreen *gs);
static void SplashScreen_onKey5(GUIScreen *gs);
static void SplashScreen_onLeftSoftKey(GUIScreen *gs);
static void SplashScreen_sizeChanged(MyCanvas *mc, int w, int h);
static void SplashScreen_destroy_v(MyCanvas *mc) { SplashScreen_free((SplashScreen*) mc); }

static const GUIScreenVTable g_vt = {
    /* canvas: */ {
        SplashScreen_paint,
        GUIScreen_default_keyPressed,
        GUIScreen_default_keyRepeated,
        GUIScreen_default_keyReleased,
        GUIScreen_default_pointerPressed,
        MyCanvas_default_mouseScrollDown,
        MyCanvas_default_mouseScrollUp,
        MyCanvas_default_pointerReleased,
        GUIScreen_default_pointerDragged,
        MyCanvas_default_pointerClicked,
        SplashScreen_sizeChanged,
        MyCanvas_default_showNotify,
        MyCanvas_default_hideNotify
    },
    SplashScreen_onLeftSoftKey,
    NULL,
    SplashScreen_onKey5,
    NULL, NULL,
    SplashScreen_onKey2,
    SplashScreen_onKey8,
    NULL, NULL, NULL, NULL, NULL, NULL
};

static void onAction(SplashScreen *self, int action);

SplashScreen *SplashScreen_new(Main *main) {
    SplashScreen *s = (SplashScreen*) calloc(1, sizeof(SplashScreen));
    GUIScreen_init(&s->base, &g_vt);
    s->main = main;
    s->splashBeginTime = -1;
    GUIScreen_setFont(&s->base, Main_getFont());
    onAction(s, 1);

    if (Main_bcks  != NULL) Main_bcks  = ImageResize_createImage_wh(Main_bcks,  MyCanvas_getWidth(&s->base.base), Image_getHeight(Main_bcks));
    if (Main_bcks2 != NULL) Main_bcks2 = ImageResize_createImage_wh(Main_bcks2, MyCanvas_getWidth(&s->base.base), Image_getHeight(Main_bcks2));
    return s;
}

void SplashScreen_free(SplashScreen *s) {
    if (!s) return;
    GUIScreen_destroy(&s->base);
    free(s->splash);
    if (s->list) ItemList_free(s->list);
    free(s);
}

static void onAction(SplashScreen *self, int action) {
    self->action = action;

    if (action == 1) {
        if (self->splashBeginTime != -1) return;
        self->splash_n = Main_splash_count;
        self->splash   = (Image**) calloc(self->splash_n, sizeof(Image*));
        float ratio = (float) MyCanvas_getHeight(&self->base.base) / 320.0f;
        for (int i = 0; i < self->splash_n; i++)
            self->splash[i] = ImageResize_createImage_sc(Image_createImage(Main_splash[i]), ratio, ratio);
        self->splashBeginTime = qe_now_ms();
    } else if (action == 2) {
        if (Main_langs_count == 1) { onAction(self, 3); return; }
        self->splash = NULL;
        self->background = ImageResize_createImage_wh(Image_createImage(Main_background_logo),
            MyCanvas_getWidth(&self->base.base), MyCanvas_getHeight(&self->base.base));

        char **langs = (char**) calloc(Main_langs_count, sizeof(char*));
        for (int i = 0; i < Main_langs_count; i++) {
            langs[i] = strdup(Main_langs[i]);
            if (GameIni_getNoLang(Main_settings, langs[i]) != NULL) {
                free(langs[i]); langs[i] = strdup(GameIni_getNoLang(Main_settings, Main_langs[i]));
            }
            if (IniFile_get(Main_getGameText(), langs[i]) != NULL) {
                const char *v = GameIni_getNoLang(Main_settings, langs[i]);
                if (v) { free(langs[i]); langs[i] = strdup(v); }
            }
        }
        self->list = ItemList_new_font(langs, Main_langs_count, Main_getFont());
        GUIScreen_setSoftKeysNames(&self->base, IniFile_get(Main_getGameText(), "SELECT"), NULL);
    } else if (action == 3) {
        self->splash = NULL;
        if (self->background == NULL)
            self->background = ImageResize_createImage_wh(Image_createImage(Main_background_logo),
                MyCanvas_getWidth(&self->base.base), MyCanvas_getHeight(&self->base.base));

        const char *audio = IniFile_get(Main_getGameText(), "AUDIO");
        const char *on_   = IniFile_get(Main_getGameText(), "ON");
        const char *off_  = IniFile_get(Main_getGameText(), "OFF");

        char **items = (char**) calloc(2, sizeof(char*));
        int need = (int) (strlen(audio) + strlen(on_) + 2);
        items[0] = (char*) malloc(need); snprintf(items[0], need, "%s:%s", audio, on_);
        need = (int) (strlen(audio) + strlen(off_) + 2);
        items[1] = (char*) malloc(need); snprintf(items[1], need, "%s:%s", audio, off_);

        self->list = ItemList_new_font(items, 2, Main_getFont());
        GUIScreen_setSoftKeysNames(&self->base, IniFile_get(Main_getGameText(), "SELECT"), NULL);
    }
    MyCanvas_repaint(&self->base.base);
}

static void SplashScreen_paint(MyCanvas *mc, Graphics *g) {
    SplashScreen *self = (SplashScreen*) mc;
    int W = MyCanvas_getWidth(mc), H = MyCanvas_getHeight(mc);
    if (self->action == 1) {
        Graphics_setColor(g, 0xffffff);
        Graphics_fillRect(g, 0, 0, W, H);
        int splashIndex = (int)((qe_now_ms() - self->splashBeginTime) / 3500);
        if (self->splash && self->splash_n > splashIndex)
            Graphics_drawImage(g, self->splash[splashIndex], W / 2, H / 2, 3);

        if (qe_now_ms() - self->splashBeginTime >= 3500LL * self->splash_n) {
            if (Main_lang == -1) onAction(self, 2);
            else                 onAction(self, 3);
        } else {
            MyCanvas_repaint(mc);
        }
    } else if (self->action == 2 || self->action == 3) {
        if (self->background) Graphics_drawImage(g, self->background, 0, 0, 0);
        if (self->list) ItemList_draw(self->list, g, 0, 0, W, H);
    }
    if (self->action != 1) GUIScreen_drawSoftKeys(&self->base, g);
}

static void SplashScreen_onKey2(GUIScreen *gs) {
    SplashScreen *self = (SplashScreen*) gs;
    if (self->action == 2 || self->action == 3) { ItemList_scrollUp(self->list); MyCanvas_repaint((MyCanvas*) gs); }
}
static void SplashScreen_onKey8(GUIScreen *gs) {
    SplashScreen *self = (SplashScreen*) gs;
    if (self->action == 2 || self->action == 3) { ItemList_scrollDown(self->list); MyCanvas_repaint((MyCanvas*) gs); }
}
static void SplashScreen_onKey5(GUIScreen *gs) {
    SplashScreen *self = (SplashScreen*) gs;
    if (self->action == 1) {
        self->splashBeginTime = qe_now_ms() - 3500LL * self->splash_n;
    } else if (self->action == 2) {
        char lc[128]; int i = 0;
        const char *l = Main_langs[ItemList_getIndex(self->list)];
        for (; l[i] && i + 1 < 128; i++) lc[i] = (char) tolower((unsigned char) l[i]);
        lc[i] = 0;
        char path[256]; snprintf(path, sizeof path, "/languages/%s.txt", lc);
        Main_setLanguage(path);
        Main_lang = ItemList_getIndex(self->list);
        Main_saveSettingToStore();
        GUIScreen_setFont(&self->base, Main_getFont());
        onAction(self, 3);
    } else if (self->action == 3) {
        int mus = ItemList_getIndex(self->list) == 0;
        Main_isSounds = mus; Main_isMusic = mus; Main_isFootsteps = mus;
        Main_setCurrent((MyCanvas*) Menu_new(self->main));
        SplashScreen_free(self);
    }
}
static void SplashScreen_onLeftSoftKey(GUIScreen *gs) { SplashScreen_onKey5(gs); }

void SplashScreen_reloadScreen(SplashScreen *self) { onAction(self, 3); }

static void SplashScreen_sizeChanged(MyCanvas *mc, int w, int h) {
    SplashScreen *self = (SplashScreen*) mc;
    self->background = ImageResize_createImage_wh(Image_createImage(Main_background_logo), w, h);
}
