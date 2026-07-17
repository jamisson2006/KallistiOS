/*
 * blood.c — porte fiel de code/AI/misc/Blood.java
 */
#include "blood.h"
#include "../../math/vector3d.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct Sprite   Sprite;
typedef struct Texture  Texture;
typedef struct DirectX7 DirectX7;
typedef struct Matrix   Matrix;

extern Sprite  *Sprite_new_scale(int scale);
extern void     Sprite_destroy(Sprite *spr);
extern void     Sprite_setTextures(Sprite *spr, Texture **texs, int n);
extern void     Sprite_setScale(Sprite *spr, int scale);
extern int      Sprite_getHeight(Sprite *spr);
extern void     Sprite_setOffset(Sprite *spr, int ox, int oy);
extern void     Sprite_project(Sprite *spr, Matrix *invCam, DirectX7 *g3d);
extern Vector3D *Sprite_getPosition(Sprite *spr);
extern void     DirectX7_addRenderObjectNoRect(DirectX7 *g3d, void *obj);
extern Matrix  *DirectX7_getInvCamera(DirectX7 *g3d);
extern void     Vector3D_set(Vector3D *self, int x, int y, int z);

typedef struct RawImage { int alphaMixing; int scale; } RawImage;
extern RawImage *Texture_getRImg(Texture *tex);

extern int  QFPS_frameTime;
extern float Main_bloodscale;

Texture *Blood_texture = NULL;
static int8_t s_mir = 0;

Blood *Blood_new(GameObject *obj) {
    Blood *self = (Blood *)calloc(1, sizeof(Blood));
    self->sprite = Sprite_new_scale(5);
    Sprite_setTextures(self->sprite, &Blood_texture, 1);
    Sprite_setScale(self->sprite, 5);

    /* mode = alphaMixing ? 3 : 0 */
    extern void Sprite_setMode(Sprite *spr, int8_t mode);
    extern void Sprite_setMirX(Sprite *spr, bool m);
    extern void Sprite_setMirY(Sprite *spr, bool m);

    RawImage *ri = Texture_getRImg(Blood_texture);
    Sprite_setMode(self->sprite, ri->alphaMixing ? 3 : 0);
    Sprite_setMirX(self->sprite, s_mir >= 2);
    Sprite_setMirY(self->sprite, s_mir == 1 || s_mir == 2);

    s_mir = (int8_t)((s_mir + 1) % 4);
    self->frame = 255;
    return self;
}

void Blood_destroy(Blood *self) {
    if (!self) return;
    Sprite_destroy(self->sprite);
    self->sprite = NULL;
    free(self);
}

void Blood_reset(Blood *self) {
    Sprite_setScale(self->sprite, 5);
    self->frame = 255;
}

void Blood_bleed(Blood *self) {
    self->frame = 0;
    extern void Sprite_setMirX(Sprite *spr, bool m);
    extern void Sprite_setMirY(Sprite *spr, bool m);
    extern bool Sprite_getMirX(Sprite *spr);
    bool mx = Sprite_getMirX(self->sprite);
    Sprite_setMirX(self->sprite, !mx);
    Sprite_setMirY(self->sprite, !(!mx));
}

void Blood_render(Blood *self, DirectX7 *g3d, int sz, Vector3D *pos) {
    Vector3D *spos = Sprite_getPosition(self->sprite);
    Vector3D_set(spos, pos->x, pos->y, pos->z);
    self->frame += QFPS_frameTime / 5;
    Sprite_setScale(self->sprite, (int)(self->frame / 2 * Main_bloodscale));
    Sprite_setOffset(self->sprite, 0, -Sprite_getHeight(self->sprite) / 2 - self->frame * 4);
    Sprite_project(self->sprite, DirectX7_getInvCamera(g3d), g3d);
    DirectX7_addRenderObjectNoRect(g3d, self->sprite);
    extern int *Sprite_getSzPtr(Sprite *spr);
    *Sprite_getSzPtr(self->sprite) += sz;
}

bool Blood_isBleeding(Blood *self) {
    return self->frame < 70;
}
