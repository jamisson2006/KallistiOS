/*
 * skybox.h — porte fiel de code/Gameplay/Map/Skybox.java
 */
#ifndef QE_GAMEPLAY_MAP_SKYBOX_H
#define QE_GAMEPLAY_MAP_SKYBOX_H

#include <stdbool.h>
#include <stdint.h>

typedef struct Mesh         Mesh;
typedef struct MultyTexture MultyTexture;
typedef struct Texture      Texture;
typedef struct DirectX7     DirectX7;

typedef struct Skybox {
    int mode;  /* 0=mesh, 1=solid color, 2=2D texture scroll, 3=gradient */

    /* Mode 0 — mesh skybox */
    Mesh        *mesh;
    MultyTexture *texture;

    /* Mode 1 — solid color */
    int color;

    /* Mode 2/3 — 2D texture / gradient */
    Texture *tex2d;
    float    repeatX;
    float    repeatY;

    /* Gradient / horizon params */
    int   lowestDegree;
    float horizonScale;
    float horizonOffset;
    int   groundColor;

    /* Sky lighting texture */
    Texture *skyLighting;
    bool     skyboxAlways;
    bool     lighting;
    int64_t  lastLighting;

    /* Rotation */
    int rotateX;
    int rotateY;

    /* Viewport */
    int vpX, vpY, vpW, vpH;
} Skybox;

/* Constructors */
Skybox *Skybox_new_mesh(Mesh *mesh, MultyTexture *texture);
Skybox *Skybox_new_color(int color);
Skybox *Skybox_new_2d(Texture *tex, float repeatX, float repeatY);
Skybox *Skybox_new_gradient(Texture *tex, float repeatX, float repeatY,
                            int lowestDegree, float horizonScale, float horizonOffset,
                            int groundColor);
void    Skybox_destroy(Skybox *self);

/* Render */
void Skybox_render(Skybox *self, DirectX7 *g3d, int rotX, int rotY);

/* Viewport */
void Skybox_addViewport(Skybox *self, int x, int y, int w, int h);

/* Lighting */
void Skybox_setSkyLighting(Skybox *self, Texture *tex);
void Skybox_setSkyboxAlways(Skybox *self, bool always);

#endif
