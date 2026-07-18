/*
 * help.c — porte fiel de code/HUD/Help.java
 */
#include "help.h"
#include "menu.h"
#include "base/text_view.h"
#include "base/font.h"
#include "../utils/main.h"
#include "../utils/inifile.h"
#include "../utils/canvas/main_canvas.h"

#include <stdlib.h>

short Help_toMove = 0;

static inline int qe_max(int a, int b) { return a > b ? a : b; }
static inline int qe_min(int a, int b) { return a < b ? a : b; }

static void Help_paint(MyCanvas *mc, Graphics *g) {
    Help *self = (Help*) mc;
    if (self->base.pressUp   && Help_toMove == 0 && !self->base.base.pointerPressed) Help_toMove += Font_height(Main_getFont());
    if (self->base.pressDown && Help_toMove == 0 && !self->base.base.pointerPressed) Help_toMove -= Font_height(Main_getFont());
    if (Help_toMove != 0 && !self->base.base.pointerPressed) {
        int step = qe_max(qe_min(3, Help_toMove), -3);
        TextView_move(self->text, step);
        Help_toMove -= step;
    }
    Menu_drawBackground(self->menu, g);
    TextView_paint(self->text, g, self->x, self->y);
    GUIScreen_drawSoftKeys(&self->base, g);
    if (!self->base.base.pointerPressed) MyCanvas_repaint(mc);
}

static void Help_onRightSoftKey(GUIScreen *gs) {
    Help *self = (Help*) gs;
    Main_setCurrent((MyCanvas*) self->menu);
}

static void Help_pointerPressed(MyCanvas *mc, int x, int y) {
    Help *self = (Help*) mc;
    GUIScreen_default_pointerPressed(mc, x, y);
    self->y0 = y;
}
static void Help_pointerReleased(MyCanvas *mc, int x, int y) {
    MyCanvas_default_pointerReleased(mc, x, y);
    MyCanvas_repaint(mc);
}
static void Help_pointerDragged(MyCanvas *mc, int x, int y) {
    Help *self = (Help*) mc;
    (void) x;
    int dy = y - self->y0;
    self->y0 = y;
    Help_toMove = 0;
    MyCanvas_serviceRepaints(mc);
    TextView_move(self->text, dy);
    MyCanvas_repaint(mc);
}

static const GUIScreenVTable g_vt = {
    {
        Help_paint,
        GUIScreen_default_keyPressed,
        GUIScreen_default_keyRepeated,
        GUIScreen_default_keyReleased,
        Help_pointerPressed,
        MyCanvas_default_mouseScrollDown, MyCanvas_default_mouseScrollUp,
        Help_pointerReleased,
        Help_pointerDragged,
        MyCanvas_default_pointerClicked,
        MyCanvas_default_sizeChanged,
        MyCanvas_default_showNotify, MyCanvas_default_hideNotify
    },
    NULL, Help_onRightSoftKey,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

Help *Help_new(Main *main, Menu *menu) {
    Help *h = (Help*) calloc(1, sizeof(Help));
    GUIScreen_init(&h->base, &g_vt);
    h->main = main; h->menu = menu;
    GUIScreen_setFont(&h->base, Main_getFont());
    GUIScreen_setSoftKeysNames(&h->base, NULL, IniFile_get(Main_getGameText(), "BACK"));
    h->x = MyCanvas_getWidth (&h->base.base) / 15;
    h->y = MyCanvas_getHeight(&h->base.base) / 13;
    int w = MyCanvas_getWidth (&h->base.base) - h->x * 2;
    int hh= MyCanvas_getHeight(&h->base.base) - h->y * 2;
    h->text = TextView_new(IniFile_get(Main_getGameText(), "HELP_TEXT"), w, hh, Main_getFont());
    TextView_setCenter(h->text, 1);
    Help_toMove = 0;
    h->base.base.pointerPressed = 0;
    return h;
}

void Help_free(Help *h) {
    if (!h) return;
    GUIScreen_destroy(&h->base);
    if (h->text) TextView_free(h->text);
    free(h);
}
