/*
 * directx7.h — porte fiel de code/Rendering/DirectX7.java
 */
#ifndef QE_RENDERING_DIRECTX7_H
#define QE_RENDERING_DIRECTX7_H

#include <stdint.h>
#include "../math/matrix.h"

typedef struct Vertex Vertex;
typedef struct Texture Texture;
typedef struct MultyTexture MultyTexture;
typedef struct RenderObject RenderObject;
typedef struct RenderObjectBuffer RenderObjectBuffer;
typedef struct Mesh Mesh;
typedef struct Graphics Graphics;

typedef struct DirectX7 {
    int width, height;
    int *display;
    int  flushAlpha;
    Matrix *camera, *invCamera, *finalCamera;
    int centreX, centreY;
    int distX, distY;
    int fovX, fovY;

    RenderObjectBuffer *buffer;
    int lx, ly, lz;
    int shootIntensity, shootLength;
    int flashlightEnabled;
} DirectX7;

/* Static fields (Java: public static) */
extern int   DirectX7_fDist;
extern int   DirectX7_drDist;
extern int   DirectX7_waterDistance;
extern int8_t DirectX7_standartDrawmode;
extern int   DirectX7_fogc;
extern int16_t DirectX7_lightdirx, DirectX7_lightdiry, DirectX7_lightdirz;
extern int   DirectX7_useAutoWMove;

DirectX7 *DirectX7_new(int width, int height);
void      DirectX7_free(DirectX7 *self);

void      DirectX7_resize(DirectX7 *self, int width, int height);
void      DirectX7_updateFov(DirectX7 *self, int fov);
void      DirectX7_destroy(DirectX7 *self);

int       DirectX7_getWidth(const DirectX7 *self);
int       DirectX7_getHeight(const DirectX7 *self);
int      *DirectX7_getDisplay(const DirectX7 *self);
int       DirectX7_fovY(const DirectX7 *self);
int       DirectX7_centreY(const DirectX7 *self);
int       DirectX7_distY(const DirectX7 *self);
int       DirectX7_height(const DirectX7 *self);

void      DirectX7_setCamera(DirectX7 *self, const Matrix *matrix);
Matrix   *DirectX7_getInvCamera(DirectX7 *self);
Matrix   *DirectX7_getCamera(DirectX7 *self);
Matrix   *DirectX7_computeFinalMatrix(DirectX7 *self, const Matrix *matrix);

void      DirectX7_addRenderObject     (DirectX7 *self, RenderObject *obj);
void      DirectX7_addRenderObjectRect (DirectX7 *self, RenderObject *obj, int x1, int y1, int x2, int y2);
void      DirectX7_addRenderObjectTex  (DirectX7 *self, RenderObject *obj, Texture *tex);
void      DirectX7_addRenderObjectDT   (DirectX7 *self, RenderObject *obj, Texture *tex);

void      DirectX7_addMesh   (DirectX7 *self, Mesh *mesh, int x1, int y1, int x2, int y2);
void      DirectX7_addMeshMT (DirectX7 *self, Mesh *mesh, int x1, int y1, int x2, int y2, MultyTexture *tex);

void      DirectX7_render(DirectX7 *self);
void      DirectX7_drawLine(DirectX7 *self, int x1, int y1, int x2, int y2, int fat, int col);
void      DirectX7_flush   (DirectX7 *self, Graphics *g, int x, int y);
void      DirectX7_clearDisplay(DirectX7 *self, int col);

/* Static transforms */
void      DirectX7_transformMesh(Mesh *ms, const Matrix *m);
void      DirectX7_transformVerts(Vertex **vers, int n, const Matrix *m);

void      DirectX7_project(DirectX7 *self, Vertex **vertices, int n, int rz);

void      DirectX7_returnMesh    (Mesh *ms);
void      DirectX7_transformSave (Vertex **vers, int n, const Matrix *m);
void      DirectX7_transformReturn(Vertex **vers, int n);

void      DirectX7_transformAndProjectVertices    (DirectX7 *self, Mesh *ms, const Matrix *m);
void      DirectX7_transformAndProjectVerticesArr (DirectX7 *self, Vertex **vertices, int n, const Matrix *m);

void      DirectX7_setFogDist(int d);
void      DirectX7_setDrDist (int d);

#endif
