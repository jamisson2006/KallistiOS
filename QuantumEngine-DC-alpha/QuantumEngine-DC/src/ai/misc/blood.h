/*
 * blood.h — porte fiel de code/AI/misc/Blood.java
 */
#ifndef QE_AI_MISC_BLOOD_H
#define QE_AI_MISC_BLOOD_H

#include <stdbool.h>

typedef struct Texture  Texture;
typedef struct Sprite   Sprite;
typedef struct DirectX7 DirectX7;
typedef struct Vector3D Vector3D;
typedef struct GameObject GameObject;

typedef struct Blood {
    Sprite *sprite;
    int     frame;
} Blood;

extern Texture *Blood_texture;

Blood *Blood_new(GameObject *obj);
void   Blood_destroy(Blood *self);
void   Blood_reset(Blood *self);
void   Blood_bleed(Blood *self);
void   Blood_render(Blood *self, DirectX7 *g3d, int sz, Vector3D *pos);
bool   Blood_isBleeding(Blood *self);

#endif
