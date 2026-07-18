/*
 * pause_screen.h — porte fiel de code/HUD/PauseScreen.java
 */
#ifndef QE_HUD_PAUSE_SCREEN_H
#define QE_HUD_PAUSE_SCREEN_H

#include "base/selectable.h"

typedef struct Main       Main;
typedef struct GameScreen GameScreen;

typedef struct PauseScreen {
    Selectable  base;
    Main       *main;
    GameScreen *gameScreen;
    int        *background;   /* int[] no Java (framebuffer congelado) */
    int         mus;
    long long   mt;
    int         height3D;
    int         backgroundH;
    int         state;        /* 0=menu, 1=confirm exit, 2=save-on-quit */
    int         lastIndex;
    int         saveGame;
} PauseScreen;

PauseScreen *PauseScreen_new(Main *main, GameScreen *gs, int *background, long long time);
void         PauseScreen_free(PauseScreen *self);
void         PauseScreen_reloadText(PauseScreen *self);

#endif
