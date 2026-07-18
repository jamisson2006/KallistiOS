/*
 * camera.c — porte fiel de code/Rendering/Camera.java
 */
#include "camera.h"
#include "../math/matrix.h"
#include "../math/math_utils2.h"

#include <math.h>
#include <stdlib.h>

/* Gameplay/Map/House */
extern int House_calcPart(House *h, int oldPart, int cx, int cy, int cz);

static float qe_absf(float a) { return a < 0 ? -a : a; }

Camera *Camera_new(void) {
    Camera *c = (Camera*) calloc(1, sizeof(Camera));
    c->tmpPos = Matrix_new();
    c->tmp    = Matrix_new();
    c->tmp2   = Matrix_new();
    c->camera = Matrix_new();

    c->y = 1400; c->x = 500; c->z = 1000;
    c->rotX = 0; c->rotY = 0;
    c->smoothSteps = 8;
    c->oldPart = -1; c->part = -1;
    c->fset = 1;
    return c;
}
void Camera_free(Camera *c) {
    if (!c) return;
    Matrix_free(c->tmpPos); Matrix_free(c->tmp);
    Matrix_free(c->tmp2);   Matrix_free(c->camera);
    free(c);
}

static float invLength_i(int x, int y, int z) {
    return MathUtils2_invSqrt((float)(x*x + y*y + z*z));
}

static void interpolation(Camera *self, Matrix *m1, const Matrix *m2) {
    int ss = self->smoothSteps;
    m1->m00 += (m2->m00 - m1->m00) / ss;
    m1->m01 += (m2->m01 - m1->m01) / ss;
    m1->m02 += (m2->m02 - m1->m02) / ss;

    m1->m10 += (m2->m10 - m1->m10) / ss;
    m1->m11 += (m2->m11 - m1->m11) / ss;
    m1->m12 += (m2->m12 - m1->m12) / ss;

    m1->m20 += (m2->m20 - m1->m20) / ss;
    m1->m21 += (m2->m21 - m1->m21) / ss;
    m1->m22 += (m2->m22 - m1->m22) / ss;

    m1->m03 = m2->m03; m1->m13 = m2->m13; m1->m23 = m2->m23;

    float l = invLength_i(m1->m00, m1->m01, m1->m02) * QE_MATRIX_FP_ONE;
    m1->m00 = (int)(m1->m00 * l);
    m1->m01 = (int)(m1->m01 * l);
    m1->m02 = (int)(m1->m02 * l);

    l = invLength_i(m1->m10, m1->m11, m1->m12) * QE_MATRIX_FP_ONE;
    m1->m10 = (int)(m1->m10 * l);
    m1->m11 = (int)(m1->m11 * l);
    m1->m12 = (int)(m1->m12 * l);

    l = invLength_i(m1->m20, m1->m21, m1->m22) * QE_MATRIX_FP_ONE;
    m1->m20 = (int)(m1->m20 * l);
    m1->m21 = (int)(m1->m21 * l);
    m1->m22 = (int)(m1->m22 * l);
}

void Camera_set(Camera *self, const Matrix *pos, float newRotX, float newRotY,
                int absolutePos, int absoluteRot) {
    if (absoluteRot) {
        Matrix_setIdentity(self->tmp);
        newRotX = 0; newRotY = 0;
        Matrix_setPosition(self->tmp, pos->m03, pos->m13, pos->m23);
    } else {
        Matrix_set_m(self->tmp, pos);
    }
    if (absolutePos) Matrix_setPosition(self->tmp, 0, 0, 0);

    Matrix_translate(self->tmp, self->x, self->y, self->z);
    Matrix_setRotX(self->tmp2, self->rotX);
    Matrix_mul(self->tmp, self->tmp2);
    Matrix_rotY(self->tmp, self->rotY);

    newRotX += self->rotX;
    newRotY += self->rotY;

    if (self->fset || self->smoothSteps == 1) {
        Matrix_set_m(self->camera, self->tmp);
        self->currentRotX = newRotX;
        self->currentRotY = newRotY;
        self->fset = 0;
    } else {
        interpolation(self, self->tmpPos, self->tmp);
        self->currentRotX += (newRotX - self->currentRotX) / self->smoothSteps;
        float adj = newRotY + (newRotY < self->currentRotY ? 360 : -360);
        if (qe_absf(adj - self->currentRotY) < qe_absf(newRotY - self->currentRotY)) {
            self->currentRotY += (adj - self->currentRotY) / self->smoothSteps;
        } else {
            self->currentRotY += (newRotY - self->currentRotY) / self->smoothSteps;
        }
        Matrix_set_m(self->camera, self->tmpPos);
    }
}

void Camera_addAngle(Camera *s, int i) { s->rotX += i; }
void Camera_addY    (Camera *s, int i) { s->y    += i; }
void Camera_addZ    (Camera *s, int i) { s->z    += i; }
void Camera_addX    (Camera *s, int i) { s->x    += i; }

Matrix *Camera_getCamera(Camera *s) { return s->camera; }

void Camera_calcPart(Camera *self, House *home) {
    self->part = House_calcPart(home, self->oldPart, self->camera->m03, self->camera->m13, self->camera->m23);
    self->oldPart = self->part;
}
int  Camera_getPart(const Camera *s) { return s->part; }
void Camera_setPart(Camera *s, int p) { s->part = p; }
