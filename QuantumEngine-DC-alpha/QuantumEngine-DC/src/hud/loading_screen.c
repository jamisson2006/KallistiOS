/*
 * loading_screen.c — porte fiel de code/HUD/LoadingScreen.java
 */
#include "loading_screen.h"
#include "base/font.h"
#include "../utils/main.h"
#include "../utils/inifile.h"
#include "../utils/canvas/main_canvas.h"

#include <stdlib.h>

/* GameScreen a converter em zip Gameplay */
typedef struct GameScreen GameScreen;
extern GameScreen *GameScreen_new(Main *main, int lvl, void *tmp);
extern void       *GameScreen_player(GameScreen *gs);
extern void       *GameScreen_scene(GameScreen *gs);
extern int         GameScreen_getWidth(GameScreen *gs);
extern int         GameScreen_getHeight(GameScreen *gs);
extern int         GameScreen_levelNumber(GameScreen *gs);
extern void        GameScreen_start(GameScreen *gs);
extern void        GameScreen_copyNewToUsed_player(void *player);
extern void       *Scene_getG3D_gs(void *scene);
extern void        G3D_updateFov(void *g3d, int fov);
extern float       Player_fov_v(void *player);
extern void        Scene_deleteUsedObjects(void *scene, void *player);
extern void        Scene_removeKilledBots(void *scene);

static void LoadingScreen_paint(MyCanvas *mc, Graphics *g) {
    LoadingScreen *self = (LoadingScreen*) mc;
    if (self->frames == 0 && self->showLoad) {
        Font *font = GUIScreen_getFont(&self->base);
        int y = MyCanvas_getHeight(mc) / 2 - Font_height(font) / 2;
        Main_drawBck(g, y - Font_height(font) / 2, y + Font_height(font) / 2);
        Font_drawString(font, g,
            IniFile_getDef(Main_getGameText(), "LOADING_SCREEN", "LOADING_SCREEN"),
            MyCanvas_getWidth(mc) / 2, y, 1 | 2);
    }
    self->frames++;
    if (self->frames < 2) {
        MyCanvas_repaint(mc);
    } else if (self->frames == 2) {
        GameScreen *gs = GameScreen_new(self->main, self->lvl, NULL);
        void *player = GameScreen_player(gs);
        void *scene  = GameScreen_scene(gs);
        if (self->loadSave) {
            Main_loadGame((void*) player, MyCanvas_getWidth(mc), MyCanvas_getHeight(mc), (void*) scene);
            G3D_updateFov(Scene_getG3D_gs(scene), (int) Player_fov_v(player));
            Main_loadObjects((void*) player, GameScreen_getWidth(gs), GameScreen_getHeight(gs), (void*) scene, GameScreen_levelNumber(gs));
            if (self->loadPos) Main_loadPosition((void*) player);
        }
        GameScreen_copyNewToUsed_player(player);
        GameScreen_start(gs);
        Scene_deleteUsedObjects(scene, player);
        Scene_removeKilledBots(scene);
        Main_setCurrent((MyCanvas*) gs);
    }
}

static const GUIScreenVTable g_vt = {
    {
        LoadingScreen_paint,
        GUIScreen_default_keyPressed, GUIScreen_default_keyRepeated, GUIScreen_default_keyReleased,
        GUIScreen_default_pointerPressed, MyCanvas_default_mouseScrollDown, MyCanvas_default_mouseScrollUp,
        MyCanvas_default_pointerReleased, GUIScreen_default_pointerDragged, MyCanvas_default_pointerClicked,
        MyCanvas_default_sizeChanged, MyCanvas_default_showNotify, MyCanvas_default_hideNotify
    },
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

LoadingScreen *LoadingScreen_new(Main *main, int lvl, int loadSave, int loadPos, int showLoad) {
    LoadingScreen *l = (LoadingScreen*) calloc(1, sizeof(LoadingScreen));
    GUIScreen_init(&l->base, &g_vt);
    l->main = main;
    l->lvl = lvl; l->loadSave = loadSave; l->loadPos = loadPos; l->showLoad = showLoad;
    GUIScreen_setFont(&l->base, Main_getFont());
    return l;
}
void LoadingScreen_free(LoadingScreen *l) { if (!l) return; GUIScreen_destroy(&l->base); free(l); }
