/*
 * benchmark.c — porte fiel de code/HUD/Benchmark.java
 */
#include "benchmark.h"
#include "menu.h"
#include "../rendering/directx7.h"
#include "../rendering/texture.h"
#include "../rendering/vertex.h"
#include "../rendering/texturing_affine.h"
#include "../rendering/texturing_pers.h"
#include "../utils/main.h"
#include "../utils/canvas/main_canvas.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _arch_dreamcast
#include <arch/timer.h>
static inline long long qe_now_ms(void) { return (long long) timer_ms_gettime64(); }
#else
#include <sys/time.h>
static inline long long qe_now_ms(void) {
    struct timeval tv; gettimeofday(&tv,0);
    return (long long) tv.tv_sec * 1000LL + tv.tv_usec / 1000;
}
#endif

extern void Graphics_setColor(Graphics *g, int c);
extern void Graphics_translate(Graphics *g, int x, int y);
extern void Graphics_drawString(Graphics *g, const char *str, int x, int y, int anchor);

static void test_bench(Benchmark *self, int tp) {
    int count = tp ? 200 : 2000;

    long long bTime = qe_now_ms();
    for (int i = 0; i < count; i++) {
        TexturingPers_paint(self->g3d, self->tex,
            self->a, 0, 0,
            self->b, 0xff, 0,
            self->c, 0, 0xff, 0x000000, self->rendermode, 0, 24,
            128, 192, 128, 128, 128, 192, 128, 128, 128, 128, 128, 128);
    }
    self->PersTime = qe_now_ms() - bTime;

    bTime = qe_now_ms();
    for (int i = 0; i < count; i++) {
        TexturingPers_paint(self->g3d, self->tex,
            self->a, 0, 0,
            self->b, 0xff, 0,
            self->c, 0, 0xff, 0x000000, self->rendermode, 0, 9999,
            128, 192, 128, 128, 128, 192, 128, 128, 128, 0, 4096, 0);
    }
    self->PersTimeFloor = qe_now_ms() - bTime;

    bTime = qe_now_ms();
    for (int i = 0; i < count; i++) {
        TexturingAffine_paint(self->g3d, self->tex,
            self->a, 0, 0,
            self->b, 0xff, 0,
            self->c, 0, 0xff, 0x000000, self->rendermode, 0,
            128, 192, 128, 128, 128, 192, 128, 128, 128, 128, 128, 128);
    }
    self->AffTime = qe_now_ms() - bTime;
}

static void Benchmark_paint(MyCanvas *mc, Graphics *g) {
    Benchmark *self = (Benchmark*) mc;
    Menu_drawBackground(self->menu, g);
    Graphics_setColor(g, 0xffffff);
    Graphics_translate(g, MyCanvas_getWidth(mc) / 2, MyCanvas_getHeight(mc) / 2);
    char buf[128];
    snprintf(buf, sizeof buf, "Render Mode: %d", self->rendermode);         Graphics_drawString(g, buf, 0, -50, 64 | 1);
    snprintf(buf, sizeof buf, "Affine Time: %lld ms", self->AffTime);       Graphics_drawString(g, buf, 0, -20, 64 | 1);
    snprintf(buf, sizeof buf, "Perspective Time: %lld ms", self->PersTime); Graphics_drawString(g, buf, 0,  20, 64 | 1);
    snprintf(buf, sizeof buf, "Perspective Time Floors: %lld ms", self->PersTimeFloor); Graphics_drawString(g, buf, 0, 60, 64 | 1);
    Graphics_translate(g, -MyCanvas_getWidth(mc) / 2, -MyCanvas_getHeight(mc) / 2);
    GUIScreen_drawSoftKeys(&self->base, g);
}

static void Benchmark_onKey5(GUIScreen *gs) {
    Benchmark *self = (Benchmark*) gs;
    self->a->sy = 0;   self->a->sx = 0;   self->a->rz = 0;
    self->b->sy = 0;   self->b->sx = 256; self->b->rz = -100;
    self->c->sy = 256; self->c->sx = 0;   self->c->rz = -25;
    test_bench(self, 1);
    MyCanvas_repaint(&gs->base);
}
static void Benchmark_onLeftSoftKey(GUIScreen *gs) {
    Benchmark *self = (Benchmark*) gs;
    self->rendermode++;
    if (self->rendermode > 13) self->rendermode = 0;
    Benchmark_onKey5(gs);
}
static void Benchmark_onRightSoftKey(GUIScreen *gs) {
    Benchmark *self = (Benchmark*) gs;
    DirectX7_destroy(self->g3d);
    self->a = self->b = self->c = NULL;
    Main_setCurrent((MyCanvas*) self->menu);
}
static void Benchmark_onKey2(GUIScreen *gs) { MyCanvas_repaint(&gs->base); }
static void Benchmark_onKey8(GUIScreen *gs) { MyCanvas_repaint(&gs->base); }

static const GUIScreenVTable g_vt = {
    {
        Benchmark_paint,
        GUIScreen_default_keyPressed, GUIScreen_default_keyRepeated, GUIScreen_default_keyReleased,
        GUIScreen_default_pointerPressed, MyCanvas_default_mouseScrollDown, MyCanvas_default_mouseScrollUp,
        MyCanvas_default_pointerReleased, GUIScreen_default_pointerDragged, MyCanvas_default_pointerClicked,
        MyCanvas_default_sizeChanged, MyCanvas_default_showNotify, MyCanvas_default_hideNotify
    },
    Benchmark_onLeftSoftKey, Benchmark_onRightSoftKey,
    Benchmark_onKey5, NULL, NULL,
    Benchmark_onKey2, Benchmark_onKey8, NULL, NULL,
    NULL, NULL, NULL, NULL
};

Benchmark *Benchmark_new(Main *main, Menu *menu) {
    Benchmark *b = (Benchmark*) calloc(1, sizeof(Benchmark));
    GUIScreen_init(&b->base, &g_vt);
    b->main = main; b->menu = menu;
    GUIScreen_setFont(&b->base, Main_getFont());
    GUIScreen_setSoftKeysNames(&b->base, "Change Render Mode", "Back");
    b->g3d = DirectX7_new(256, 256);
    b->tex = Texture_createTexture("/images/icon.png");
    b->a = Vertex_new_xyz(0, 0, 0); b->a->sy = 0;   b->a->sx = 0;   b->a->rz = 0;
    b->b = Vertex_new_xyz(0, 0, 0); b->b->sy = 0;   b->b->sx = 255; b->b->rz = -100;
    b->c = Vertex_new_xyz(0, 0, 0); b->c->sy = 255; b->c->sx = 0;   b->c->rz = -25;
    test_bench(b, 1);
    return b;
}
void Benchmark_free(Benchmark *b) { if (!b) return; GUIScreen_destroy(&b->base); free(b); }
