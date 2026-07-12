/*
 * directx7.c — porte fiel de code/Rendering/DirectX7.java
 */
#include "directx7.h"
#include "vertex.h"
#include "renderobjectbuffer.h"
#include "tmpelement.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* externals: HUD/DeveloperMenu, Meshes/Mesh */
extern int DeveloperMenu_renderPolygonsOverwrite;
extern Vertex **Mesh_vertices(Mesh *m, int *out_n);
extern void  *Mesh_polygons(Mesh *m, int *out_n);
extern void  *Mesh_getTexture(Mesh *m);

extern void RenderObject_render(RenderObject *obj, DirectX7 *g3d, Texture *tex);

extern void  RenderObjectBuffer_addRenderObject      (RenderObjectBuffer *b, RenderObject *obj, int x1, int y1, int x2, int y2);
extern void  RenderObjectBuffer_addRenderObjectTex   (RenderObjectBuffer *b, RenderObject *obj, Texture *t, int x1, int y1, int x2, int y2);
extern void  RenderObjectBuffer_addRenderObjectDT    (RenderObjectBuffer *b, RenderObject *obj, Texture *t, int x1, int y1, int x2, int y2);
extern void  RenderObjectBuffer_addRenderObjects     (RenderObjectBuffer *b, void *polys, int npolys, void *tex, int x1, int y1, int x2, int y2);
extern void  RenderObjectBuffer_sort                 (RenderObjectBuffer *b, int lo, int hi);
extern int   RenderObjectBuffer_getSize              (RenderObjectBuffer *b);
extern TMPElement **RenderObjectBuffer_getBuffer     (RenderObjectBuffer *b);
extern void  RenderObjectBuffer_reset                (RenderObjectBuffer *b);
extern void  RenderObjectBuffer_resetTex             (RenderObjectBuffer *b);
extern RenderObjectBuffer *RenderObjectBuffer_new(void);
extern void  RenderObjectBuffer_free(RenderObjectBuffer *b);

extern void Graphics_drawRGB(Graphics *g, int *display, int off, int stride,
                             int x, int y, int w, int h, int alpha);

/* --- static state --- */
int    DirectX7_fDist  = 1;
int    DirectX7_drDist = 30000000;
int    DirectX7_waterDistance = 10000;
int8_t DirectX7_standartDrawmode = 0;
int    DirectX7_fogc  = 0x5500ff;
int16_t DirectX7_lightdirx = 0, DirectX7_lightdiry = 0, DirectX7_lightdirz = 0;
int    DirectX7_useAutoWMove = 1;

DirectX7 *DirectX7_new(int width, int height) {
    DirectX7 *g = (DirectX7*) calloc(1, sizeof(DirectX7));
    g->width = width; g->height = height;
    g->camera      = Matrix_new();
    g->invCamera   = Matrix_new();
    g->finalCamera = Matrix_new();
    g->buffer      = RenderObjectBuffer_new();
    DirectX7_resize(g, width, height);
    g->lx = g->ly = g->lz = 0;
    return g;
}
void DirectX7_free(DirectX7 *g) {
    if (!g) return;
    free(g->display);
    Matrix_free(g->camera);
    Matrix_free(g->invCamera);
    Matrix_free(g->finalCamera);
    RenderObjectBuffer_free(g->buffer);
    free(g);
}

void DirectX7_resize(DirectX7 *self, int width, int height) {
    self->width = width; self->height = height;
    free(self->display);
    self->display = (int*) calloc(width * height, sizeof(int));
    self->centreX = width / 2;
    self->centreY = height / 2;
    DirectX7_updateFov(self, 74);
}

void DirectX7_updateFov(DirectX7 *self, int fov) {
    double t = tan(fov * 3.14159265358979323846 / 360.0);
    self->distX = (int)(self->centreX / t);
    self->distY = (int)(self->centreY / t);
    self->distX = self->distX * self->height / self->width;
    self->fovY = fov;
    self->fovX = fov * self->width / self->height;
}

void DirectX7_destroy(DirectX7 *self) {
    DirectX7_fDist = 1;
    DirectX7_drDist = 30000000;
    DirectX7_standartDrawmode = 0;
    DirectX7_fogc = 0x5500ff;
    /* Java: camera=invCamera=finalCamera=null; buffer.resetTex(); */
    Matrix_free(self->camera); self->camera = NULL;
    Matrix_free(self->invCamera); self->invCamera = NULL;
    Matrix_free(self->finalCamera); self->finalCamera = NULL;
    RenderObjectBuffer_resetTex(self->buffer);
}

int  DirectX7_getWidth(const DirectX7 *self)  { return self->width; }
int  DirectX7_getHeight(const DirectX7 *self) { return self->height; }
int *DirectX7_getDisplay(const DirectX7 *self){ return self->display; }
int  DirectX7_fovY(const DirectX7 *self)      { return self->fovY; }
int  DirectX7_centreY(const DirectX7 *self)   { return self->centreY; }
int  DirectX7_distY(const DirectX7 *self)     { return self->distY; }
int  DirectX7_height(const DirectX7 *self)    { return self->height; }

void DirectX7_setCamera(DirectX7 *self, const Matrix *matrix) {
    Matrix_set_m(self->invCamera, matrix);
    Matrix_set_m(self->camera,    matrix);
    Matrix_invert(self->invCamera);
}
Matrix *DirectX7_getInvCamera(DirectX7 *self) { return self->invCamera; }
Matrix *DirectX7_getCamera   (DirectX7 *self) { return self->camera; }

Matrix *DirectX7_computeFinalMatrix(DirectX7 *self, const Matrix *matrix) {
    Matrix_mul2(self->finalCamera, self->invCamera, matrix);
    return self->finalCamera;
}

void DirectX7_addRenderObject(DirectX7 *self, RenderObject *obj) {
    DirectX7_addRenderObjectRect(self, obj, 0, 0, self->width, self->height);
}
void DirectX7_addRenderObjectRect(DirectX7 *self, RenderObject *obj, int x1, int y1, int x2, int y2) {
    RenderObjectBuffer_addRenderObject(self->buffer, obj, x1, y1, x2, y2);
}
void DirectX7_addRenderObjectTex(DirectX7 *self, RenderObject *obj, Texture *tex) {
    RenderObjectBuffer_addRenderObjectTex(self->buffer, obj, tex, 0, 0, self->width, self->height);
}
void DirectX7_addRenderObjectDT(DirectX7 *self, RenderObject *obj, Texture *tex) {
    RenderObjectBuffer_addRenderObjectDT(self->buffer, obj, tex, 0, 0, self->width, self->height);
}

void DirectX7_addMesh(DirectX7 *self, Mesh *mesh, int x1, int y1, int x2, int y2) {
    int np; void *polys = Mesh_polygons(mesh, &np);
    RenderObjectBuffer_addRenderObjects(self->buffer, polys, np, Mesh_getTexture(mesh), x1, y1, x2, y2);
}
void DirectX7_addMeshMT(DirectX7 *self, Mesh *mesh, int x1, int y1, int x2, int y2, MultyTexture *tex) {
    int np; void *polys = Mesh_polygons(mesh, &np);
    RenderObjectBuffer_addRenderObjects(self->buffer, polys, np, tex, x1, y1, x2, y2);
}

void DirectX7_render(DirectX7 *self) {
    self->lx = self->camera->m03;
    self->ly = self->camera->m13;
    self->lz = self->camera->m23;
    RenderObjectBuffer_sort(self->buffer, 0, RenderObjectBuffer_getSize(self->buffer) - 1);
    if (DeveloperMenu_renderPolygonsOverwrite) DirectX7_clearDisplay(self, 0);
    TMPElement **buf = RenderObjectBuffer_getBuffer(self->buffer);
    int i = RenderObjectBuffer_getSize(self->buffer) - 1;

    /* Java: unroll de 5 em 5 */
    for (; i >= 5; i -= 5) {
        RenderObject_render(buf[i  ]->obj, self, buf[i  ]->tex);
        RenderObject_render(buf[i-1]->obj, self, buf[i-1]->tex);
        RenderObject_render(buf[i-2]->obj, self, buf[i-2]->tex);
        RenderObject_render(buf[i-3]->obj, self, buf[i-3]->tex);
        RenderObject_render(buf[i-4]->obj, self, buf[i-4]->tex);
    }
    for (; i >= 0; i--) RenderObject_render(buf[i]->obj, self, buf[i]->tex);

    RenderObjectBuffer_reset(self->buffer);
}

void DirectX7_drawLine(DirectX7 *self, int x1, int y1, int x2, int y2, int fat, int col) {
    int tmp;
    if (y1 > y2) { tmp=y1; y1=y2; y2=tmp; tmp=x1; x1=x2; x2=tmp; }
    if ((x1 > self->width && x2 > self->width) || y1 > self->height ||
        (x2 < 0 && x1 < 0) || y2 < 0) return;

    int ySize = y2 - y1;
    int halfFat = fat / 2;
    if (fat == 1) halfFat = 0;

    int y = y1; if (y < 0) y = 0;
    int yEnd = y2; if (yEnd > self->height) yEnd = self->height;

    for (; y < yEnd; y++) {
        int perc = y - y1;
        int x = (x1 * (ySize - perc) + x2 * perc) / ySize;
        int t = y * self->width;
        int xx1 = x - halfFat, xx2 = xx1 + fat;
        if (xx1 > self->width || xx2 < 0) continue;
        if (xx1 < 0) xx1 = 0;
        if (xx2 >= self->width) xx2 = self->width - 1;
        xx1 += t; xx2 += t;
        while (xx1 < xx2) { self->display[xx1++] = col; }
    }
}

void DirectX7_flush(DirectX7 *self, Graphics *g, int x, int y) {
    if (DeveloperMenu_renderPolygonsOverwrite) {
        int len = self->width * self->height;
        for (int i = len - 1; i >= 0; i--) {
            int c = self->display[i];
            if      (c == 0) self->display[i] = 0xffffff;
            else if (c == 1) self->display[i] = 0x00ff00;
            else if (c == 2) self->display[i] = 0xffff00;
            else if (c == 3) self->display[i] = 0xff8800;
            else if (c == 4) self->display[i] = 0xff0000;
            else if (c == 5) self->display[i] = 0x880000;
        }
    }
    Graphics_drawRGB(g, self->display, 0, self->width, x, y, self->width, self->height, self->flushAlpha);
}

void DirectX7_clearDisplay(DirectX7 *self, int col) {
    int length = self->width * self->height;
    int i = 0;
    for (; length - i > 5; i += 5) {
        self->display[i]   = col;
        self->display[i+1] = col;
        self->display[i+2] = col;
        self->display[i+3] = col;
        self->display[i+4] = col;
    }
    for (; i < length; i++) self->display[i] = col;
}

/* --- transforms --- */

void DirectX7_transformMesh(Mesh *ms, const Matrix *m) {
    int n; Vertex **vers = Mesh_vertices(ms, &n);
    DirectX7_transformVerts(vers, n, m);
}

void DirectX7_transformVerts(Vertex **vers, int n, const Matrix *m) {
    if (m == NULL) {
        for (int i = n - 1; i >= 0; i--) {
            Vertex *v = vers[i];
            v->sx = v->x; v->sy = v->y; v->rz = v->z;
        }
        return;
    }
    const int m00 = m->m00 >> 2, m01 = m->m01 >> 2, m02 = m->m02 >> 2, m03 = m->m03;
    const int m10 = m->m10 >> 2, m11 = m->m11 >> 2, m12 = m->m12 >> 2, m13 = m->m13;
    const int m20 = m->m20 >> 2, m21 = m->m21 >> 2, m22 = m->m22 >> 2, m23 = m->m23;
    const int fp = QE_MATRIX_FP - 2;

    for (int i = n - 1; i >= 0; i--) {
        Vertex *v = vers[i];
        int x = v->x, y = v->y, z = v->z;
        v->sx = (x*m00 >> fp) + (y*m01 >> fp) + (z*m02 >> fp) + m03;
        v->sy = (x*m10 >> fp) + (y*m11 >> fp) + (z*m12 >> fp) + m13;
        v->rz = (x*m20 >> fp) + (y*m21 >> fp) + (z*m22 >> fp) + m23;
    }
}

void DirectX7_project(DirectX7 *self, Vertex **vertices, int n, int rz) {
    for (int i = n - 1; i >= 0; i--) {
        Vertex *v = vertices[i];
        int sx = v->sx, sy = -v->sy;
        if (rz <= 0) {
            sx = sx * self->distX / (rz + self->distX);
            sy = sy * self->distY / (rz + self->distY);
        }
        v->sx = (int16_t)(sx + self->centreX);
        v->sy = (int16_t)(sy + self->centreY);
    }
}

void DirectX7_returnMesh(Mesh *ms) {
    int n; Vertex **vers = Mesh_vertices(ms, &n);
    for (int i = n - 1; i >= 0; i--) {
        Vertex *v = vers[i];
        v->x = v->sx; v->y = v->sy; v->z = v->rz;
    }
}

void DirectX7_transformSave(Vertex **vers, int n, const Matrix *m) {
    const int m00 = m->m00 >> 2, m01 = m->m01 >> 2, m02 = m->m02 >> 2, m03 = m->m03;
    const int m10 = m->m10 >> 2, m11 = m->m11 >> 2, m12 = m->m12 >> 2, m13 = m->m13;
    const int m20 = m->m20 >> 2, m21 = m->m21 >> 2, m22 = m->m22 >> 2, m23 = m->m23;
    const int fp = QE_MATRIX_FP - 2;

    for (int i = n - 1; i >= 0; i--) {
        Vertex *v = vers[i];
        int x = v->x, y = v->y, z = v->z;
        v->sx = x; v->sy = y; v->rz = z;
        v->x = (x*m00 >> fp) + (y*m01 >> fp) + (z*m02 >> fp) + m03;
        v->y = (x*m10 >> fp) + (y*m11 >> fp) + (z*m12 >> fp) + m13;
        v->z = (x*m20 >> fp) + (y*m21 >> fp) + (z*m22 >> fp) + m23;
    }
}

void DirectX7_transformReturn(Vertex **vers, int n) {
    for (int i = n - 1; i >= 0; i--) {
        Vertex *v = vers[i];
        int x = v->x, y = v->y, z = v->z;
        v->sx = x; v->sy = y; v->rz = z;
        v->x = v->sx; v->y = v->sy; v->z = v->rz;
    }
}

/* Fiel ao original: usa comparacoes m00!=0 etc. + inverte sy (com -m13). */
static void qe_tap_impl(DirectX7 *self, Vertex **vertices, int n, const Matrix *matrix) {
    const int m00 = matrix->m00 >> 2, m01 = matrix->m01 >> 2, m02 = matrix->m02 >> 2, m03 = matrix->m03;
    const int m10 = matrix->m10 >> 2, m11 = matrix->m11 >> 2, m12 = matrix->m12 >> 2, m13 = matrix->m13;
    const int m20 = matrix->m20 >> 2, m21 = matrix->m21 >> 2, m22 = matrix->m22 >> 2, m23 = matrix->m23;
    const int fp = QE_MATRIX_FP - 2;

    for (int i = n - 1; i >= 0; i--) {
        Vertex *v = vertices[i];
        int x = v->x, y = v->y, z = v->z;
        int sx, sy, rz;

        sx = m03;
        if (m00 != 0) sx += (x * m00 >> fp);
        if (m01 != 0) sx += (y * m01 >> fp);
        if (m02 != 0) sx += (z * m02 >> fp);

        sy = -m13;
        if (m10 != 0) sy -= (x * m10 >> fp);
        if (m11 != 0) sy -= (y * m11 >> fp);
        if (m12 != 0) sy -= (z * m12 >> fp);

        rz = m23;
        if (m20 != 0) rz += (x * m20 >> fp);
        if (m21 != 0) rz += (y * m21 >> fp);
        if (m22 != 0) rz += (z * m22 >> fp);

        if (rz < 0) {
            sx = sx * self->distX / (-rz + self->distX);
            sy = sy * self->distY / (-rz + self->distY);
        }
        v->sx = sx + self->centreX;
        v->sy = sy + self->centreY;
        v->rz = rz;
    }
}

void DirectX7_transformAndProjectVertices(DirectX7 *self, Mesh *ms, const Matrix *m) {
    int n; Vertex **vers = Mesh_vertices(ms, &n);
    qe_tap_impl(self, vers, n, m);
}
void DirectX7_transformAndProjectVerticesArr(DirectX7 *self, Vertex **vs, int n, const Matrix *m) {
    qe_tap_impl(self, vs, n, m);
}

void DirectX7_setFogDist(int d) { DirectX7_fDist  = d; }
void DirectX7_setDrDist (int d) { DirectX7_drDist = d; }
