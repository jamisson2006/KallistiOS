#ifndef QE_HUD_LOADING_SCREEN_H
#define QE_HUD_LOADING_SCREEN_H

#include "gui_screen.h"

typedef struct Main       Main;
typedef struct GameScreen GameScreen;

typedef struct LoadingScreen {
    GUIScreen  base;
    Main      *main;
    int        frames;
    int        lvl;
    int        loadSave;
    int        loadPos;
    int        showLoad;
} LoadingScreen;

LoadingScreen *LoadingScreen_new(Main *main, int lvl, int loadSave, int loadPos, int showLoad);
void           LoadingScreen_free(LoadingScreen *self);

#endif
