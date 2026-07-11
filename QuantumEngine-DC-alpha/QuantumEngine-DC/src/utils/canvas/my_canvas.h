/*
 * my_canvas.h — porte fiel de code/utils/canvas/MyCanvas.java (Quantum Engine J2ME)
 * @author DDDENISSS
 *
 * MyCanvas eh a classe base (abstrata) para todas as telas (Splash, Menu,
 * GameScreen, ...). Em Java: extends Canvas. Aqui usamos uma vtable de
 * ponteiros de funcao para simular o polimorfismo.
 */
#ifndef QE_UTILS_CANVAS_MY_CANVAS_H
#define QE_UTILS_CANVAS_MY_CANVAS_H

typedef struct Graphics  Graphics;
typedef struct MyCanvas  MyCanvas;

typedef struct MyCanvasVTable {
    void (*paint)(MyCanvas *self, Graphics *g); /* abstract no Java */
    void (*keyPressed)(MyCanvas *self, int keyCode);
    void (*keyRepeated)(MyCanvas *self, int keyCode);
    void (*keyReleased)(MyCanvas *self, int keyCode);
    void (*pointerPressed)(MyCanvas *self, int x, int y);
    void (*mouseScrollDown)(MyCanvas *self);
    void (*mouseScrollUp)(MyCanvas *self);
    void (*pointerReleased)(MyCanvas *self, int x, int y);
    void (*pointerDragged)(MyCanvas *self, int x, int y);
    void (*pointerClicked)(MyCanvas *self, int x, int y);
    void (*sizeChanged)(MyCanvas *self, int x, int y);
    void (*showNotify)(MyCanvas *self);
    void (*hideNotify)(MyCanvas *self);
} MyCanvasVTable;

struct MyCanvas {
    const MyCanvasVTable *vt;
    int pointerPressed;
};

/* Setup base (chamado por subclasses no init). */
void MyCanvas_init(MyCanvas *self, const MyCanvasVTable *vt);

/* Metodos que redirecionam para MainCanvas (mesmo comportamento Java). */
void MyCanvas_repaint(MyCanvas *self);
void MyCanvas_repaintRect(MyCanvas *self, int x, int y, int width, int height);
void MyCanvas_serviceRepaints(MyCanvas *self);

int  MyCanvas_getWidth(MyCanvas *self);
int  MyCanvas_getHeight(MyCanvas *self);
const char *MyCanvas_getKeyName(MyCanvas *self, int keyCode);
int         MyCanvas_getKeyCode(MyCanvas *self, int action);

/* Implementacoes default do Java (as subclasses fazem override). */
void MyCanvas_default_showNotify(MyCanvas *self);
void MyCanvas_default_hideNotify(MyCanvas *self);
void MyCanvas_default_paint(MyCanvas *self, Graphics *g);
void MyCanvas_default_keyPressed(MyCanvas *self, int keyCode);
void MyCanvas_default_keyRepeated(MyCanvas *self, int keyCode);
void MyCanvas_default_keyReleased(MyCanvas *self, int keyCode);
void MyCanvas_default_pointerPressed(MyCanvas *self, int x, int y);
void MyCanvas_default_pointerReleased(MyCanvas *self, int x, int y);
void MyCanvas_default_pointerDragged(MyCanvas *self, int x, int y);
void MyCanvas_default_pointerClicked(MyCanvas *self, int x, int y);
void MyCanvas_default_mouseScrollDown(MyCanvas *self);
void MyCanvas_default_mouseScrollUp(MyCanvas *self);
void MyCanvas_default_sizeChanged(MyCanvas *self, int x, int y);

#endif
