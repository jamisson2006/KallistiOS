/*
 * game_screen.c — porte fiel de code/Gameplay/GameScreen.java
 */
#include "game_screen.h"
#include "scene.h"
#include "weapon.h"
#include "arsenal.h"
#include "../utils/main.h"
#include "../utils/gameini.h"
#include "../utils/delayed_dialog.h"
#include "../utils/weather_generator.h"
#include "../rendering/directx7.h"
#include "../hud/base/hud_info.h"
#include "../hud/base/game_keyboard.h"
#include <stdlib.h>
#include <string.h>

/* Extern dependencies */
extern int64_t qe_current_ms(void);
extern void Player_update(void *player, void *scene, int frameTime);
extern void Player_render(void *player, void *g3d);
extern int  Player_getHP(void *player);
extern int  Player_getMoney(void *player);
extern int  Player_getFrags(void *player);

/* Static vtable */
static const MyCanvasVTable gs_vtable = {
    .paint           = GameScreen_paint,
    .keyPressed      = GameScreen_keyPressed,
    .keyRepeated     = MyCanvas_default_keyRepeated,
    .keyReleased     = GameScreen_keyReleased,
    .pointerPressed  = GameScreen_pointerPressed,
    .mouseScrollDown = GameScreen_mouseScrollDown,
    .mouseScrollUp   = GameScreen_mouseScrollUp,
    .pointerReleased = GameScreen_pointerReleased,
    .pointerDragged  = GameScreen_pointerDragged,
    .pointerClicked  = MyCanvas_default_pointerClicked,
    .sizeChanged     = GameScreen_sizeChanged,
    .showNotify      = MyCanvas_default_showNotify,
    .hideNotify      = MyCanvas_default_hideNotify,
};

const MyCanvasVTable *GameScreen_vt(void) {
    return &gs_vtable;
}

void GameScreen_init(GameScreen *gs, Main *main) {
    memset(gs, 0, sizeof(GameScreen));
    MyCanvas_init(&gs->base, &gs_vtable);

    gs->main = main;
    gs->time = qe_current_ms();
    gs->firstUpdate = true;
    gs->width = MyCanvas_getWidth(&gs->base);
    gs->height = MyCanvas_getHeight(&gs->base);
}

void GameScreen_destroy(GameScreen *gs) {
    if (gs->scene) {
        Scene_destroy(gs->scene);
        gs->scene = NULL;
    }
    if (gs->delayDialogs) {
        for (int i = 0; i < gs->delayDialogCount; i++) {
            DelayedDialog_free(gs->delayDialogs[i]);
        }
        free(gs->delayDialogs);
        gs->delayDialogs = NULL;
        gs->delayDialogCount = 0;
    }
    if (gs->lines) { free(gs->lines); gs->lines = NULL; }
    if (gs->mus) { free(gs->mus); gs->mus = NULL; }
    if (gs->levelFile) { free(gs->levelFile); gs->levelFile = NULL; }
    if (gs->customMessage) { free(gs->customMessage); gs->customMessage = NULL; }
    if (gs->vignette) { free(gs->vignette); gs->vignette = NULL; }
    if (gs->shopItems) { free(gs->shopItems); gs->shopItems = NULL; }
    if (gs->newPos) { free(gs->newPos); gs->newPos = NULL; }

    GameScreen_destroyMusic(gs);
}

void GameScreen_paint(MyCanvas *self, Graphics *g) {
    GameScreen *gs = (GameScreen *)self;

    if (gs->paused) return;

    int64_t now = qe_current_ms();
    int frameTime = (int)(now - gs->time);
    gs->time = now;

    /* Clamp frame time to avoid huge jumps */
    if (frameTime > 200) frameTime = 200;
    if (frameTime < 1) frameTime = 1;

    /* Update game logic */
    if (gs->scene) {
        Scene_update(gs->scene, gs->player, gs);
    }

    /* Update player */
    if (gs->player) {
        Player_update(gs->player, gs->scene, frameTime);
    }

    /* Update weather */
    if (gs->wg) {
        WeatherGenerator_update(gs->wg, frameTime, 0, 0);
    }

    /* Update delayed dialogs */
    for (int i = 0; i < gs->delayDialogCount; i++) {
        DelayedDialog_update(gs->delayDialogs[i], gs);
    }

    /* Render scene */
    if (gs->scene) {
        DirectX7 *g3d = Scene_getG3D(gs->scene);

        /* Render world */
        Scene_render(gs->scene, g, 0, gs->scene->part, gs->player);

        /* Render player (weapon) */
        if (gs->player) {
            Player_render(gs->player, g3d);
        }

        Scene_flush(gs->scene);

        /* Weather overlay */
        if (gs->wg) {
            WeatherGenerator_paint(gs->wg, g, 0);
        }
    }

    /* Bloom effect */
    if (gs->bloom) {
        GameScreen_fastbloomScr(gs, g);
    }

    /* Vignette */
    if (gs->vignette) {
        GameScreen_applyVignette(gs, g);
    }

    /* Overlay */
    if (gs->overlay && gs->overlayTimeOut > 0) {
        int64_t elapsed = now - gs->overlayStart;
        if (elapsed > gs->overlayTimeOut) {
            gs->overlay = NULL;
            gs->overlayTimeOut = 0;
        }
    }

    /* Custom message */
    if (gs->customMessage) {
        if (!gs->customMessagePause && now > gs->customMessageEndTime) {
            free(gs->customMessage);
            gs->customMessage = NULL;
        }
    }

    /* HUD drawing handled by HUD module */

    /* End timer checks */
    if (gs->msToEnd > 0) {
        gs->msToEnd -= frameTime;
        if (gs->msToEnd <= 0) {
            GameScreen_gameEnd(gs, false);
        }
    }
    if (gs->msToExit > 0) {
        gs->msToExit -= frameTime;
        if (gs->msToExit <= 0) {
            GameScreen_gameEnd(gs, true);
        }
    }

    gs->firstUpdate = false;
}

void GameScreen_keyPressed(MyCanvas *self, int keyCode) {
    GameScreen *gs = (GameScreen *)self;
    gs->key = keyCode;

    /* Forward to game keyboard handler */
    if (gs->keys) {
        /* GameKeyboard_keyPressed(gs->keys, keyCode) — extern */
    }
}

void GameScreen_keyReleased(MyCanvas *self, int keyCode) {
    GameScreen *gs = (GameScreen *)self;

    if (gs->keys) {
        /* GameKeyboard_keyReleased(gs->keys, keyCode) — extern */
    }
    (void)keyCode;
}

void GameScreen_pointerPressed(MyCanvas *self, int x, int y) {
    GameScreen *gs = (GameScreen *)self;
    gs->pointerX = x;
    gs->pointerY = y;
    gs->dirX = 0;
    gs->dirY = 0;
}

void GameScreen_pointerDragged(MyCanvas *self, int x, int y) {
    GameScreen *gs = (GameScreen *)self;
    gs->dirX = x - gs->pointerX;
    gs->dirY = y - gs->pointerY;
    gs->pointerX = x;
    gs->pointerY = y;
}

void GameScreen_pointerReleased(MyCanvas *self, int x, int y) {
    GameScreen *gs = (GameScreen *)self;
    gs->dirX = 0;
    gs->dirY = 0;
    (void)x;
    (void)y;
}

void GameScreen_mouseScrollUp(MyCanvas *self) {
    GameScreen *gs = (GameScreen *)self;
    /* Switch to previous weapon */
    if (gs->player) {
        /* Arsenal_previous via player — extern */
    }
}

void GameScreen_mouseScrollDown(MyCanvas *self) {
    GameScreen *gs = (GameScreen *)self;
    /* Switch to next weapon */
    if (gs->player) {
        /* Arsenal_next via player — extern */
    }
}

void GameScreen_sizeChanged(MyCanvas *self, int w, int h) {
    GameScreen *gs = (GameScreen *)self;
    gs->width = w;
    gs->height = h;
    GameScreen_resize(gs, w, h);
}

void GameScreen_run(GameScreen *gs) {
    /* Main game loop tick — called by MainCanvas run loop */
    MyCanvas_repaint(&gs->base);
}

void GameScreen_start(GameScreen *gs) {
    gs->paused = false;
    gs->time = qe_current_ms();
    GameScreen_startMus(gs);
}

void GameScreen_stop(GameScreen *gs) {
    gs->paused = true;
    GameScreen_stopMus(gs);
}

void GameScreen_startMus(GameScreen *gs) {
    if (gs->mus && Main_isMusic) {
        /* Sound_playMusic(gs->mus) — extern */
        gs->musTime = qe_current_ms();
    }
}

void GameScreen_stopMus(GameScreen *gs) {
    if (Main_isMusic) {
        /* Sound_stopMusic() — extern */
    }
    (void)gs;
}

void GameScreen_destroyMusic(GameScreen *gs) {
    GameScreen_stopMus(gs);
    if (gs->mus) {
        free(gs->mus);
        gs->mus = NULL;
    }
}

void GameScreen_loadLevel(GameScreen *gs, int levelNumber, bool loadSave, bool loadPos) {
    gs->levelNumber = levelNumber;
    gs->firstUpdate = true;
    gs->changed = true;

    /* Level loading delegated to LevelLoader_createScene */
    /* Scene assigned after creation */
    (void)loadSave;
    (void)loadPos;
}

void GameScreen_restartGame(GameScreen *gs) {
    if (gs->scene) {
        Scene_reset(gs->scene);
    }
    gs->msToEnd = 0;
    gs->msToExit = 0;
    gs->time = qe_current_ms();
    gs->firstUpdate = true;
}

void GameScreen_gameEnd(GameScreen *gs, bool win) {
    gs->paused = true;
    GameScreen_stopMus(gs);
    /* Transition to end screen / next level */
    (void)win;
}

void GameScreen_resize(GameScreen *gs, int w, int h) {
    gs->width = w;
    gs->height = h;

    if (gs->scene) {
        DirectX7 *g3d = Scene_getG3D(gs->scene);
        if (g3d) {
            DirectX7_resize(g3d, w, h);
        }
    }
}

void GameScreen_showDialog(GameScreen *gs, const char *text) {
    if (gs->dialogScreen) {
        /* DialogScreen_set(gs->dialogScreen, text) — extern */
    }
    (void)text;
}

void GameScreen_openShop(GameScreen *gs) {
    /* Create and display shop screen */
    (void)gs;
}

void GameScreen_openInventory(GameScreen *gs) {
    /* Create and display inventory screen */
    (void)gs;
}

void GameScreen_openPause(GameScreen *gs) {
    gs->paused = true;
    /* Display pause screen */
}

void GameScreen_runScriptFromFile(GameScreen *gs, const char *file) {
    /* Load script from file, then execute */
    (void)gs;
    (void)file;
}

void GameScreen_runScript(GameScreen *gs, char **script, int scriptCount) {
    if (gs->scene) {
        Scene_runScript(gs->scene, script, scriptCount, gs->player, gs);
    }
}

bool GameScreen_readBooleanFromScript(GameScreen *gs, const char *expr) {
    if (!expr) return false;
    if (gs->scene) {
        House *house = Scene_getHouse(gs->scene);
        return RoomObject_readBoolean(expr, gs->player, house, gs);
    }
    return false;
}

void GameScreen_applyVignette(GameScreen *gs, Graphics *g) {
    /* Apply vignette overlay from gs->vignette buffer to screen edges */
    (void)gs;
    (void)g;
}

void GameScreen_applySub(GameScreen *gs, Graphics *g, int color, int alpha) {
    /* Apply subtractive color blend over screen */
    (void)gs;
    (void)g;
    (void)color;
    (void)alpha;
}

void GameScreen_damageScr(GameScreen *gs) {
    /* Flash red overlay on damage */
    gs->overlayStart = qe_current_ms();
    gs->overlayTimeOut = 300;
    /* overlay = red tint image */
}

void GameScreen_deathScr(GameScreen *gs) {
    /* Death effect: fade to black/red */
    gs->overlayStart = qe_current_ms();
    gs->overlayTimeOut = 2000;
}

void GameScreen_fastbloomScr(GameScreen *gs, Graphics *g) {
    /* Fast bloom post-processing effect */
    /* Simplified: brighten pixels above threshold */
    (void)gs;
    (void)g;
}
