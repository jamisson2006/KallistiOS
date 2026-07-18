/*
 * splash_screen.h — porte fiel de code/HUD/SplashScreen.java
 */
#ifndef QE_HUD_SPLASH_SCREEN_H
#define QE_HUD_SPLASH_SCREEN_H

#include "gui_screen.h"
#include "base/item_list.h"

typedef struct Image Image;
typedef struct Main  Main;

typedef struct SplashScreen {
    GUIScreen   base;
    Main       *main;
    int         action;
    Image     **splash;    int splash_n;
    Image      *background;
    ItemList   *list;
    long long   splashBeginTime;
} SplashScreen;

SplashScreen *SplashScreen_new(Main *main);
void          SplashScreen_free(SplashScreen *self);
void          SplashScreen_reloadScreen(SplashScreen *self);

#endif
