/*
 * keys_settings.c — porte fiel de code/HUD/KeysSettings.java
 */
#include "keys_settings.h"
#include "setting.h"
#include "base/font.h"
#include "base/item_list.h"
#include "base/game_keyboard.h"
#include "../utils/main.h"
#include "../utils/inifile.h"
#include "../utils/canvas/main_canvas.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void Graphics_setColor(Graphics *g, int c);
extern void Graphics_fillRect(Graphics *g, int x, int y, int w, int h);

static char *key_name_or_num(int code) {
    /* No DC nao ha getKeyName do controller — devolvemos so o codigo. */
    char buf[32]; snprintf(buf, sizeof buf, "%d", code);
    return strdup(buf);
}

static int getMove(int keysFolder) {
    if (keysFolder == 1) return 14 - 4;
    if (keysFolder == 2) return 8 - 4;
    return -4;
}

static char *label_for(const char *base_label, int move, int i) {
    char tmp[128] = "";
    if (GameKeyboard_hasKeyCodes[(i + move) * 2])
        snprintf(tmp, sizeof tmp, "%d", GameKeyboard_keyCodes[(i + move) * 2]);
    if (GameKeyboard_hasKeyCodes[(i + move) * 2 + 1]) {
        char t2[64]; snprintf(t2, sizeof t2, " %d", GameKeyboard_keyCodes[(i + move) * 2 + 1]);
        strncat(tmp, t2, sizeof tmp - strlen(tmp) - 1);
    }
    char out[192]; snprintf(out, sizeof out, "%s: %s", base_label, tmp);
    return strdup(out);
}

static void setItems(KeysSettings *self) {
    if (self->keysFolder == 2) self->keysCount = 7;
    else                       self->keysCount = 8;

    int total = 4 + self->keysCount;
    char **var2 = (char**) calloc(total, sizeof(char*));
    int   *ms   = (int*)   calloc(total, sizeof(int));
    IniFile *txt = Main_getGameText();
    int indexOld = 4;
    if (self->base.list) indexOld = self->base.list->index;
    int i = 0;

    const char *hdr = "";
    if (self->keysFolder == 0) hdr = IniFile_get(txt, "MAIN_KEYS");
    if (self->keysFolder == 1) hdr = IniFile_get(txt, "SIGHT_KEYS");
    if (self->keysFolder == 2) hdr = IniFile_get(txt, "OTHER_KEYS");
    var2[i] = strdup(hdr); ms[i] = 1; i++;
    var2[i] = strdup(""); ms[i] = 1; i++;
    var2[i] = strdup(IniFile_get(txt, "RESET_ALL_KEYS")); ms[i] = 0; i++;
    var2[i] = strdup(""); ms[i] = 1; i++;

    int move = getMove(self->keysFolder);
    if (self->keysFolder <= 1) {
        const char *labels[8] = {
            "WALK_FORWARD_KEY","WALK_BACKWARD_KEY","WALK_LEFT_KEY","WALK_RIGHT_KEY",
            "LOOK_LEFT_KEY","LOOK_RIGHT_KEY","LOOK_UP_KEY","LOOK_DOWN_KEY"
        };
        for (int j = 0; j < 8; j++) {
            var2[i] = label_for(IniFile_get(txt, labels[j]), move, i);
            ms[i] = 0; i++;
        }
    } else {
        const char *labels[7] = {
            "FIRE_KEY","JUMP_KEY","INVENTORY_KEY","NEXT_WEAPON_KEY",
            "PREVIOUS_WEAPON_KEY","SIGHT_MODE_KEY","INTERACT_KEY"
        };
        /* Java: FIRE..SIGHT_MODE (move=8-4). Depois INTERACT_KEY usa move=23-10. */
        for (int j = 0; j < 6; j++) {
            var2[i] = label_for(IniFile_get(txt, labels[j]), move, i);
            ms[i] = 0; i++;
        }
        int move2 = 23 - 10;
        var2[i] = label_for(IniFile_get(txt, "INTERACT_KEY"), move2, i);
        ms[i] = 0; i++;
    }
    Selectable_set_ms(&self->base, Main_getFont(), var2, total,
        IniFile_get(txt, "RESET_KEYS"), IniFile_get(txt, "BACK"),
        ms, total);
    ItemList_setIndex(self->base.list, indexOld);
    self->base.list->left = 1;
}

static void KeysSettings_paint(MyCanvas *mc, Graphics *g) {
    KeysSettings *self = (KeysSettings*) mc;
    Graphics_setColor(g, 0);
    Graphics_fillRect(g, 0, 0, MyCanvas_getWidth(mc), MyCanvas_getHeight(mc));

    if (self->pauseScreen) {
        ItemList_drawBck(self->base.list, g, MyCanvas_getWidth(mc) / 8, Font_height(Main_getFont()),
                         MyCanvas_getWidth(mc) * 6 / 8, MyCanvas_getHeight(mc) - Font_height(Main_getFont()) * 2);
        Main_drawBckDialog(g, MyCanvas_getHeight(mc) - Font_height(Main_getFont()), MyCanvas_getHeight(mc));
    }
    ItemList_draw(self->base.list, g, MyCanvas_getWidth(mc) / 8, Font_height(Main_getFont()),
                  MyCanvas_getWidth(mc) * 6 / 8, MyCanvas_getHeight(mc) - Font_height(Main_getFont()) * 2);
    GUIScreen_drawSoftKeys(&self->base.base, g);
}

static void KeysSettings_setKey(int keyCode, int index) {
    if (GameKeyboard_hasKeyCodes[index] && GameKeyboard_hasKeyCodes[index + 1]) {
        GameKeyboard_hasKeyCodes[index]     = 0;
        GameKeyboard_hasKeyCodes[index + 1] = 0;
    }
    if (!GameKeyboard_hasKeyCodes[index]) {
        GameKeyboard_hasKeyCodes[index] = 1;
        GameKeyboard_keyCodes[index]    = keyCode;
    } else if (!GameKeyboard_hasKeyCodes[index + 1]) {
        GameKeyboard_hasKeyCodes[index + 1] = 1;
        GameKeyboard_keyCodes[index + 1]    = keyCode;
    }
}

static void KeysSettings_keyPressed(MyCanvas *mc, int keyCode) {
    KeysSettings *self = (KeysSettings*) mc;
    if (!self->set) { GUIScreen_default_keyPressed(mc, keyCode); return; }
    int ki = (ItemList_getIndex(self->base.list) + getMove(self->keysFolder)) * 2;
    if (self->keysFolder == 2 && ItemList_getIndex(self->base.list) >= 10) {
        ki = (ItemList_getIndex(self->base.list) - 10 + 23) * 2;
    }
    KeysSettings_setKey(keyCode, ki);
    self->set = 0;
    self->base.list->redact = 0;
    setItems(self);
    MyCanvas_repaint(mc);
}

static void KeysSettings_onRightSoftKey(GUIScreen *gs) {
    KeysSettings *self = (KeysSettings*) gs;
    Main_setCurrent((MyCanvas*) self->setting);
}
static void KeysSettings_onLeftSoftKey(GUIScreen *gs) {
    KeysSettings *self = (KeysSettings*) gs;
    int idx = ItemList_getIndex(self->base.list);
    if (idx < 4) return;
    int index = (idx + getMove(self->keysFolder)) * 2;
    if (self->keysFolder == 2 && idx >= 10) index = (idx - 10 + 23) * 2;
    GameKeyboard_hasKeyCodes[index]     = 0;
    GameKeyboard_hasKeyCodes[index + 1] = 0;
    setItems(self);
    MyCanvas_repaint(&gs->base);
}
static void KeysSettings_onKey5(GUIScreen *gs) {
    KeysSettings *self = (KeysSettings*) gs;
    int idx = ItemList_getIndex(self->base.list);
    if (idx == 3) { GameKeyboard_initKeycodes(self->base.base.keys); setItems(self); MyCanvas_repaint(&gs->base); }
    if (idx >= 4) { self->set = 1; self->base.list->redact = 1; MyCanvas_repaint(&gs->base); }
}
static void KeysSettings_onKey6(GUIScreen *gs) {
    KeysSettings *self = (KeysSettings*) gs;
    self->keysFolder++;
    if (self->keysFolder > 2) self->keysFolder = 0;
    setItems(self);
    if (ItemList_getIndex(self->base.list) >= self->keysCount) ItemList_setIndex(self->base.list, self->keysCount);
    MyCanvas_repaint(&gs->base);
}
static void KeysSettings_onKey4(GUIScreen *gs) {
    KeysSettings *self = (KeysSettings*) gs;
    self->keysFolder--;
    if (self->keysFolder < 0) self->keysFolder = 2;
    setItems(self);
    if (ItemList_getIndex(self->base.list) >= self->keysCount) ItemList_setIndex(self->base.list, self->keysCount);
    MyCanvas_repaint(&gs->base);
}

static const GUIScreenVTable g_vt = {
    {
        KeysSettings_paint,
        KeysSettings_keyPressed, GUIScreen_default_keyRepeated, GUIScreen_default_keyReleased,
        GUIScreen_default_pointerPressed, MyCanvas_default_mouseScrollDown, MyCanvas_default_mouseScrollUp,
        MyCanvas_default_pointerReleased, GUIScreen_default_pointerDragged, MyCanvas_default_pointerClicked,
        MyCanvas_default_sizeChanged, MyCanvas_default_showNotify, MyCanvas_default_hideNotify
    },
    KeysSettings_onLeftSoftKey, KeysSettings_onRightSoftKey,
    KeysSettings_onKey5, KeysSettings_onKey4, KeysSettings_onKey6,
    Selectable_onKey2, Selectable_onKey8,
    NULL, NULL, NULL, NULL, NULL, NULL
};

KeysSettings *KeysSettings_new(Main *main, Setting *setting, void *background, int pauseScreen) {
    KeysSettings *k = (KeysSettings*) calloc(1, sizeof(KeysSettings));
    GUIScreen_init(&k->base.base, &g_vt);
    k->main = main; k->setting = setting; k->background = background;
    k->pauseScreen = pauseScreen; k->keysFolder = 0; k->keysCount = 8;
    setItems(k);
    k->hei = MyCanvas_getHeight(&k->base.base.base) / 2 - MyCanvas_getHeight(&k->base.base.base) * Main_getDisplaySize() / 200;
    k->h   = MyCanvas_getHeight(&k->base.base.base) * Main_getDisplaySize() / 100;
    return k;
}
void KeysSettings_free(KeysSettings *k) { if (!k) return; Selectable_destroy(&k->base); free(k); }
