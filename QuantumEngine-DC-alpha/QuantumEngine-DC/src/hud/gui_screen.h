/*
 * gui_screen.h — porte fiel de code/HUD/GUIScreen.java
 *
 * Base para telas com softkeys, key handlers e drag scrolling.
 */
#ifndef QE_HUD_GUI_SCREEN_H
#define QE_HUD_GUI_SCREEN_H

#include "../utils/canvas/my_canvas.h"

typedef struct Font         Font;
typedef struct GameKeyboard GameKeyboard;
typedef struct Graphics     Graphics;

typedef struct GUIScreen GUIScreen;

typedef struct GUIScreenVTable {
    MyCanvasVTable canvas; /* superset da vtable de MyCanvas */
    void (*onLeftSoftKey)(GUIScreen *self);
    void (*onRightSoftKey)(GUIScreen *self);
    void (*onKey5)(GUIScreen *self);
    void (*onKey4)(GUIScreen *self);
    void (*onKey6)(GUIScreen *self);
    void (*onKey2)(GUIScreen *self);
    void (*onKey8)(GUIScreen *self);
    void (*onKey7)(GUIScreen *self);
    void (*onKey9)(GUIScreen *self);
    void (*onKeyRepeated4)(GUIScreen *self);
    void (*onKeyRepeated6)(GUIScreen *self);
    void (*onKeyRepeated2)(GUIScreen *self);
    void (*onKeyRepeated8)(GUIScreen *self);
} GUIScreenVTable;

struct GUIScreen {
    MyCanvas    base;
    char       *leftSoft;
    char       *rightSoft;
    int         pressUp, pressDown;
    Font       *font;
    GameKeyboard *keys;
    int         x, y;                 /* ultimo touch */
    int         inverseHScroll;
    int         inverseVScroll;
    int         dragIgnore;
    const GUIScreenVTable *gvt;       /* pointer para a mesma vtable estendida */
};

void GUIScreen_init(GUIScreen *self, const GUIScreenVTable *vt);
void GUIScreen_destroy(GUIScreen *self);

void GUIScreen_set(GUIScreen *self, Font *font, const char *leftSoft, const char *rightSoft);
void GUIScreen_setFont(GUIScreen *self, Font *font);
Font *GUIScreen_getFont(const GUIScreen *self);

void GUIScreen_setSoftKeysNames(GUIScreen *self, const char *left, const char *right);
void GUIScreen_setLeftSoft (GUIScreen *self, const char *n);
void GUIScreen_setRightSoft(GUIScreen *self, const char *n);

void GUIScreen_drawSoftKeys(GUIScreen *self, Graphics *g);

/* Handlers default (no-op para subclasses fazerem override). */
void GUIScreen_default_keyPressed (MyCanvas *self, int key);
void GUIScreen_default_keyRepeated(MyCanvas *self, int key);
void GUIScreen_default_keyReleased(MyCanvas *self, int key);
void GUIScreen_default_pointerPressed(MyCanvas *self, int x, int y);
void GUIScreen_default_pointerDragged(MyCanvas *self, int x, int y);

/* Helpers estaticos (Java: static) */
int  GUIScreen_isLeftSoft (int x, int y, int w, int h);
int  GUIScreen_isRightSoft(int x, int y, int w, int h);

#endif
