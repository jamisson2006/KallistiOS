#ifndef QE_HUD_TITLE_SCREEN_H
#define QE_HUD_TITLE_SCREEN_H
#include "gui_screen.h"
typedef struct Main     Main;
typedef struct Menu     Menu;
typedef struct TextView TextView;

typedef struct TitleScreen {
    GUIScreen  base;
    Main      *main;
    Menu      *menu;
    TextView  *text;
    int        pixelsPerSec;
    int        w, h;
    long long  start;
} TitleScreen;

TitleScreen *TitleScreen_new(Main *main, Menu *menu);
void         TitleScreen_free(TitleScreen *self);
int          TitleScreen_hasTitleScreen(void);
#endif
