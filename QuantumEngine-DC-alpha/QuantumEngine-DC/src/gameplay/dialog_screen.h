/*
 * dialog_screen.h — porte fiel de code/Gameplay/DialogScreen.java
 */
#ifndef QE_GAMEPLAY_DIALOG_SCREEN_H
#define QE_GAMEPLAY_DIALOG_SCREEN_H

#include <stdbool.h>
#include <stdint.h>

typedef struct DirectX7    DirectX7;
typedef struct GameScreen  GameScreen;
typedef struct Font        Font;
typedef struct Graphics    Graphics;
typedef struct Image       Image;

typedef struct DialogScreen {
    DirectX7   *g3d;
    GameScreen *gs;

    char      **dialog;
    int         dialogCount;
    int         index;

    Image      *bckList[10];
    Image      *avatar;

    bool scrollDown;
    bool scrollUp;

    /* ItemList for questions */
    char      **questionItems;
    int         questionCount;
    int         questionIndex;
    bool        itemListHasCaption;
    int        *answersGoIndex;
    int         answersGoCount;
} DialogScreen;

DialogScreen *DialogScreen_new(const char *text, Font *font, DirectX7 *g3d, GameScreen *gs);
void          DialogScreen_destroy(DialogScreen *self);
void          DialogScreen_set(DialogScreen *self, const char *text, Font *font,
                               DirectX7 *g3d, GameScreen *gs);
void          DialogScreen_paint(DialogScreen *self, Graphics *g);
void          DialogScreen_keyPressed(DialogScreen *self, int keyCode);
void          DialogScreen_keyReleased(DialogScreen *self, int keyCode);
void          DialogScreen_pointerPressed(DialogScreen *self, int x, int y);
void          DialogScreen_pointerReleased(DialogScreen *self, int x, int y);

char *DialogScreen_loadTextFromFile(const char *text);

#endif
