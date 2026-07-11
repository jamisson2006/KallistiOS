/*
 * main_canvas.c — porte fiel de code/utils/canvas/MainCanvas.java (Quantum Engine J2ME)
 */
#include "main_canvas.h"

#include <stdlib.h>
#include <stddef.h>

#ifdef __GNUC__
#include <stdlib.h> /* abs */
#endif

int MainCanvas_pointerX = 0, MainCanvas_pointerY = 0;
int MainCanvas_pointerMovedX = 0, MainCanvas_pointerMovedY = 0;
int MainCanvas_mouseX = 0, MainCanvas_mouseY = 0;
int MainCanvas_emulatorScreenWidth = 0, MainCanvas_emulatorScreenHeight = 0;
int MainCanvas_pstros = 0;

/* Java: private static MainCanvas mainCanvas; */
static MainCanvas *g_mainCanvas = NULL;
static const MainCanvasBackend *g_backend = NULL;

void MainCanvas_setBackend(const MainCanvasBackend *bk) { g_backend = bk; }

MainCanvas *MainCanvas_new(MIDlet *midlet) {
    (void) midlet;
    MainCanvas *m = (MainCanvas*) calloc(1, sizeof(MainCanvas));
    if (g_backend && g_backend->setFullScreenMode) g_backend->setFullScreenMode(1);
    g_mainCanvas = m;
    if (g_backend && g_backend->setCurrentDisplay) g_backend->setCurrentDisplay(m);
    return m;
}

void MainCanvas_free(MainCanvas *self) {
    if (g_mainCanvas == self) g_mainCanvas = NULL;
    free(self);
}

void MainCanvas_setScreen(MainCanvas *self, MyCanvas *newScreen) {
    if (self->screen != NULL && self->screen->vt && self->screen->vt->hideNotify)
        self->screen->vt->hideNotify(self->screen);
    self->screen = newScreen;
    if (self->screen && self->screen->vt && self->screen->vt->showNotify)
        self->screen->vt->showNotify(self->screen);
}

MyCanvas *MainCanvas_getScreen(MainCanvas *self) { return self->screen; }

/* --- helpers estaticos que consultam mainCanvas via g_backend --- */

int MainCanvas_getMainWidth(void)  { return g_backend && g_backend->getWidth  ? g_backend->getWidth()  : 0; }
int MainCanvas_getMainHeight(void) { return g_backend && g_backend->getHeight ? g_backend->getHeight() : 0; }
const char *MainCanvas_getMainKeyName(int key) {
    return g_backend && g_backend->getKeyName ? g_backend->getKeyName(key) : "";
}
int MainCanvas_getMainKeyCode(int action) {
    return g_backend && g_backend->getKeyCode ? g_backend->getKeyCode(action) : 0;
}

void MainCanvas_mainRepaint(MyCanvas *screen) {
    if (g_mainCanvas && g_mainCanvas->screen == screen && g_backend && g_backend->repaintAll)
        g_backend->repaintAll();
}
void MainCanvas_mainRepaintRect(MyCanvas *screen, int x, int y, int width, int height) {
    if (g_mainCanvas && g_mainCanvas->screen == screen && g_backend && g_backend->repaintRegion)
        g_backend->repaintRegion(x, y, width, height);
}
void MainCanvas_mainServiceRepaints(MyCanvas *screen) {
    if (g_mainCanvas && g_mainCanvas->screen == screen && g_backend && g_backend->serviceRepaints)
        g_backend->serviceRepaints();
}

/* --- Encaminhadores para a screen atual (Java: protected paint/keyPressed/...) --- */

void MainCanvas_paint(MainCanvas *self, Graphics *g) {
    if (self->screen && self->screen->vt && self->screen->vt->paint)
        self->screen->vt->paint(self->screen, g);
}

void MainCanvas_keyPressed(MainCanvas *self, int keyCode) {
    if (self->screen && self->screen->vt && self->screen->vt->keyPressed)
        self->screen->vt->keyPressed(self->screen, keyCode);
}
void MainCanvas_keyRepeated(MainCanvas *self, int keyCode) {
    if (self->screen && self->screen->vt && self->screen->vt->keyRepeated)
        self->screen->vt->keyRepeated(self->screen, keyCode);
}
void MainCanvas_keyReleased(MainCanvas *self, int keyCode) {
    if (self->screen && self->screen->vt && self->screen->vt->keyReleased)
        self->screen->vt->keyReleased(self->screen, keyCode);
}

void MainCanvas_pointerPressed(MainCanvas *self, int x, int y) {
    MainCanvas_pointerX = x; MainCanvas_pointerY = y;
    MainCanvas_pointerMovedX = MainCanvas_pointerMovedY = 0;
    if (self->screen && self->screen->vt && self->screen->vt->pointerPressed)
        self->screen->vt->pointerPressed(self->screen, x, y);
}

void MainCanvas_pointerReleased(MainCanvas *self, int x, int y) {
    if (self->screen && self->screen->vt && self->screen->vt->pointerReleased)
        self->screen->vt->pointerReleased(self->screen, x, y);
    if (MainCanvas_pointerMovedX <= 4 && MainCanvas_pointerMovedY <= 4
        && self->screen && self->screen->vt && self->screen->vt->pointerClicked)
        self->screen->vt->pointerClicked(self->screen, x, y);
}

void MainCanvas_pointerDragged(MainCanvas *self, int x, int y) {
    MainCanvas_pointerMovedX += abs(x - MainCanvas_pointerX);
    MainCanvas_pointerMovedY += abs(y - MainCanvas_pointerY);
    MainCanvas_pointerX = x; MainCanvas_pointerY = y;
    if (self->screen && self->screen->vt && self->screen->vt->pointerDragged)
        self->screen->vt->pointerDragged(self->screen, x, y);
}

void MainCanvas_sizeChanged(MainCanvas *self, int x, int y) {
    if (self->screen && self->screen->vt && self->screen->vt->sizeChanged)
        self->screen->vt->sizeChanged(self->screen, x, y);
}

void MainCanvas_updateMousePos(MainCanvas *self, int x, int y) {
    (void) self;
    MainCanvas_mouseX = x; MainCanvas_mouseY = y;
}

void MainCanvas_mouseScrollDown(MainCanvas *self) {
    if (self->screen && self->screen->vt && self->screen->vt->mouseScrollDown)
        self->screen->vt->mouseScrollDown(self->screen);
}
void MainCanvas_mouseScrollUp(MainCanvas *self) {
    if (self->screen && self->screen->vt && self->screen->vt->mouseScrollUp)
        self->screen->vt->mouseScrollUp(self->screen);
}

void MainCanvas_thisIsPstros(MainCanvas *self) { (void)self; MainCanvas_pstros = 1; }

void MainCanvas_updateScreenSize(MainCanvas *self, int w, int h) {
    (void) self;
    MainCanvas_emulatorScreenWidth  = w;
    MainCanvas_emulatorScreenHeight = h;
}
