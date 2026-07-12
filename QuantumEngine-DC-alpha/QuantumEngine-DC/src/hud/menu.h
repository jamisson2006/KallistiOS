/*
 * menu.h — porte fiel de code/HUD/Menu.java
 */
#ifndef QE_HUD_MENU_H
#define QE_HUD_MENU_H

#include "base/selectable.h"

typedef struct Main  Main;
typedef struct Image Image;

typedef struct Menu {
    Selectable base;
    Main      *main;
    Image     *backgroundMain;
    Image     *background;
    int        newGameIndex, continueIndex, levelSelectIndex;
    int        helpIndex, optionsIndex, exitIndex;
} Menu;

/* Estado global (Java: static) */
extern int Menu_w;
extern int Menu_h;
extern int Menu_hasSave;

Menu *Menu_new(Main *main);
void  Menu_free(Menu *self);
void  Menu_reloadText(Menu *self);
void  Menu_drawBackground   (Menu *self, Graphics *g);
void  Menu_drawBackgroundLogo(Menu *self, Graphics *g);

/* Java: destroy() eh chamado externamente antes de setCurrent(nova tela) */
void  Menu_destroy(Menu *self);

#endif
