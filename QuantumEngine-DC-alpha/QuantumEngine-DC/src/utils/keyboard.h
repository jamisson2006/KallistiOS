/*
 * keyboard.h — porte fiel de code/utils/Keyboard.java (Quantum Engine J2ME)
 * @author Roman Lahin
 *
 * No Java as constantes (LEFT/RIGHT/...) eram detectadas via Canvas.getKeyName
 * do dispositivo. No Dreamcast fixamos o mapeamento no default Nokia/SE (nunca
 * havera Motorola/Siemens); getKeyName generico via NULL — o binding real
 * do controller vive em src/core/input (a converter). Este arquivo eh apenas
 * a mesma tabela + estado on/off que o Java expunha.
 */
#ifndef QE_UTILS_KEYBOARD_H
#define QE_UTILS_KEYBOARD_H

#include <stdint.h>

/* Constantes KEY_NUM* do Java Canvas (mesmos valores). */
#define QE_KEY_NUM0 48
#define QE_KEY_NUM1 49
#define QE_KEY_NUM2 50
#define QE_KEY_NUM3 51
#define QE_KEY_NUM4 52
#define QE_KEY_NUM5 53
#define QE_KEY_NUM6 54
#define QE_KEY_NUM7 55
#define QE_KEY_NUM8 56
#define QE_KEY_NUM9 57
#define QE_KEY_STAR 42
#define QE_KEY_POUND 35

typedef int (*qe_isSelect_fn)(void *canvas_ctx, int keycode);

typedef struct Keyboard {
    /* Constantes (Java: final int) — determinadas no construtor. */
    int LEFT, RIGHT, UP, DOWN, FIRE, SOFT_LEFT, SOFT_RIGHT;

    /* Estados de tecla */
    int up, down, left, right, key1, key3, key7, key9, star, pound, key0, fire;

    /* Ultimo momento de acao (usado para deteccao de repeat). */
    int64_t upAction, downAction, leftAction, rightAction, okAction;
} Keyboard;

/* Ctor: detecta layout usando o callback isSelect (retorno != 0 quando o
 * nome da tecla contem "SELECT"). Pode passar NULL — cai no default
 * Nokia/SE, como no Dreamcast. */
Keyboard *Keyboard_new(qe_isSelect_fn is_select, void *canvas_ctx);
void      Keyboard_free(Keyboard *self);

void      Keyboard_reset(Keyboard *self);
void      Keyboard_keyPressed(Keyboard *self, int keyCode);
void      Keyboard_keyReleased(Keyboard *self, int keyCode);
void      Keyboard_keyInput(Keyboard *self, int keyCode, int pressed);

#endif
