/*
 * help.h — porte fiel de code/HUD/Help.java
 */
#ifndef QE_HUD_HELP_H
#define QE_HUD_HELP_H

#include "gui_screen.h"

typedef struct Main     Main;
typedef struct Menu     Menu;
typedef struct TextView TextView;

typedef struct Help {
    GUIScreen  base;
    int        y0;
    Main      *main;
    Menu      *menu;
    TextView  *text;
    int        x, y;
} Help;

extern short Help_toMove;

Help *Help_new(Main *main, Menu *menu);
void  Help_free(Help *self);

#endif
