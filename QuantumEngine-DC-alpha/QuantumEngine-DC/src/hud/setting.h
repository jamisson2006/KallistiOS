/*
 * setting.h — porte fiel de code/HUD/Setting.java
 */
#ifndef QE_HUD_SETTING_H
#define QE_HUD_SETTING_H

#include "base/selectable.h"

typedef struct Main     Main;
typedef struct MyCanvas MyCanvas;
typedef struct Setting {
    Selectable  base;
    Main       *main;
    MyCanvas   *menu;
    void       *background;   /* Image* ou int[] */
    int         hei, h;
    int         pr7, pr9;
} Setting;

Setting *Setting_new(Main *main, MyCanvas *menu, void *background);
void     Setting_free(Setting *self);

#endif
