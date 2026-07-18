/*
 * main_canvas.h — porte fiel de code/utils/canvas/MainCanvas.java (Quantum Engine J2ME)
 * @author DDDENISSS
 */
#ifndef QE_UTILS_CANVAS_MAIN_CANVAS_H
#define QE_UTILS_CANVAS_MAIN_CANVAS_H

#include "my_canvas.h"

typedef struct MIDlet MIDlet; /* proxy Java; no port DC, opaco */

typedef struct MainCanvas {
    MyCanvas *screen;
} MainCanvas;

/* Estado estatico (Java: public static fields). */
extern int MainCanvas_pointerX, MainCanvas_pointerY;
extern int MainCanvas_pointerMovedX, MainCanvas_pointerMovedY;
extern int MainCanvas_mouseX, MainCanvas_mouseY;
extern int MainCanvas_emulatorScreenWidth, MainCanvas_emulatorScreenHeight;
extern int MainCanvas_pstros;

/* Ctor / setup */
MainCanvas *MainCanvas_new(MIDlet *midlet);
void        MainCanvas_free(MainCanvas *self);

void      MainCanvas_setScreen(MainCanvas *self, MyCanvas *newScreen);
MyCanvas *MainCanvas_getScreen(MainCanvas *self);

/* Encaminhadores usados por MyCanvas (equivalentes aos static getMain*). */
int         MainCanvas_getMainWidth(void);
int         MainCanvas_getMainHeight(void);
const char *MainCanvas_getMainKeyName(int key);
int         MainCanvas_getMainKeyCode(int action);

void MainCanvas_mainRepaint(MyCanvas *screen);
void MainCanvas_mainRepaintRect(MyCanvas *screen, int x, int y, int width, int height);
void MainCanvas_mainServiceRepaints(MyCanvas *screen);

/* API que o loop principal do engine chama (paint/keyPressed/...) */
void MainCanvas_paint(MainCanvas *self, Graphics *g);
void MainCanvas_keyPressed(MainCanvas *self, int keyCode);
void MainCanvas_keyRepeated(MainCanvas *self, int keyCode);
void MainCanvas_keyReleased(MainCanvas *self, int keyCode);
void MainCanvas_pointerPressed(MainCanvas *self, int x, int y);
void MainCanvas_pointerReleased(MainCanvas *self, int x, int y);
void MainCanvas_pointerDragged(MainCanvas *self, int x, int y);
void MainCanvas_sizeChanged(MainCanvas *self, int x, int y);

void MainCanvas_updateMousePos(MainCanvas *self, int x, int y);
void MainCanvas_mouseScrollDown(MainCanvas *self);
void MainCanvas_mouseScrollUp(MainCanvas *self);
void MainCanvas_thisIsPstros(MainCanvas *self);
void MainCanvas_updateScreenSize(MainCanvas *self, int w, int h);

/* Backend: widget nativo do canvas (largura/altura reais da tela).
 * O port DC define isso apontando para o framebuffer. */
typedef struct {
    int  (*getWidth)(void);
    int  (*getHeight)(void);
    const char *(*getKeyName)(int key);
    int  (*getKeyCode)(int action);
    void (*repaintAll)(void);
    void (*repaintRegion)(int x, int y, int w, int h);
    void (*serviceRepaints)(void);
    void (*setFullScreenMode)(int on);
    void (*setCurrentDisplay)(void *canvas_self); /* Display.setCurrent */
} MainCanvasBackend;

void MainCanvas_setBackend(const MainCanvasBackend *bk);

#endif
