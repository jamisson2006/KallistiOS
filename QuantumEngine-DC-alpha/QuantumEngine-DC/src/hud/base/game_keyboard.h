/*
 * game_keyboard.h — porte fiel de code/HUD/Base/GameKeyboard.java
 * @author Roman Lahin
 *
 * Estende Keyboard adicionando 48 binds configuráveis (24 acoes x 2 slots).
 */
#ifndef QE_HUD_BASE_GAME_KEYBOARD_H
#define QE_HUD_BASE_GAME_KEYBOARD_H

#include "../../utils/keyboard.h"

typedef struct GameKeyboard {
    Keyboard base;
    int pressedNums[10];
    int pressed[10];       /* boolean */
} GameKeyboard;

/* Java: public static int[] keyCodes; public static boolean[] hasKeyCodes; */
extern int  *GameKeyboard_keyCodes;
extern int   GameKeyboard_keyCodes_len;
extern int  *GameKeyboard_hasKeyCodes;

GameKeyboard *GameKeyboard_new(void);
void          GameKeyboard_free(GameKeyboard *self);

void GameKeyboard_reset   (GameKeyboard *self);
void GameKeyboard_keyInput(GameKeyboard *self, int key, int action);
int  GameKeyboard_isPressed(const GameKeyboard *self, int key);
void GameKeyboard_initKeycodes(GameKeyboard *self);

int  GameKeyboard_isWalkForward (const GameKeyboard *self, int sight);
int  GameKeyboard_isWalkBackward(const GameKeyboard *self, int sight);
int  GameKeyboard_isWalkLeft    (const GameKeyboard *self, int sight);
int  GameKeyboard_isWalkRight   (const GameKeyboard *self, int sight);
int  GameKeyboard_isLookLeft    (const GameKeyboard *self, int sight);
int  GameKeyboard_isLookRight   (const GameKeyboard *self, int sight);
int  GameKeyboard_isLookUp      (const GameKeyboard *self, int sight);
int  GameKeyboard_isLookDown    (const GameKeyboard *self, int sight);
int  GameKeyboard_isPlayerShooting(const GameKeyboard *self);
void GameKeyboard_releasePlayerShoot(GameKeyboard *self);

/* Java: static — usam GameKeyboard_keyCodes / hasKeyCodes globais */
int  GameKeyboard_isSightKey        (int key);
int  GameKeyboard_isUseKey          (int key);
int  GameKeyboard_isInventoryKey    (int key);
int  GameKeyboard_isJumpKey         (int key);
int  GameKeyboard_isNextWeaponKey   (int key);
int  GameKeyboard_isPreviousWeaponKey(int key);

#endif
