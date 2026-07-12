/*
 * game_keyboard.c — porte fiel de code/HUD/Base/GameKeyboard.java
 */
#include "game_keyboard.h"
#include "../../utils/main.h"
#include "../../utils/canvas/main_canvas.h"

#include <stdlib.h>
#include <string.h>

int  *GameKeyboard_keyCodes    = NULL;
int   GameKeyboard_keyCodes_len= 0;
int  *GameKeyboard_hasKeyCodes = NULL;

GameKeyboard *GameKeyboard_new(void) {
    GameKeyboard *k = (GameKeyboard*) calloc(1, sizeof(GameKeyboard));
    /* Java: super(Main.mainCanvas) — sem isSelect callback (DC nao tem
     * getKeyName variando por device); default Nokia/SE. */
    Keyboard *kb = Keyboard_new(NULL, NULL);
    k->base = *kb; free(kb);

    if (GameKeyboard_keyCodes == NULL) GameKeyboard_initKeycodes(k);
    if (GameKeyboard_keyCodes != NULL && GameKeyboard_keyCodes_len != 48) GameKeyboard_initKeycodes(k);
    return k;
}
void GameKeyboard_free(GameKeyboard *self) { free(self); }

void GameKeyboard_reset(GameKeyboard *self) {
    Keyboard_reset(&self->base);
    for (int i = 0; i < 10; i++) self->pressed[i] = 0;
}

void GameKeyboard_keyInput(GameKeyboard *self, int key, int action) {
    for (int i = 0; i < 10; i++) {
        if (self->pressed[i] == !action) {
            if (!action && self->pressedNums[i] == key) self->pressed[i] = 0;
            if (action) { self->pressedNums[i] = key; self->pressed[i] = 1; break; }
        }
    }
    Keyboard_keyInput(&self->base, key, action);
}

int GameKeyboard_isPressed(const GameKeyboard *self, int key) {
    for (int i = 0; i < 10; i++)
        if (self->pressed[i] && self->pressedNums[i] == key) return 1;
    return 0;
}

void GameKeyboard_initKeycodes(GameKeyboard *self) {
    static int codes[48];
    static int has[48];
    int UP=self->base.UP, DOWN=self->base.DOWN, LEFT=self->base.LEFT, RIGHT=self->base.RIGHT;
    int FIRE=self->base.FIRE, SL=self->base.SOFT_LEFT;

    int c[48] = {
        UP, QE_KEY_NUM2,   DOWN, QE_KEY_NUM8,   QE_KEY_NUM7, 0,   QE_KEY_NUM9, 0,
        LEFT, QE_KEY_NUM4, RIGHT, QE_KEY_NUM6,  QE_KEY_NUM3, 0,   QE_KEY_NUM1, 0,
        FIRE, QE_KEY_NUM5, QE_KEY_NUM0, 0,      SL, 0,            QE_KEY_POUND, 0,
        0, 0,              QE_KEY_STAR, 0,
        QE_KEY_NUM3, 0,    QE_KEY_NUM1, 0,      QE_KEY_NUM7, 0,   QE_KEY_NUM9, 0,
        LEFT, QE_KEY_NUM4, RIGHT, QE_KEY_NUM6,  UP, QE_KEY_NUM2,  DOWN, QE_KEY_NUM8,
        0, 0,              SL, 0
    };
    int h[48] = {
        1,1, 1,1, 1,0, 1,0,
        1,1, 1,1, 1,0, 1,0,
        1,1, 1,0, 1,0, 1,0,
        0,0, 1,0,
        1,0, 1,0, 1,0, 1,0,
        1,1, 1,1, 1,1, 1,1,
        0,0, 1,0
    };
    memcpy(codes, c, sizeof codes);
    memcpy(has, h, sizeof has);
    GameKeyboard_keyCodes     = codes;
    GameKeyboard_keyCodes_len = 48;
    GameKeyboard_hasKeyCodes  = has;
}

/* Movimento — o padrao Java eh: hasKeyCodes[i] && isPressed(keyCodes[i]).
 * Cada acao usa 2 slots consecutivos (base + base+1) para sight-off e
 * (base+28 + base+29) para sight-on. Fatorei em macros. */

#define QE_CHECK_PAIR(kb, base) ( \
    (GameKeyboard_hasKeyCodes[(base)  ] && GameKeyboard_isPressed((kb), GameKeyboard_keyCodes[(base)  ])) || \
    (GameKeyboard_hasKeyCodes[(base)+1] && GameKeyboard_isPressed((kb), GameKeyboard_keyCodes[(base)+1])))

#define QE_CHECK_WALK(kb, sight, base_off, base_on) \
    ((sight) ? QE_CHECK_PAIR((kb), (base_on)) : QE_CHECK_PAIR((kb), (base_off)))

int GameKeyboard_isWalkForward (const GameKeyboard *k, int s) { return QE_CHECK_WALK(k, s, 0, 28); }
int GameKeyboard_isWalkBackward(const GameKeyboard *k, int s) { return QE_CHECK_WALK(k, s, 2, 30); }
int GameKeyboard_isWalkLeft    (const GameKeyboard *k, int s) { return QE_CHECK_WALK(k, s, 4, 32); }
int GameKeyboard_isWalkRight   (const GameKeyboard *k, int s) { return QE_CHECK_WALK(k, s, 6, 34); }
int GameKeyboard_isLookLeft    (const GameKeyboard *k, int s) { return QE_CHECK_WALK(k, s, 8, 36); }
int GameKeyboard_isLookRight   (const GameKeyboard *k, int s) { return QE_CHECK_WALK(k, s, 10, 38); }
int GameKeyboard_isLookUp      (const GameKeyboard *k, int s) { return QE_CHECK_WALK(k, s, 12, 40); }
int GameKeyboard_isLookDown    (const GameKeyboard *k, int s) { return QE_CHECK_WALK(k, s, 14, 42); }

int GameKeyboard_isPlayerShooting(const GameKeyboard *k) {
    return (GameKeyboard_hasKeyCodes[16] && GameKeyboard_isPressed(k, GameKeyboard_keyCodes[16]))
        || (GameKeyboard_hasKeyCodes[17] && GameKeyboard_isPressed(k, GameKeyboard_keyCodes[17]));
}

void GameKeyboard_releasePlayerShoot(GameKeyboard *k) {
    if (GameKeyboard_hasKeyCodes[16]) GameKeyboard_keyInput(k, GameKeyboard_keyCodes[16], 0);
    if (GameKeyboard_hasKeyCodes[17]) GameKeyboard_keyInput(k, GameKeyboard_keyCodes[17], 0);
}

static int check_static(int key, int base) {
    return (GameKeyboard_hasKeyCodes[base]   && GameKeyboard_keyCodes[base]   == key)
        || (GameKeyboard_hasKeyCodes[base+1] && GameKeyboard_keyCodes[base+1] == key);
}
int GameKeyboard_isSightKey         (int k) { return check_static(k, 26); }
int GameKeyboard_isUseKey           (int k) { return check_static(k, 46); }
int GameKeyboard_isInventoryKey     (int k) { return check_static(k, 20); }
int GameKeyboard_isJumpKey          (int k) { return check_static(k, 18); }
int GameKeyboard_isNextWeaponKey    (int k) { return check_static(k, 22); }
int GameKeyboard_isPreviousWeaponKey(int k) { return check_static(k, 24); }
