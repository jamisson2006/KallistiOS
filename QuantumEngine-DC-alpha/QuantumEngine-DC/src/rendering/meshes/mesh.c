/*
 * mesh.c — porte fiel de code/Rendering/Meshes/Mesh.java
 */
#include "mesh.h"
#include "polygon3v.h"
#include "polygon4v.h"
#include "lighted_polygon3v.h"
#include "lighted_polygon4v.h"
#include "morphing.h"

#include "../vertex.h"
#include "../renderobject.h"
#include "../renderobjectbuffer.h"
#include "../tmpelement.h"
#include "../multytexture.h"
#include "../texture.h"
#include "../directx7.h"
#include "../../math/matrix.h"
#include "../../math/math_utils.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

/* externals — Gameplay/Map ainda por converter */
typedef struct Light { void *pos; int color[3]; void *direction; } Light;
extern Light   **Room_lights(Room *room, int *out_n);
extern long     LightMapper_distanceSqr(Vertex *tmp, void *pos);
extern int      LightMapper_sqrMeter;
extern int      LightMapper_ambientLightMid;

static RenderObjectBuffer *g_buffer  = NULL;
static Matrix             *g_tmpMatrix = NULL;

static void ensure_globals(void) {
    if (!g_buffer)    g_buffer    = RenderObjectBuffer_new();
    if (!g_tmpMatrix) g_tmpMatrix = Matrix_new();
}

Mesh *Mesh_new(void) { return (Mesh*) calloc(1, sizeof(Mesh)); }

Mesh *Mesh_new_vp(Vertex **vertices, int nv, RenderObject **polygons, int np) {
    Mesh *m = Mesh_new();
    m->vertices = vertices; m->vertices_n = nv;
    m->polygons = polygons; m->polygons_n = np;
    return m;
}

Mesh *Mesh_new_vpt(Vertex **vertices, int nv, RenderObject **polygons, int np, MultyTexture *tex) {
    Mesh *m = Mesh_new_vp(vertices, nv, polygons, np);
    m->texture = tex;
    return m;
}

/* Java: iterar polygons e chamar destroy() em cada RenderObject. */
void Mesh_destroy(Mesh *self) {
    if (!self) return;
    self->texture = NULL;
    if (self->polygons) {
        for (int i = 0; i < self->polygons_n; i++) {
            RenderObject *p = self->polygons[i];
            if (p && p->vt && p->vt->destroy) p->vt->destroy(p);
        }
        free(self->polygons);
    }
    if (self->vertices) {
        for (int i = 0; i < self->vertices_n; i++) Vertex_free(self->vertices[i]);
        free(self->vertices);
    }
    free(self);
}

static int search_vertex(Vertex **buf, int nbuf, Vertex *vertex) {
    const int E = 10;
    for (int i = 0; i < nbuf; i++) {
        Vertex *v = buf[i];
        if (v == vertex) continue;
        int dx = v->x - vertex->x, dy = v->y - vertex->y, dz = v->z - vertex->z;
        if (dx < 0) dx = -dx; if (dy < 0) dy = -dy; if (dz < 0) dz = -dz;
        if (dx < E && dy < E && dz < E) return i;
    }
    return -1;
}

/* Substitui referencia oldVertex por newVertex em todos os poligonos. */
static void replace_vertex(RenderObject **polygons, int np, Vertex *oldV, Vertex *newV) {
    for (int i = 0; i < np; i++) {
        RenderObject *ro = polygons[i];
        if (ro->vt == Polygon4V_vt()) {
            Polygon4V *p = (Polygon4V*) ro;
            if (p->a == oldV) p->a = newV;
            if (p->b == oldV) p->b = newV;
            if (p->c == oldV) p->c = newV;
            if (p->d == oldV) p->d = newV;
        } else if (ro->vt == Polygon3V_vt()) {
            Polygon3V *p = (Polygon3V*) ro;
            if (p->a == oldV) p->a = newV;
            if (p->b == oldV) p->b = newV;
            if (p->c == oldV) p->c = newV;
        }
    }
}

void Mesh_optimize(Mesh *self) {
    int cap = self->vertices_n, n = 0;
    Vertex **buf = (Vertex**) malloc(sizeof(Vertex*) * cap);
    for (int i = 0; i < self->vertices_n; i++) {
        Vertex *v = self->vertices[i];
        int s = search_vertex(buf, n, v);
        if (s == -1) buf[n++] = v;
        else replace_vertex(self->polygons, self->polygons_n, v, buf[s]);
    }
    free(self->vertices);
    self->vertices = buf;
    self->vertices_n = n;
}

void Mesh_setTexture(Mesh *self, Texture *texture) {
    self->texture = MultyTexture_new_single(texture);
    for (int i = 0; i < self->polygons_n; i++) {
        RenderObject *p = self->polygons[i];
        if (p->vt == Polygon4V_vt()) ((Polygon4V*) p)->tex = 0;
        else                          ((Polygon3V*) p)->tex = 0;
    }
}
void Mesh_setTextureMT(Mesh *self, MultyTexture *texture) { self->texture = texture; }
void Mesh_resetTexture(Mesh *self) { self->texture = NULL; }

#define QE_MESH_MINMAX(name, cmp, init, axis) \
int Mesh_##name(const Mesh *self) { \
    int r = init; \
    for (int i = 0; i < self->vertices_n; i++) if (self->vertices[i]->axis cmp r) r = self->vertices[i]->axis; \
    return r; \
}
QE_MESH_MINMAX(maxX, >, INT_MIN, x)
QE_MESH_MINMAX(maxY, >, INT_MIN, y)
QE_MESH_MINMAX(maxZ, >, INT_MIN, z)
QE_MESH_MINMAX(minX, <, INT_MAX, x)
QE_MESH_MINMAX(minY, <, INT_MAX, y)
QE_MESH_MINMAX(minZ, <, INT_MAX, z)

MultyTexture *Mesh_getTexture(Mesh *self) { return self->texture; }
Vertex      **Mesh_vertices(Mesh *self, int *out_n) { *out_n = self->vertices_n; return self->vertices; }
RenderObject**Mesh_polygons(Mesh *self, int *out_n) { *out_n = self->polygons_n; return self->polygons; }
Texture     **Mesh_textures(Mesh *self, int *out_n) {
    if (!self->texture) { *out_n = 0; return NULL; }
    *out_n = self->texture->textures_n;
    return self->texture->textures;
}

void Mesh_render(Mesh *self, DirectX7 *g3d) {
    Mesh_renderRect(self, g3d, 0, 0, g3d->width, g3d->height);
}

void Mesh_renderRect(Mesh *self, DirectX7 *g3d, int x1, int y1, int x2, int y2) {
    ensure_globals();
    RenderObjectBuffer_addRenderObjectsMT(g_buffer, self->polygons, self->polygons_n, self->texture, x1, y1, x2, y2);
    RenderObjectBuffer_sort(g_buffer, 0, RenderObjectBuffer_getSize(g_buffer) - 1);
    TMPElement **b2 = RenderObjectBuffer_getBuffer(g_buffer);
    int n = RenderObjectBuffer_getSize(g_buffer) - 1;
    for (int i = n; i >= 0; i--) RenderObject_renderFast(b2[i]->obj, g3d, b2[i]->tex);
    RenderObjectBuffer_reset(g_buffer);
}

void Mesh_resetBuffer(void) {
    ensure_globals();
    RenderObjectBuffer_resetTex(g_buffer);
    RenderObjectBuffer_reset(g_buffer);
}

void Mesh_applySz(Mesh *self) {
    for (int i = 0; i < self->polygons_n; i++) {
        RenderObject *obj = self->polygons[i];
        if (obj->vt == Polygon4V_vt())
            obj->sz += self->texture->textures[((Polygon4V*) obj)->tex]->addsz;
        else if (obj->vt == Polygon3V_vt())
            obj->sz += self->texture->textures[((Polygon3V*) obj)->tex]->addsz;
    }
}

void Mesh_increaseMeshSz(Mesh *self, int z) {
    for (int i = 0; i < self->polygons_n; i++) self->polygons[i]->sz += z;
}

void Mesh_setAnimation(Mesh *self, Morphing *animation) {
    extern void Morphing_interpolation(Morphing *m, Vertex **vertices, int n);
    Morphing_interpolation(animation, self->vertices, self->vertices_n);
}

void Mesh_recalculateNormals(Mesh *self, const Matrix *transform) {
    ensure_globals();
    Vertex *normal = NULL;
    if (transform != NULL) {
        normal = Vertex_new_xyz(0, 0, 0);
        Matrix_set_m(g_tmpMatrix, transform);
        Matrix_setPosition(g_tmpMatrix, 0, 0, 0);
    }
    for (int i = 0; i < self->polygons_n; i++) {
        RenderObject *ro = self->polygons[i];
        if (ro->vt == Polygon4V_vt()) {
            Polygon4V *p = (Polygon4V*) ro;
            RenderObject_calculateNormals(ro, p->a, p->b, p->c);
        } else if (ro->vt == Polygon3V_vt()) {
            Polygon3V *p = (Polygon3V*) ro;
            RenderObject_calculateNormals(ro, p->a, p->b, p->c);
        }
        if (transform != NULL) {
            Vertex_set(normal, ro->nx, ro->ny, ro->nz);
            Vertex_transform(normal, g_tmpMatrix);
            ro->nx = (short) normal->x;
            ro->ny = (short) normal->y;
            ro->nz = (short) normal->z;
        }
    }
    if (normal) Vertex_free(normal);
}

void Mesh_rotateNormals(Mesh *self, const Matrix *transform) {
    ensure_globals();
    Vertex *normal = Vertex_new_xyz(0, 0, 0);
    Matrix_set_m(g_tmpMatrix, transform);
    Matrix_setPosition(g_tmpMatrix, 0, 0, 0);

    for (int i = 0; i < self->polygons_n; i++) {
        RenderObject *ro = self->polygons[i];
        Vertex_set(normal, ro->nx, ro->ny, ro->nz);
        Vertex_transform(normal, g_tmpMatrix);
        ro->nx = (short) normal->x;
        ro->ny = (short) normal->y;
        ro->nz = (short) normal->z;
    }
    Vertex_free(normal);
}

/* light calc (fiel ao Java) */
static int8_t getVertexLit(int orig, Vertex *norm, Vertex *v, Room *room) {
    (void) orig;
    int nl; Light **lights = Room_lights(room, &nl);
    long lit = 0;
    Vertex tmp; tmp.x = v->sx; tmp.y = v->sy; tmp.z = v->rz;

    for (int i = 0; i < nl; i++) {
        Light *light = lights[i];
        long distSqr = LightMapper_distanceSqr(&tmp, light->pos);
        long meter = LightMapper_sqrMeter;
        long denom = distSqr > 1 ? distSqr : 1;
        long intensity = (light->color[0] + light->color[1] + light->color[2]) / 3 * meter / denom * 8;

        extern int Vector3D_x(const void *v); extern int Vector3D_y(const void *v); extern int Vector3D_z(const void *v);
        if (distSqr < 0) intensity = 0;
        if (intensity > 1) {
            if (light->direction == NULL) {
                intensity = intensity * MathUtils_calcLight(norm->sx, norm->sy, norm->rz,
                    v->sx - Vector3D_x(light->pos), v->sy - Vector3D_y(light->pos), v->rz - Vector3D_z(light->pos)) / 255;
            } else {
                intensity = intensity * MathUtils_calcLight(Vector3D_x(light->direction), Vector3D_y(light->direction), Vector3D_z(light->direction),
                    v->sx - Vector3D_x(light->pos), v->sy - Vector3D_y(light->pos), v->rz - Vector3D_z(light->pos)) / 255;
                intensity = intensity * MathUtils_calcLight(norm->sx, norm->sy, norm->rz,
                    v->sx - Vector3D_x(light->pos), v->sy - Vector3D_y(light->pos), v->rz - Vector3D_z(light->pos)) / 255;
            }
        }
        if (intensity > 1) lit += intensity;
    }

    int r = (int)(lit - 128 + LightMapper_ambientLightMid);
    if (r > 127)  r = 127;
    if (r < -128) r = -128;
    return (int8_t) r;
}

void Mesh_updateLighting(Mesh *self, const Matrix *transform, int smoothNormals, Room *room) {
    (void) smoothNormals;
    int nl; Light **lights = Room_lights(room, &nl);
    if (lights == NULL) return;

    ensure_globals();
    Vertex *normal = Vertex_new_xyz(0, 0, 0);
    DirectX7_transformMesh(self, transform);
    Matrix_set_m(g_tmpMatrix, transform);
    Matrix_setPosition(g_tmpMatrix, 0, 0, 0);

    for (int i = 0; i < self->polygons_n; i++) {
        RenderObject *ro = self->polygons[i];
        if (ro->vt == LightedPolygon3V_vt()) {
            LightedPolygon3V *p = (LightedPolygon3V*) ro;
            Vertex_set(normal, ro->nx, ro->ny, ro->nz);
            Vertex_transform(normal, g_tmpMatrix);
            p->la = getVertexLit(p->la, normal, p->a, room);
            p->lb = getVertexLit(p->lb, normal, p->b, room);
            p->lc = getVertexLit(p->lc, normal, p->c, room);
        } else if (ro->vt == LightedPolygon4V_vt()) {
            LightedPolygon4V *p = (LightedPolygon4V*) ro;
            Vertex_set(normal, ro->nx, ro->ny, ro->nz);
            Vertex_transform(normal, g_tmpMatrix);
            p->la = getVertexLit(p->la, normal, p->a, room);
            p->lb = getVertexLit(p->lb, normal, p->b, room);
            p->lc = getVertexLit(p->lc, normal, p->c, room);
            p->ld = getVertexLit(p->ld, normal, p->d, room);
        }
    }
    Vertex_free(normal);
}
