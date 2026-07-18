/*
 * keyboard.c — porte fiel de code/utils/Keyboard.java (Quantum Engine J2ME)
 */
#include "keyboard.h"
#include "fps.h"

#include <stdlib.h>
#include <string.h>

Keyboard *Keyboard_new(qe_isSelect_fn is_select, void *canvas_ctx) {
    Keyboard *k = (Keyboard*) calloc(1, sizeof(Keyboard));

    if (is_select && is_select(canvas_ctx, -26)) {          /* Siemens */
        k->LEFT = -61; k->RIGHT = -62; k->DOWN = -60; k->UP = -59;
        k->FIRE = -26; k->SOFT_LEFT = -1; k->SOFT_RIGHT = -4;
    } else if (is_select && is_select(canvas_ctx, -20)) {   /* Motorola */
        k->LEFT = -2;  k->RIGHT = -5;  k->DOWN = -6;  k->UP = -1;
        k->FIRE = -20; k->SOFT_LEFT = -21; k->SOFT_RIGHT = -22;
    } else {                                                /* default (Nokia, SE) */
        k->LEFT = -3;  k->RIGHT = -4;  k->DOWN = -2;  k->UP = -1;
        k->FIRE = -5;  k->SOFT_LEFT = -6; k->SOFT_RIGHT = -7;
    }

    Keyboard_reset(k);
    return k;
}

void Keyboard_free(Keyboard *self) { free(self); }

void Keyboard_reset(Keyboard *self) {
    self->up = self->down = self->left = self->right = 0;
    self->key1 = self->key3 = self->key7 = self->key9 = 0;
    self->star = self->pound = self->fire = 0;
    /* Java nao zera key0 no reset — deixamos fiel. */
}

void Keyboard_keyPressed(Keyboard *self, int keyCode)  { Keyboard_keyInput(self, keyCode, 1); }
void Keyboard_keyReleased(Keyboard *self, int keyCode) { Keyboard_keyInput(self, keyCode, 0); }

void Keyboard_keyInput(Keyboard *self, int keyCode, int pressed) {
    /* A cadeia de if/else-if do Java tem uma peculiaridade: os dois primeiros
     * ("UP" e "DOWN") NAO fazem else-if, e o restante faz. Preservamos
     * exatamente esse fluxo — inclusive o fato de UP e DOWN poderem disparar
     * ambos em teoria (embora as constantes sejam distintas). */
    if (keyCode == self->UP   || keyCode == QE_KEY_NUM2) { self->up = pressed;   self->upAction = FPS_currentTime; }
    if (keyCode == self->DOWN || keyCode == QE_KEY_NUM8) { self->down = pressed; self->downAction = FPS_currentTime; }
    else if (keyCode == self->LEFT  || keyCode == QE_KEY_NUM4) { self->left = pressed;  self->leftAction = FPS_currentTime; }
    else if (keyCode == self->RIGHT || keyCode == QE_KEY_NUM6) { self->right = pressed; self->rightAction = FPS_currentTime; }
    else if (keyCode == self->FIRE  || keyCode == QE_KEY_NUM5) { self->fire = pressed;  self->okAction = FPS_currentTime; }

    else if (keyCode == QE_KEY_NUM1) self->key1 = pressed;
    else if (keyCode == QE_KEY_NUM3) self->key3 = pressed;
    else if (keyCode == QE_KEY_NUM7) self->key7 = pressed;
    else if (keyCode == QE_KEY_NUM9) self->key9 = pressed;
    else if (keyCode == QE_KEY_NUM0) self->key0 = pressed;
    else if (keyCode == QE_KEY_STAR) self->star = pressed;
    else if (keyCode == QE_KEY_POUND) self->pound = pressed;
}
