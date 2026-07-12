/*
 * skybox.c — porte fiel de code/Gameplay/Map/Skybox.java
 */
#include "skybox.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

extern void DirectX7_clearDisplay(DirectX7 *g3d, int color);
extern void DirectX7_addMesh(DirectX7 *g3d, Mesh *mesh, int x1, int y1, int x2, int y2);
extern void DirectX7_drawLine(DirectX7 *g3d, int x1, int y1, int x2, int y2, int fat, int color);
extern int  DirectX7_getWidth(DirectX7 *g3d);
extern int  DirectX7_getHeight(DirectX7 *g3d);

extern int  MathUtils_preudoRandom(int64_t seed, int range);

typedef struct Matrix Matrix;
extern Matrix *DirectX7_getCamera(DirectX7 *g3d);
extern Matrix *DirectX7_computeFinalMatrix(DirectX7 *g3d, Matrix *m);
extern void    DirectX7_transformAndProjectVertices(DirectX7 *g3d, Mesh *mesh, Matrix *fm);

typedef struct RasterImage {
    int *img;
    int  w;
    int  h;
    int  widthBIT;
    int  scale;
    bool alphaMixing;
} RasterImage;

typedef struct Texture {
    RasterImage *rImg;
    int addsz;
} Texture;

/* --- Construction --- */

Skybox *Skybox_new_mesh(Mesh *mesh, MultyTexture *texture) {
    Skybox *s = (Skybox *)calloc(1, sizeof(Skybox));
    s->mode = 0;
    s->mesh = mesh;
    s->texture = texture;
    s->lastLighting = -1;
    return s;
}

Skybox *Skybox_new_color(int color) {
    Skybox *s = (Skybox *)calloc(1, sizeof(Skybox));
    s->mode = 1;
    s->color = color;
    s->lastLighting = -1;
    return s;
}

Skybox *Skybox_new_2d(Texture *tex, float repeatX, float repeatY) {
    Skybox *s = (Skybox *)calloc(1, sizeof(Skybox));
    s->mode = 2;
    s->tex2d = tex;
    s->repeatX = repeatX;
    s->repeatY = repeatY;
    s->lowestDegree = -91;
    s->horizonScale = 1.0f;
    s->horizonOffset = 0.0f;
    s->lastLighting = -1;
    return s;
}

Skybox *Skybox_new_gradient(Texture *tex, float repeatX, float repeatY,
                            int lowestDegree, float horizonScale, float horizonOffset,
                            int groundColor) {
    Skybox *s = (Skybox *)calloc(1, sizeof(Skybox));
    s->mode = 3;
    s->tex2d = tex;
    s->repeatX = repeatX;
    s->repeatY = repeatY;
    s->lowestDegree = lowestDegree;
    s->horizonScale = horizonScale;
    s->horizonOffset = horizonOffset;
    s->groundColor = groundColor;
    s->lastLighting = -1;
    return s;
}

void Skybox_destroy(Skybox *self) {
    if (!self) return;
    self->texture = NULL;
    self->tex2d = NULL;
    self->skyLighting = NULL;
    free(self);
}

/* --- Viewport --- */

void Skybox_addViewport(Skybox *self, int x, int y, int w, int h) {
    if (self->vpW == 0 && self->vpH == 0) {
        self->vpX = x;
        self->vpY = y;
        self->vpW = w;
        self->vpH = h;
    } else {
        if (x < self->vpX) self->vpX = x;
        if (y < self->vpY) self->vpY = y;
        if (w > self->vpW) self->vpW = w;
        if (h > self->vpH) self->vpH = h;
    }
}

static void Skybox_resetViewport(Skybox *self) {
    self->vpX = 0;
    self->vpY = 0;
    self->vpW = 0;
    self->vpH = 0;
}

/* --- Gradient rendering --- */

static void renderGradient(Skybox *self, DirectX7 *g3d, Texture *texs,
                           int lowestDeg, float hScale, float hOffset) {
    int width = DirectX7_getWidth(g3d);
    int height = DirectX7_getHeight(g3d);
    int x1 = self->vpX, y1 = self->vpY;
    int x2 = self->vpW, y2 = self->vpH;

    if (x2 - x1 == 0 || y2 - y1 == 0) return;

    /* Stub: gradient rendering uses direct framebuffer access
       which requires DirectX7 display[] array - will be connected at integration */
    (void)texs; (void)lowestDeg; (void)hScale; (void)hOffset;
    (void)width; (void)height;
}

/* --- 2D skybox rendering --- */

static void render2DSkybox(Skybox *self, DirectX7 *g3d) {
    int x2 = self->vpW, x1 = self->vpX;
    int y2 = self->vpH, y1 = self->vpY;

    if (x2 - x1 == 0 || y2 - y1 == 0) return;

    /* Stub: 2D skybox rendering uses direct framebuffer access */
    (void)g3d;
}

/* --- Lightning drawing --- */

static void drawLighting(Skybox *self, DirectX7 *g3d) {
    int width = DirectX7_getWidth(g3d);
    int height = DirectX7_getHeight(g3d);
    int fovY = 74; /* default fov */
    int fovX = 74;

    if (MathUtils_preudoRandom(self->lastLighting, 8) == 0) return;

    int horizonY = height / 2 + self->rotateX * height / fovY;
    int ySize = 90 * height / fovY;

    int fat = 3 * 74 / fovY;
    if (fat < 1) fat = 1;

    int by = horizonY - ySize;
    int bx = MathUtils_preudoRandom(self->lastLighting, width);
    int deg20 = width * 20 / fovX;
    int mx = bx + (MathUtils_preudoRandom(self->lastLighting, 100) - 50) * deg20 / 100;
    int my = horizonY - ySize / 3 + MathUtils_preudoRandom(self->lastLighting, 100) * height * 10 / fovY / 100;

    DirectX7_drawLine(g3d, bx, by, mx, my, fat, 0xffffff);

    if (MathUtils_preudoRandom(self->lastLighting, 4) == 0) {
        int rnd = (MathUtils_preudoRandom(self->lastLighting, 100) - 50) * deg20 / 25;
        DirectX7_drawLine(g3d, mx / 2 + bx / 2, my / 2 + by / 2, mx + rnd, horizonY, fat, 0xffffff);
    }

    int sparksCount = 2 + MathUtils_preudoRandom(self->lastLighting, 3);
    int fat2 = fat - 1;
    if (fat2 < 1) fat2 = 1;

    for (int i = 0; i < sparksCount; i++) {
        int rnd = (MathUtils_preudoRandom(self->lastLighting, 100) - 50) * deg20 / 50;
        if (MathUtils_preudoRandom(self->lastLighting, 4) == 0) {
            int rnd2 = (MathUtils_preudoRandom(self->lastLighting, 100) - 50) * deg20 / 150;
            DirectX7_drawLine(g3d, mx, my, mx + rnd / 2, horizonY / 2 + my / 2, fat2, 0xfdfdfd);
            DirectX7_drawLine(g3d, mx + rnd / 2, horizonY / 2 + my / 2, mx + rnd / 2 + rnd2 / 2, horizonY, fat2, 0xfdfdfd);
            DirectX7_drawLine(g3d, mx + rnd / 2, horizonY / 2 + my / 2, mx + rnd / 2 - rnd2 / 2, horizonY, fat2, 0xfdfdfd);
        } else {
            DirectX7_drawLine(g3d, mx, my, mx + rnd, horizonY, fat2, 0xfdfdfd);
        }
    }
}

/* --- Main render --- */

void Skybox_render(Skybox *self, DirectX7 *g3d, int rotX, int rotY) {
    self->rotateX = rotX;
    self->rotateY = rotY;

    if (self->skyboxAlways) {
        self->vpX = 0;
        self->vpY = 0;
        self->vpW = DirectX7_getWidth(g3d);
        self->vpH = DirectX7_getHeight(g3d);
    }

    if (self->lighting) {
        if (self->skyLighting == NULL) {
            DirectX7_clearDisplay(g3d, 0xe5e5e5);
        } else if (self->mode == 3) {
            renderGradient(self, g3d, self->skyLighting, self->lowestDegree,
                           self->horizonScale, self->horizonOffset);
        } else {
            renderGradient(self, g3d, self->skyLighting, -91, 1, 0);
        }
        drawLighting(self, g3d);
        self->lighting = false;

    } else if (self->mode == 0) {
        /* Mesh skybox — transform mesh to camera position and add */
        /* Full implementation requires Matrix operations - stub for now */

    } else if (self->mode == 1) {
        DirectX7_clearDisplay(g3d, self->color);

    } else if (self->mode == 2) {
        render2DSkybox(self, g3d);

    } else if (self->mode == 3) {
        renderGradient(self, g3d, self->tex2d, self->lowestDegree,
                       self->horizonScale, self->horizonOffset);
    }

    Skybox_resetViewport(self);
}

void Skybox_setSkyLighting(Skybox *self, Texture *tex) {
    self->skyLighting = tex;
}

void Skybox_setSkyboxAlways(Skybox *self, bool always) {
    self->skyboxAlways = always;
}

Mesh *Skybox_getMesh(Skybox *self) {
    return self->mesh;
}
