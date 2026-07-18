/*
 * developer_menu.h — porte fiel de code/HUD/DeveloperMenu.java
 */
#ifndef QE_HUD_DEVELOPER_MENU_H
#define QE_HUD_DEVELOPER_MENU_H

#include "base/selectable.h"

typedef struct Main     Main;
typedef struct MyCanvas MyCanvas;

extern int DeveloperMenu_debugMode;
extern int DeveloperMenu_drawLevelPoints;
extern int DeveloperMenu_drawPortals;
extern int DeveloperMenu_renderPolygonsOverwrite;
extern int DeveloperMenu_godMode;
extern int DeveloperMenu_showShootCollision;
extern int DeveloperMenu_showFps;
extern int DeveloperMenu_showRam;
extern int DeveloperMenu_showRoomID;
extern int DeveloperMenu_showPlayerPos;
extern int DeveloperMenu_fly;

typedef struct DeveloperMenu {
    Selectable  base;
    Main       *main;
    MyCanvas   *menu;
    void       *background;
    int         backgroundH;
    int         h;
} DeveloperMenu;

DeveloperMenu *DeveloperMenu_new(Main *main, MyCanvas *menu, void *background);
void           DeveloperMenu_free(DeveloperMenu *self);
void           DeveloperMenu_setFly(DeveloperMenu *self, int fl);

#endif
