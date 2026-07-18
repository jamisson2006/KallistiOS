/*
 * keys_settings.h — porte fiel de code/HUD/KeysSettings.java
 */
#ifndef QE_HUD_KEYS_SETTINGS_H
#define QE_HUD_KEYS_SETTINGS_H

#include "base/selectable.h"
#include <stdint.h>

typedef struct Main    Main;
typedef struct Setting Setting;

typedef struct KeysSettings {
    Selectable base;
    Main      *main;
    Setting   *setting;
    void      *background;
    int        pauseScreen;
    int        set;
    int        hei, h;
    int8_t     keysFolder;    /* 0=movimento, 1=sight, 2=acoes */
    int        keysCount;
} KeysSettings;

KeysSettings *KeysSettings_new(Main *main, Setting *setting, void *background, int pauseScreen);
void          KeysSettings_free(KeysSettings *self);

#endif
