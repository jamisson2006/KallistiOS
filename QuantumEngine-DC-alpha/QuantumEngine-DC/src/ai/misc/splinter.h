/*
 * splinter.h — porte fiel de code/AI/misc/Splinter.java
 */
#ifndef QE_AI_MISC_SPLINTER_H
#define QE_AI_MISC_SPLINTER_H

#include <stdbool.h>

typedef struct Texture  Texture;
typedef struct Sprite   Sprite;
typedef struct DirectX7 DirectX7;

typedef struct Splinter {
    int frame;
} Splinter;

extern Texture *Splinter_texture;
extern Sprite  *Splinter_sprite;

void     Splinter_cache(void);
Splinter *Splinter_new(void);
void     Splinter_set(Splinter *self, int x, int y, int z);
void     Splinter_project(Splinter *self, DirectX7 *g3d);
void     Splinter_render(Splinter *self, DirectX7 *g3d, int sz);
bool     Splinter_isShatters(Splinter *self);

#endif
