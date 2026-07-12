/*
 * game_help.h — porte fiel de code/HUD/GameHelp.java
 */
#ifndef QE_HUD_GAME_HELP_H
#define QE_HUD_GAME_HELP_H

#include "gui_screen.h"

typedef struct Main     Main;
typedef struct Menu     Menu;
typedef struct TextView TextView;
typedef struct Image    Image;

typedef struct GameHelp {
    GUIScreen  base;
    int        y0;
    Main      *main;
    Menu      *menu;
    TextView  *text;
    int        x, y;
    int        levelNumber;
    void      *hudInfo;
    int        stateCode;
    int        showLoad;
    int        preview;
    int        levelEnd;
    Image     *background;
} GameHelp;

extern int   GameHelp_loadSave;
extern int   GameHelp_loadpos;
extern short GameHelp_toMove;

GameHelp *GameHelp_new(Main *main, Menu *menu, int levelNumber, int levelEnd,
                        void *hudInfo, int helpState, int showLoad);
void      GameHelp_free(GameHelp *self);
void      GameHelp_init(GameHelp *self, Main *main, Menu *menu, int levelNumber, int levelEnd,
                         void *hudInfo, int helpState, int showLoad);
int       GameHelp_needToShow(int levelNumber, int helpState, int levelEnd);

#endif
