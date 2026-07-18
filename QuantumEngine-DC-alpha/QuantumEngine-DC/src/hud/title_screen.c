/*
 * title_screen.c — porte fiel de code/HUD/TitleScreen.java
 */
#include "title_screen.h"
#include "menu.h"
#include "base/text_view.h"
#include "../utils/main.h"
#include "../utils/inifile.h"
#include "../utils/canvas/main_canvas.h"

#include <stdlib.h>

#ifdef _arch_dreamcast
#include <arch/timer.h>
static inline long long qe_now_ms(void) { return (long long) timer_ms_gettime64(); }
#else
#include <sys/time.h>
static inline long long qe_now_ms(void) {
    struct timeval tv; gettimeofday(&tv,0);
    return (long long) tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
#endif

extern void Graphics_setColor(Graphics *g, int col);
extern void Graphics_fillRect(Graphics *g, int x, int y, int w, int h);

static void TitleScreen_onRightSoftKey(GUIScreen *gs) {
    TitleScreen *self = (TitleScreen*) gs;
    if (Sound_hasPlayer(Main_musicPlayer)) Sound_stop(Main_musicPlayer);
    if (self->menu == NULL) self->menu = Menu_new(self->main);
    Main_setCurrent((MyCanvas*) self->menu);
}
static void TitleScreen_paint(MyCanvas *mc, Graphics *g) {
    TitleScreen *self = (TitleScreen*) mc;
    Graphics_setColor(g, 0);
    Graphics_fillRect(g, 0, 0, self->w, self->h);
    Graphics_setColor(g, 0xffffff);

    int y = self->h - (int) ((qe_now_ms() - self->start) * self->pixelsPerSec / 1000);
    TextView_setY(self->text, y);
    TextView_paint(self->text, g, 0, 0);
    if (y == -TextView_getTextHeight(self->text)) { TitleScreen_onRightSoftKey(&self->base); return; }
    MyCanvas_repaint(mc);
}

static const GUIScreenVTable g_vt = {
    {
        TitleScreen_paint,
        GUIScreen_default_keyPressed, GUIScreen_default_keyRepeated, GUIScreen_default_keyReleased,
        GUIScreen_default_pointerPressed, MyCanvas_default_mouseScrollDown, MyCanvas_default_mouseScrollUp,
        MyCanvas_default_pointerReleased, GUIScreen_default_pointerDragged, MyCanvas_default_pointerClicked,
        MyCanvas_default_sizeChanged, MyCanvas_default_showNotify, MyCanvas_default_hideNotify
    },
    NULL, TitleScreen_onRightSoftKey,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

TitleScreen *TitleScreen_new(Main *main, Menu *menu) {
    TitleScreen *ts = (TitleScreen*) calloc(1, sizeof(TitleScreen));
    GUIScreen_init(&ts->base, &g_vt);
    ts->main = main; ts->menu = menu;
    ts->w = MyCanvas_getWidth (&ts->base.base);
    ts->h = MyCanvas_getHeight(&ts->base.base);
    GUIScreen_setFont(&ts->base, Main_getFont());
    GUIScreen_setSoftKeysNames(&ts->base, NULL, NULL);

    ts->text = TextView_new(IniFile_get(Main_getGameText(), "TITLE_SCREEN_TEXT"), ts->w, ts->h, Main_getFont());
    TextView_setCenter(ts->text, 1);
    ts->pixelsPerSec = IniFile_getIntDef(Main_settings, "TITLE_SCREEN_SPEED", 10);
    const char *music = IniFile_get(Main_settings, "TITLE_SCREEN_MUSIC");
    if (Main_isMusic && music) {
        Sound_loadFile(Main_musicPlayer, music);
        Sound_setVolume(Main_musicPlayer, Main_music);
        Sound_start(Main_musicPlayer);
    }
    ts->start = qe_now_ms();
    return ts;
}
void TitleScreen_free(TitleScreen *ts) { if (!ts) return; GUIScreen_destroy(&ts->base); if (ts->text) TextView_free(ts->text); free(ts); }
int  TitleScreen_hasTitleScreen(void) { return IniFile_get(Main_getGameText(), "TITLE_SCREEN_TEXT") != NULL; }
