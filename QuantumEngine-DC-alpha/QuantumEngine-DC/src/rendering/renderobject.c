/*
 * renderobject.c — porte fiel de code/Rendering/RenderObject.java
 */
#include "renderobject.h"
#include "vertex.h"
#include "directx7.h"

#include "../math/math_utils.h"
#include <math.h>

/* Externo — LightMapper (Gameplay/Map) */
extern int LightMapper_cameraVectorLight;

void RenderObject_init(RenderObject *self, const RenderObjectVTable *vt) {
    self->vt = vt;
    self->nx = self->ny = self->nz = 0;
    self->sz = 0;
}

void RenderObject_initWithNormals(RenderObject *self, const RenderObjectVTable *vt,
                                  Vertex *v1, Vertex *v2, Vertex *v3) {
    self->vt = vt;
    self->sz = 0;
    RenderObject_calculateNormals(self, v1, v2, v3);
}

void RenderObject_calculateNormals(RenderObject *self, Vertex *v1, Vertex *v2, Vertex *v3) {
    long x = (long)(v1->y - v2->y) * (v1->z - v3->z) - (long)(v1->z - v2->z) * (v1->y - v3->y);
    long y = (long)(v1->z - v2->z) * (v1->x - v3->x) - (long)(v1->x - v2->x) * (v1->z - v3->z);
    long z = (long)(v1->x - v2->x) * (v1->y - v3->y) - (long)(v1->y - v2->y) * (v1->x - v3->x);
    double sqrt_ = sqrt((double)(x*x + y*y + z*z)) / QE_RENDER_NORMAL_FP_ONE;
    if (sqrt_ == 0.0) sqrt_ = 1.0;
    self->nx = (short)((double)x / sqrt_);
    self->ny = (short)((double)y / sqrt_);
    self->nz = (short)((double)z / sqrt_);
}

void RenderObject_calculateNormalsProjected(RenderObject *self, Vertex *v1, Vertex *v2, Vertex *v3) {
    long x = (long)(v1->sy - v2->sy) * (v1->rz - v3->rz) - (long)(v1->rz - v2->rz) * (v1->sy - v3->sy);
    long y = (long)(v1->rz - v2->rz) * (v1->sx - v3->sx) - (long)(v1->sx - v2->sx) * (v1->rz - v3->rz);
    long z = (long)(v1->sx - v2->sx) * (v1->sy - v3->sy) - (long)(v1->sy - v2->sy) * (v1->sx - v3->sx);
    double sqrt_ = sqrt((double)(x*x + y*y + z*z)) / QE_RENDER_NORMAL_FP_ONE;
    if (sqrt_ == 0.0) sqrt_ = 1.0;
    self->nx = (short)((double)x / sqrt_);
    self->ny = (short)((double)y / sqrt_);
    self->nz = (short)((double)z / sqrt_);
}

void RenderObject_render    (RenderObject *self, DirectX7 *g3d, Texture *tex) { self->vt->render    (self, g3d, tex); }
void RenderObject_renderFast(RenderObject *self, DirectX7 *g3d, Texture *tex) { self->vt->renderFast(self, g3d, tex); }
int  RenderObject_isVisible (RenderObject *self, int x1, int y1, int x2, int y2) { return self->vt->isVisible(self, x1, y1, x2, y2); }

int RenderObject_getLight(RenderObject *self, Vertex *a, int la, DirectX7 *g3d) {
    int lightLen = g3d->flashlightEnabled ? 6500 : 5000;
    int lita = (-a->rz < DirectX7_fDist || DirectX7_fDist <= 1) ? 255 : 0;
    int fa = 0, vec = 255;

    if ((DirectX7_fDist > 1 && LightMapper_cameraVectorLight)
        || ((g3d->shootIntensity > 0 || g3d->flashlightEnabled) && -a->rz < lightLen)) {
        vec = MathUtils_microCalcLight(self->nx, self->ny, self->nz,
            a->x - g3d->camera->m03, a->y - g3d->camera->m13, a->z - g3d->camera->m23);
    }

    if (DirectX7_fDist > 1 && -a->rz < DirectX7_fDist) {
        lita = 0xff + a->rz * 0xFF / DirectX7_fDist;
        if (LightMapper_cameraVectorLight) lita = lita * vec / 255;
        if (lita < 0) lita = 0;
        else if (lita > 0xFF) lita = 0xFF;
    }
    if ((g3d->shootIntensity > 0 || g3d->flashlightEnabled) && -a->rz < lightLen) {
        fa = (0xff + a->rz * 0xFF / lightLen) * vec / 255;
        if (!g3d->flashlightEnabled) fa = fa * g3d->shootIntensity / g3d->shootLength;
        if (fa < 0) fa = 0;
        else if (fa > 0xFF) return 0xff;
    }

    int la2 = ((la * lita) >> 8) + fa;
    if (la2 > 255) return 255;
    return la2;
}

void RenderObject_getLight3(RenderObject *self, Vertex *a, int la, int la2, int la3, DirectX7 *g3d, int *out) {
    int lightLen = g3d->flashlightEnabled ? 6500 : 5000;
    int lita = (-a->rz < DirectX7_fDist || DirectX7_fDist <= 1) ? 255 : 0;
    int fa = 0, vec = 255;

    if ((DirectX7_fDist > 1 && LightMapper_cameraVectorLight)
        || ((g3d->shootIntensity > 0 || g3d->flashlightEnabled) && -a->rz < lightLen)) {
        vec = MathUtils_microCalcLight(self->nx, self->ny, self->nz,
            a->x - g3d->camera->m03, a->y - g3d->camera->m13, a->z - g3d->camera->m23);
    }

    if (DirectX7_fDist > 1 && -a->rz < DirectX7_fDist) {
        lita = 0xff + a->rz * 0xFF / DirectX7_fDist;
        if (LightMapper_cameraVectorLight) lita = lita * vec / 255;
        if (lita < 0) lita = 0;
        else if (lita > 0xFF) lita = 0xFF;
    }
    if ((g3d->shootIntensity > 0 || g3d->flashlightEnabled) && -a->rz < lightLen) {
        fa = (0xff + a->rz * 0xFF / lightLen) * vec / 255;
        if (!g3d->flashlightEnabled) fa = fa * g3d->shootIntensity / g3d->shootLength;
        if (fa < 0) fa = 0;
        else if (fa > 0xFF) fa = 0xff;
    }

    out[0] = (la  * lita) / 255 + fa; if (out[0] > 255) out[0] = 255;
    out[1] = (la2 * lita) / 255 + fa; if (out[1] > 255) out[1] = 255;
    out[2] = (la3 * lita) / 255 + fa; if (out[2] > 255) out[2] = 255;
}
