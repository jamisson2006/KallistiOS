/*
 * game_screen.h — porte fiel de code/Gameplay/GameScreen.java
 */
#ifndef QE_GAMEPLAY_GAME_SCREEN_H
#define QE_GAMEPLAY_GAME_SCREEN_H

#include <stdbool.h>
#include <stdint.h>
#include "../utils/canvas/my_canvas.h"

typedef struct Main             Main;
typedef struct Font             Font;
typedef struct Player           Player;
typedef struct Scene            Scene;
typedef struct GameIni          GameIni;
typedef struct DialogScreen     DialogScreen;
typedef struct WeatherGenerator WeatherGenerator;
typedef struct Graphics         Graphics;
typedef struct Image            Image;
typedef struct Vector3D         Vector3D;
typedef struct HUDInfo          HUDInfo;
typedef struct GameKeyboard     GameKeyboard;
typedef struct DirectX7         DirectX7;

/* Forward for delay dialog vector element */
typedef struct DelayedDialog    DelayedDialog;

typedef struct GameScreen {
    MyCanvas    base;           /* heranca: MyCanvas */

    Main       *main;
    Font       *font;
    Player     *player;
    Scene      *scene;
    GameIni    *levelIni;
    DialogScreen *dialogScreen;
    WeatherGenerator *wg;

    /* Timing */
    int64_t     time;           /* static in Java; per-instance here */
    bool        doubleBright;

    /* Static-like fields */
    char       *mus;
    char       *levelFile;
    int         width;
    int         height;
    bool        bloom;

    /* Delayed dialogs (dynamic array) */
    DelayedDialog **delayDialogs;
    int             delayDialogCount;

    /* Lines (debug/trace vector) */
    int        *lines;
    int         lineCount;

    /* State */
    bool        finishDraw;
    void       *lookAtObj;
    int64_t     musTime;
    int64_t     usmem;

    Vector3D   *newPos;
    int         levelNumber;
    HUDInfo    *hudInfo;
    int        *shopItems;
    int         shopItemCount;

    /* Input */
    GameKeyboard *keys;
    int         key;
    int         pointerX, pointerY;
    int         dirX, dirY;
    bool        run;
    bool        paused;

    /* Timers */
    int         msToEnd;
    int         msToExit;

    /* HUD state cache */
    int         lastHP;
    int         lastAmmo;
    int         lastMoney;
    int         lastFrags;

    /* HUD images */
    Image      *imgSight;
    Image      *imgLife;
    Image      *imgPatron;
    Image      *imgMoney;
    Image      *imgSkull;
    Image      *imgHand;
    Image      *imgLifeLow;
    Image      *imgPatronLow;
    Image      *imgPatronNoWeapon;

    /* Custom message */
    char       *customMessage;
    bool        customMessagePause;
    int64_t     customMessageEndTime;

    /* Overlay effect */
    Image      *overlay;
    int64_t     overlayStart;
    int64_t     overlayTimeOut;

    /* Vignette */
    uint8_t    *vignette;
    int         vignetteSize;

    /* Flags */
    bool        changed;
    bool        firstUpdate;
    bool        fullMoveLvl;
} GameScreen;

/* Lifecycle */
void    GameScreen_init(GameScreen *gs, Main *main);
void    GameScreen_destroy(GameScreen *gs);

/* MyCanvas overrides (vtable entries) */
void    GameScreen_paint(MyCanvas *self, Graphics *g);
void    GameScreen_keyPressed(MyCanvas *self, int keyCode);
void    GameScreen_keyReleased(MyCanvas *self, int keyCode);
void    GameScreen_pointerPressed(MyCanvas *self, int x, int y);
void    GameScreen_pointerDragged(MyCanvas *self, int x, int y);
void    GameScreen_pointerReleased(MyCanvas *self, int x, int y);
void    GameScreen_mouseScrollUp(MyCanvas *self);
void    GameScreen_mouseScrollDown(MyCanvas *self);
void    GameScreen_sizeChanged(MyCanvas *self, int w, int h);

/* Game loop */
void    GameScreen_run(GameScreen *gs);
void    GameScreen_start(GameScreen *gs);
void    GameScreen_stop(GameScreen *gs);

/* Music */
void    GameScreen_startMus(GameScreen *gs);
void    GameScreen_stopMus(GameScreen *gs);
void    GameScreen_destroyMusic(GameScreen *gs);

/* Level management */
void    GameScreen_loadLevel(GameScreen *gs, int levelNumber, bool loadSave, bool loadPos);
void    GameScreen_restartGame(GameScreen *gs);
void    GameScreen_gameEnd(GameScreen *gs, bool win);
void    GameScreen_resize(GameScreen *gs, int w, int h);

/* Dialog/UI */
void    GameScreen_showDialog(GameScreen *gs, const char *text);
void    GameScreen_openShop(GameScreen *gs);
void    GameScreen_openInventory(GameScreen *gs);
void    GameScreen_openPause(GameScreen *gs);

/* Scripting */
void    GameScreen_runScriptFromFile(GameScreen *gs, const char *file);
void    GameScreen_runScript(GameScreen *gs, char **script, int scriptCount);
bool    GameScreen_readBooleanFromScript(GameScreen *gs, const char *expr);

/* Effects */
void    GameScreen_applyVignette(GameScreen *gs, Graphics *g);
void    GameScreen_applySub(GameScreen *gs, Graphics *g, int color, int alpha);
void    GameScreen_damageScr(GameScreen *gs);
void    GameScreen_deathScr(GameScreen *gs);
void    GameScreen_fastbloomScr(GameScreen *gs, Graphics *g);

/* VTable accessor */
const MyCanvasVTable *GameScreen_vt(void);

#endif
