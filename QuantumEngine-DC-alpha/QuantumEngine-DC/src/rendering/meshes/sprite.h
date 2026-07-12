/*
 * sprite.h — porte fiel de code/Rendering/Meshes/Sprite.java
 *
 * Sprite = billboard 2D projetado no espaco 3D. Suporta 5+ modos de blend
 * (normal, ADD, MUL, OPAQUE, com fog), 7 cores diferentes de tint (branco,
 * vermelho, verde, azul, violeta, amarelo, aqua), animacao por frame.
 */
#ifndef QE_RENDERING_MESHES_SPRITE_H
#define QE_RENDERING_MESHES_SPRITE_H

#include "../renderobject.h"
#include <stdint.h>

typedef struct Texture  Texture;
typedef struct DirectX7 DirectX7;
typedef struct Matrix   Matrix;
typedef struct Vertex   Vertex;

typedef struct Sprite {
    RenderObject base;
    Texture **textures;   int textures_n;
    Vertex   *pos;
    Vertex   *size;
    int       mirX, mirY;
    int       scale;
    int       offsetX, offsetY;
    int8_t    mode;      /* 0=normal, 1=ADD, 2=MUL, 3=OPAQUE */
    int8_t    cutoff;
    int       anim_index;
    int       fog;
    int64_t   animationBegin;
    float     animation_speed;
    int       limiter;
    int       color;     /* 0=white,1=red,2=green,3=blue,4=violet,5=yellow,6=aqua */
} Sprite;

Sprite *Sprite_new(int dummy);
Sprite *Sprite_new_multi(Texture **texs, int nt, int sc, float animsp);
Sprite *Sprite_new_single(Texture *tex, int sc);
void    Sprite_free(Sprite *self);

void    Sprite_setMode  (Sprite *self, const char *tmp);
void    Sprite_setFog   (Sprite *self, const char *tmp);
void    Sprite_setScale (Sprite *self, int scale);
void    Sprite_setOffset(Sprite *self, int x, int y);
Vertex *Sprite_getPosition(Sprite *self);
void    Sprite_setTextures(Sprite *self, Texture **textures, int n);

int     Sprite_getHeight(Sprite *self);
int     Sprite_getWidth (Sprite *self);

void    Sprite_project  (Sprite *self, DirectX7 *g3d);
void    Sprite_projectM (Sprite *self, const Matrix *matrix, DirectX7 *g3d);

void    Sprite_updateFrame(Sprite *self);

const RenderObjectVTable *Sprite_vt(void);

#endif
