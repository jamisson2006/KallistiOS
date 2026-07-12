/*
 * splinter.c — porte fiel de code/AI/misc/Splinter.java
 */
#include "splinter.h"
#include <stdlib.h>
#include <limits.h>

typedef struct Sprite   Sprite;
typedef struct Texture  Texture;
typedef struct DirectX7 DirectX7;
typedef struct Vector3D Vector3D;
typedef struct Matrix   Matrix;
typedef struct RawImage { int alphaMixing; } RawImage;

extern Sprite  *Sprite_new_scale(int scale);
extern void     Sprite_destroy(Sprite *spr);
extern void     Sprite_setTextures(Sprite *spr, Texture **texs, int n);
extern void     Sprite_setScale(Sprite *spr, int scale);
extern int      Sprite_getHeight(Sprite *spr);
extern void     Sprite_setOffset(Sprite *spr, int ox, int oy);
extern void     Sprite_project(Sprite *spr, Matrix *invCam, DirectX7 *g3d);
extern Vector3D *Sprite_getPosition(Sprite *spr);
extern void     Sprite_setMode(Sprite *spr, int8_t mode);
extern void     Sprite_setMirX(Sprite *spr, bool m);
extern void     Sprite_setMirY(Sprite *spr, bool m);
extern int      Sprite_isVisible(Sprite *spr, int x1, int y1, int x2, int y2);
extern void     DirectX7_addRenderObjectNoRect(DirectX7 *g3d, void *obj);
extern Matrix  *DirectX7_getInvCamera(DirectX7 *g3d);
extern int      DirectX7_getWidth(DirectX7 *g3d);
extern int      DirectX7_getHeight(DirectX7 *g3d);
extern void     Vector3D_set(Vector3D *self, int x, int y, int z);
extern RawImage *Texture_getRImg(Texture *tex);
extern int  *Sprite_getSzPtr(Sprite *spr);

extern int   QFPS_frameTime;
extern float Main_splinterscale;

Texture *Splinter_texture = NULL;
Sprite  *Splinter_sprite = NULL;
static int8_t s_mir = 0;

void Splinter_cache(void) {
    if (Splinter_sprite == NULL) Splinter_sprite = Sprite_new_scale(5);
    Sprite_setTextures(Splinter_sprite, &Splinter_texture, 1);
    Sprite_setScale(Splinter_sprite, 5);
    Sprite_setMode(Splinter_sprite, 3);
    RawImage *ri = Texture_getRImg(Splinter_texture);
    if (!ri->alphaMixing) Sprite_setMode(Splinter_sprite, 0);
}

Splinter *Splinter_new(void) {
    Splinter *self = (Splinter *)calloc(1, sizeof(Splinter));
    self->frame = INT32_MAX;
    return self;
}

void Splinter_set(Splinter *self, int x, int y, int z) {
    Vector3D *pos = Sprite_getPosition(Splinter_sprite);
    Vector3D_set(pos, x, y, z);
    self->frame = 0;
    Sprite_setMirX(Splinter_sprite, false);
    Sprite_setMirY(Splinter_sprite, false);
    if (s_mir >= 2) Sprite_setMirX(Splinter_sprite, true);
    if (s_mir == 1 || s_mir == 2) Sprite_setMirY(Splinter_sprite, true);
    s_mir++;
    if (s_mir > 3) s_mir = 0;
}

void Splinter_project(Splinter *self, DirectX7 *g3d) {
    Sprite_setScale(Splinter_sprite, (int)(self->frame / 2 * Main_splinterscale));
    Sprite_setOffset(Splinter_sprite, 0, -Sprite_getHeight(Splinter_sprite) / 2 - self->frame * 4);
    Sprite_project(Splinter_sprite, DirectX7_getInvCamera(g3d), g3d);
    Sprite_isVisible(Splinter_sprite, 0, 0, DirectX7_getWidth(g3d), DirectX7_getHeight(g3d));
}

void Splinter_render(Splinter *self, DirectX7 *g3d, int sz) {
    self->frame += QFPS_frameTime / 5;
    Splinter_project(self, g3d);
    DirectX7_addRenderObjectNoRect(g3d, Splinter_sprite);
    *Sprite_getSzPtr(Splinter_sprite) += 1500;
}

bool Splinter_isShatters(Splinter *self) {
    return self->frame < 30;
}
