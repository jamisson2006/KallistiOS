/*
 * gui_screen.c — porte fiel de code/HUD/GUIScreen.java
 */
#include "gui_screen.h"
#include "base/font.h"
#include "base/game_keyboard.h"
#include "../utils/canvas/main_canvas.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Constantes MIDP: KEY_NUM2/5/7/8/9/4/6 */
#define QE_KEY_NUM2 50
#define QE_KEY_NUM4 52
#define QE_KEY_NUM5 53
#define QE_KEY_NUM6 54
#define QE_KEY_NUM7 55
#define QE_KEY_NUM8 56
#define QE_KEY_NUM9 57

static inline int qe_abs(int a) { return a < 0 ? -a : a; }

/* dupstr helper */
static char *xdup(const char *s) {
    if (!s) return NULL;
    size_t l = strlen(s) + 1;
    char *o = (char*) malloc(l);
    memcpy(o, s, l);
    return o;
}

void GUIScreen_init(GUIScreen *self, const GUIScreenVTable *vt) {
    MyCanvas_init(&self->base, (const MyCanvasVTable*) vt);
    self->gvt   = vt;
    self->keys  = GameKeyboard_new();
}

void GUIScreen_destroy(GUIScreen *self) {
    free(self->leftSoft); free(self->rightSoft);
    self->font = NULL;
    if (self->keys) { GameKeyboard_free(self->keys); self->keys = NULL; }
}

void GUIScreen_set(GUIScreen *self, Font *font, const char *l, const char *r) {
    self->font = font;
    GUIScreen_setSoftKeysNames(self, l, r);
}

void GUIScreen_setFont(GUIScreen *self, Font *font) { self->font = font; }
Font *GUIScreen_getFont(const GUIScreen *self)       { return self->font; }

void GUIScreen_setSoftKeysNames(GUIScreen *self, const char *l, const char *r) {
    free(self->leftSoft);  self->leftSoft  = xdup(l);
    free(self->rightSoft); self->rightSoft = xdup(r);
}
void GUIScreen_setLeftSoft (GUIScreen *self, const char *n) { free(self->leftSoft);  self->leftSoft  = xdup(n); }
void GUIScreen_setRightSoft(GUIScreen *self, const char *n) { free(self->rightSoft); self->rightSoft = xdup(n); }

void GUIScreen_drawSoftKeys(GUIScreen *self, Graphics *g) {
    int w = MyCanvas_getWidth(&self->base);
    int h = MyCanvas_getHeight(&self->base);
    if (self->font == NULL) { fprintf(stderr, "GUIScreen: ERROR: font == null\n"); return; }
    Font_setY(self->font, 0);
    if (self->leftSoft)  Font_drawString(self->font, g, self->leftSoft,  2,     h, 36);
    if (self->rightSoft) Font_drawString(self->font, g, self->rightSoft, w - 2, h, 40);
}

void GUIScreen_default_keyPressed(MyCanvas *mc, int key) {
    GUIScreen *self = (GUIScreen*) mc;
    Keyboard *k = &self->keys->base;
    if      (key == k->SOFT_LEFT)  self->gvt->onLeftSoftKey(self);
    else if (key == k->SOFT_RIGHT) self->gvt->onRightSoftKey(self);
    else if (key == QE_KEY_NUM7)   self->gvt->onKey7(self);
    else if (key == QE_KEY_NUM9)   self->gvt->onKey9(self);
    else if (key == 53 || key == k->FIRE)  self->gvt->onKey5(self);
    else if (key == 52 || key == k->LEFT)  self->gvt->onKey4(self);
    else if (key == 54 || key == k->RIGHT) self->gvt->onKey6(self);
    else if (key == 50 || key == k->UP)   { self->gvt->onKey2(self); self->pressUp = 1; }
    else if (key == 56 || key == k->DOWN) { self->gvt->onKey8(self); self->pressDown = 1; }
}

void GUIScreen_default_keyRepeated(MyCanvas *mc, int key) {
    GUIScreen *self = (GUIScreen*) mc;
    Keyboard *k = &self->keys->base;
    if      (key == 52 || key == k->LEFT)  self->gvt->onKeyRepeated4(self);
    else if (key == 54 || key == k->RIGHT) self->gvt->onKeyRepeated6(self);
    else if (key == 50 || key == k->UP)    self->gvt->onKeyRepeated2(self);
    else if (key == 56 || key == k->DOWN)  self->gvt->onKeyRepeated8(self);
}

void GUIScreen_default_keyReleased(MyCanvas *mc, int key) {
    GUIScreen *self = (GUIScreen*) mc;
    Keyboard *k = &self->keys->base;
    if (key == 50 || key == k->UP)   self->pressUp = 0;
    else if (key == 56 || key == k->DOWN) self->pressDown = 0;
}

void GUIScreen_default_pointerPressed(MyCanvas *mc, int x, int y) {
    GUIScreen *self = (GUIScreen*) mc;
    if (MainCanvas_pstros) { self->gvt->onLeftSoftKey(self); return; }
    MyCanvas_default_pointerPressed(mc, x, y);
    self->x = x; self->y = y;
    int w = MyCanvas_getWidth(mc), h = MyCanvas_getHeight(mc);
    if (GUIScreen_isLeftSoft (x, y, w, h)) GUIScreen_default_keyPressed(mc, self->keys->base.SOFT_LEFT);
    if (GUIScreen_isRightSoft(x, y, w, h)) GUIScreen_default_keyPressed(mc, self->keys->base.SOFT_RIGHT);
}

void GUIScreen_default_pointerDragged(MyCanvas *mc, int x, int y) {
    GUIScreen *self = (GUIScreen*) mc;
    if (self->dragIgnore) return;
    int w = MyCanvas_getWidth(mc);
    int h = MyCanvas_getHeight(mc);
    if (qe_abs(self->x - x) > w / 7) {
        if (self->x < x) {
            if (!self->inverseHScroll) self->gvt->onKey6(self);
            if ( self->inverseHScroll) self->gvt->onKey4(self);
        }
        if (self->x > x) {
            if (!self->inverseHScroll) self->gvt->onKey4(self);
            if ( self->inverseHScroll) self->gvt->onKey6(self);
        }
        self->x = x;
    }
    if (qe_abs(self->y - y) > h / 7) {
        if (self->y > y) {
            if (!self->inverseVScroll) self->gvt->onKey2(self);
            else                        self->gvt->onKey8(self);
        }
        if (self->y < y) {
            if (!self->inverseVScroll) self->gvt->onKey8(self);
            else                        self->gvt->onKey2(self);
        }
        self->y = y;
    }
}

int GUIScreen_isLeftSoft(int x, int y, int w, int h) {
    int hh = h / 10;
    return x >= 0 && x <= w / 2 - w / 6 && y >= h - hh && y <= h;
}
int GUIScreen_isRightSoft(int x, int y, int w, int h) {
    int hh = h / 10;
    return x >= w / 2 + w / 6 && x <= w && y >= h - hh && y <= h;
}
