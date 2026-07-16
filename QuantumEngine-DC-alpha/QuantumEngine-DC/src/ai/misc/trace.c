/*
 * trace.c — porte fiel de code/AI/misc/Trace.java
 */
#include "trace.h"
#include "../../math/matrix.h"
#include <stdlib.h>
#include <math.h>

typedef struct Polygon4V    Polygon4V;
typedef struct Vertex       Vertex;
typedef struct Texture      Texture;
typedef struct DirectX7     DirectX7;
typedef struct Matrix       Matrix;
typedef struct RenderObject RenderObject;

extern Vertex     *Vertex_new_xyz(int x, int y, int z);
extern void        Vertex_mul_i(Vertex *v, int x, int y, int z);
extern void        Vertex_div_i(Vertex *v, int x, int y, int z);
extern void        Vertex_sub(Vertex *v, int x, int y, int z);
extern void        Vertex_add(Vertex *v, int x, int y, int z);
extern void        Vertex_transform(Vertex *v, const Matrix *mat);
extern void        Vertex_transformFE(Vertex *v, const Matrix *mat);
extern void        Vertex_project(Vertex *v, const DirectX7 *g3d);

extern Polygon4V  *Polygon4V_new(Vertex *a, Vertex *b, Vertex *c, Vertex *d,
                                  int8_t au, int8_t av, int8_t bu, int8_t bv,
                                  int8_t cu, int8_t cv, int8_t du, int8_t dv);
extern int         Polygon4V_isVisible(Polygon4V *p, int x1, int y1, int x2, int y2);
extern Vertex     *Polygon4V_getA(Polygon4V *p);
extern Vertex     *Polygon4V_getB(Polygon4V *p);
extern Vertex     *Polygon4V_getC(Polygon4V *p);
extern Vertex     *Polygon4V_getD(Polygon4V *p);
extern int        *Polygon4V_getSzPtr(Polygon4V *p);

extern Texture    *Asset_getTexture(const char *path);
extern void        Texture_setPerspectiveCorrection(Texture *tex, int on);

extern Matrix     *DirectX7_getInvCamera(DirectX7 *g3d);
extern void        DirectX7_addRenderObjectDT(DirectX7 *g3d, RenderObject *obj, Texture *tex);

extern void        Character_setCollision(Character *ch, bool c);
extern void        Character_setCollidable(Character *ch, bool c);
extern void        Character_setOnFloor(Character *ch, bool f);
extern void        Character_setSpeedZero(Character *ch);

extern char *Main_Blood;

Trace *Trace_new(int fx, int fy, int fz,
                 Vector3D *v1, Vector3D *v2, Vector3D *v3, Vector3D *v4,
                 Vector3D *size) {
    Trace *self = (Trace *)calloc(1, sizeof(Trace));
    self->tex = Asset_getTexture(Main_Blood);
    Texture_setPerspectiveCorrection(self->tex, 1);

    Character_setCollision(self->base.character, true);
    Character_setCollidable(self->base.character, false);
    Character_setOnFloor(self->base.character, true);
    Character_setSpeedZero(self->base.character);
    Character_setPosition(self->base.character, fx, fy, fz);

    Matrix mat;
    Matrix_setIdentity(&mat);
    Matrix_setPosition(&mat, fx, fy, fz);

    self->pos = Vertex_new_xyz(fx, fy, fz);
    self->vec.x = 1002;
    self->vec.y = 1002;
    self->vec.z = 1002;
    if (size->x == 0) self->vec.x = 996;
    if (size->y == 0) self->vec.y = 996;
    if (size->z == 0) self->vec.z = 996;

    Vertex *va = Vertex_new_xyz(v1->x, v1->y, v1->z);
    Vertex *vb = Vertex_new_xyz(v2->x, v2->y, v2->z);
    Vertex *vc = Vertex_new_xyz(v3->x, v3->y, v3->z);
    Vertex *vd = Vertex_new_xyz(v4->x, v4->y, v4->z);

    Vertex_mul_i(va, -1700, -1700, -1700);
    Vertex_mul_i(vb, -1700, -1700, -1700);
    Vertex_mul_i(vc, -1700, -1700, -1700);
    Vertex_mul_i(vd, -1700, -1700, -1700);

    Vertex_div_i(va, size->x, size->y, size->z);
    Vertex_div_i(vb, size->x, size->y, size->z);
    Vertex_div_i(vc, size->x, size->y, size->z);
    Vertex_div_i(vd, size->x, size->y, size->z);

    Vertex_transformFE(va, &mat);
    Vertex_transformFE(vb, &mat);
    Vertex_transformFE(vc, &mat);
    Vertex_transformFE(vd, &mat);

    self->pol4 = Polygon4V_new(va, vb, vc, vd,
                               0, 0, (int8_t)0xff, 0,
                               (int8_t)0xff, (int8_t)0xff, 0, (int8_t)0xff);
    self->f = 0;
    return self;
}

void Trace_render(Trace *self, DirectX7 *g3d, int x1, int y1, int x2, int y2) {
    if (self->f <= 2400) GameObject_setHp(&self->base, 10000);
    self->f++;
    Character_setCollision(self->base.character, false);

    Vertex *a = Polygon4V_getA(self->pol4);
    Vertex *b = Polygon4V_getB(self->pol4);
    Vertex *c = Polygon4V_getC(self->pol4);
    Vertex *d = Polygon4V_getD(self->pol4);
    Matrix *invCam = DirectX7_getInvCamera(g3d);

    Vertex_transform(a, invCam);
    Vertex_transform(b, invCam);
    Vertex_transform(c, invCam);
    Vertex_transform(d, invCam);
    Vertex_project(a, g3d);
    Vertex_project(b, g3d);
    Vertex_project(c, g3d);
    Vertex_project(d, g3d);

    if (Polygon4V_isVisible(self->pol4, x1, y1, x2, y2)) {
        int px = self->pos->x, py = self->pos->y, pz = self->pos->z;

        if (self->f < 250) {
            Vertex_sub(a, px, py, pz);
            Vertex_sub(b, px, py, pz);
            Vertex_sub(c, px, py, pz);
            Vertex_sub(d, px, py, pz);

            Vertex_mul_i(a, self->vec.x, self->vec.y, self->vec.z);
            Vertex_mul_i(b, self->vec.x, self->vec.y, self->vec.z);
            Vertex_mul_i(c, self->vec.x, self->vec.y, self->vec.z);
            Vertex_mul_i(d, self->vec.x, self->vec.y, self->vec.z);

            Vertex_div_i(a, 1000, 1000, 1000);
            Vertex_div_i(b, 1000, 1000, 1000);
            Vertex_div_i(c, 1000, 1000, 1000);
            Vertex_div_i(d, 1000, 1000, 1000);

            Vertex_add(a, px, py, pz);
            Vertex_add(b, px, py, pz);
            Vertex_add(c, px, py, pz);
            Vertex_add(d, px, py, pz);
        }

        if (self->f > 2400) GameObject_setHp(&self->base, 0);

        int *szp = Polygon4V_getSzPtr(self->pol4);
        if (*szp > -45000) {
            DirectX7_addRenderObjectDT(g3d, (RenderObject *)self->pol4, self->tex);
            *szp += 3002;

            RenderObject *oldFloor = self->base.character->oldFloorPoly;
            if (oldFloor != NULL) {
                extern int RenderObject_getSz(RenderObject *ro);
                *szp = RenderObject_getSz(oldFloor) + 3000;
            }
        }
    }
}

void Trace_activate(Trace *self, House *house, Player *player, GameScreen *gs) {
    (void)self; (void)house; (void)player; (void)gs;
}
