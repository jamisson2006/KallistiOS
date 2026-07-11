/*
 * my_canvas.c — porte fiel de code/utils/canvas/MyCanvas.java (Quantum Engine J2ME)
 */
#include "my_canvas.h"
#include "main_canvas.h"

#include <stddef.h>

void MyCanvas_init(MyCanvas *self, const MyCanvasVTable *vt) {
    self->vt = vt;
    self->pointerPressed = 0;
}

void MyCanvas_repaint(MyCanvas *self)                                   { MainCanvas_mainRepaint(self); }
void MyCanvas_repaintRect(MyCanvas *self, int x, int y, int w, int h)   { MainCanvas_mainRepaintRect(self, x, y, w, h); }
void MyCanvas_serviceRepaints(MyCanvas *self)                           { MainCanvas_mainServiceRepaints(self); }

int MyCanvas_getWidth(MyCanvas *self)                                   { (void)self; return MainCanvas_getMainWidth(); }
int MyCanvas_getHeight(MyCanvas *self)                                  { (void)self; return MainCanvas_getMainHeight(); }
const char *MyCanvas_getKeyName(MyCanvas *self, int k)                  { (void)self; return MainCanvas_getMainKeyName(k); }
int MyCanvas_getKeyCode(MyCanvas *self, int a)                          { (void)self; return MainCanvas_getMainKeyCode(a); }

/* --- Defaults (mesmos do Java: showNotify chama repaint; hide vazio, etc.) --- */

void MyCanvas_default_showNotify(MyCanvas *self) { MyCanvas_repaint(self); }
void MyCanvas_default_hideNotify(MyCanvas *self) { (void)self; }
void MyCanvas_default_paint(MyCanvas *self, Graphics *g) { (void)self; (void)g; }
void MyCanvas_default_keyPressed(MyCanvas *self, int keyCode)  { (void)self; (void)keyCode; }
void MyCanvas_default_keyRepeated(MyCanvas *self, int keyCode) { (void)self; (void)keyCode; }
void MyCanvas_default_keyReleased(MyCanvas *self, int keyCode) { (void)self; (void)keyCode; }
void MyCanvas_default_pointerPressed(MyCanvas *self, int x, int y) { (void)x; (void)y; self->pointerPressed = 1; }
void MyCanvas_default_pointerReleased(MyCanvas *self, int x, int y) { (void)x; (void)y; self->pointerPressed = 0; }
void MyCanvas_default_pointerDragged(MyCanvas *self, int x, int y)  { (void)self; (void)x; (void)y; }
void MyCanvas_default_pointerClicked(MyCanvas *self, int x, int y)  { (void)self; (void)x; (void)y; }
void MyCanvas_default_mouseScrollDown(MyCanvas *self) { (void)self; }
void MyCanvas_default_mouseScrollUp(MyCanvas *self)   { (void)self; }
void MyCanvas_default_sizeChanged(MyCanvas *self, int x, int y) { (void)self; (void)x; (void)y; }
