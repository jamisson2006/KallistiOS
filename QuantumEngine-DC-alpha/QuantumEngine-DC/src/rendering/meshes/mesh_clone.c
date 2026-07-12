/*
 * mesh_clone.c — porte fiel de code/Rendering/Meshes/MeshClone.java
 */
#include "mesh_clone.h"
#include "mesh.h"
#include "polygon3v.h"
#include "polygon4v.h"
#include "lighted_polygon3v.h"
#include "lighted_polygon4v.h"
#include "color_lighted_polygon3v.h"
#include "color_lighted_polygon4v.h"
#include "../vertex.h"
#include "../renderobject.h"
#include "../directx7.h"
#include "../multytexture.h"

#include <stdlib.h>

static int search(Vertex **verts, int nv, Vertex *v) {
    for (int i = 0; i < nv; i++) if (verts[i] == v) return i;
    return -1;
}

MeshClone *MeshClone_new(void) { return (MeshClone*) calloc(1, sizeof(MeshClone)); }

MeshClone *MeshClone_new_fromMesh(Mesh *mesh) {
    MeshClone *c = MeshClone_new();
    int nv, np;
    Vertex **verts = Mesh_vertices(mesh, &nv);
    RenderObject **polys = Mesh_polygons(mesh, &np);

    c->vertexCoords_n = nv * 3;
    c->vertexCoords   = (int*) malloc(sizeof(int) * c->vertexCoords_n);
    for (int i = 0; i < nv; i++) {
        c->vertexCoords[i*3]   = verts[i]->x;
        c->vertexCoords[i*3+1] = verts[i]->y;
        c->vertexCoords[i*3+2] = verts[i]->z;
    }

    /* count Polygon3V / Polygon4V */
    int n3 = 0, n4 = 0;
    for (int i = 0; i < np; i++) {
        if (polys[i]->vt == Polygon3V_vt() ||
            polys[i]->vt == LightedPolygon3V_vt() ||
            polys[i]->vt == ColorLightedPolygon3V_vt()) n3++;
        if (polys[i]->vt == Polygon4V_vt() ||
            polys[i]->vt == LightedPolygon4V_vt() ||
            polys[i]->vt == ColorLightedPolygon4V_vt()) n4++;
    }
    c->poly3vData_n = n3 * 10;
    c->poly3vData   = (int*) malloc(sizeof(int) * (c->poly3vData_n > 0 ? c->poly3vData_n : 1));
    c->poly4vData_n = n4 * 13;
    c->poly4vData   = (int*) malloc(sizeof(int) * (c->poly4vData_n > 0 ? c->poly4vData_n : 1));

    int i3 = 0, i4 = 0;
    for (int i = 0; i < np; i++) {
        RenderObject *ro = polys[i];
        if (ro->vt == Polygon3V_vt() || ro->vt == LightedPolygon3V_vt() || ro->vt == ColorLightedPolygon3V_vt()) {
            Polygon3V *p = (Polygon3V*) ro;
            c->poly3vData[i3*10]   = search(verts, nv, p->a);
            c->poly3vData[i3*10+1] = search(verts, nv, p->b);
            c->poly3vData[i3*10+2] = search(verts, nv, p->c);
            c->poly3vData[i3*10+3] = p->au;
            c->poly3vData[i3*10+4] = p->av;
            c->poly3vData[i3*10+5] = p->bu;
            c->poly3vData[i3*10+6] = p->bv;
            c->poly3vData[i3*10+7] = p->cu;
            c->poly3vData[i3*10+8] = p->cv;
            c->poly3vData[i3*10+9] = p->tex;
            i3++;
        }
        if (ro->vt == Polygon4V_vt() || ro->vt == LightedPolygon4V_vt() || ro->vt == ColorLightedPolygon4V_vt()) {
            Polygon4V *p = (Polygon4V*) ro;
            c->poly4vData[i4*13]    = search(verts, nv, p->a);
            c->poly4vData[i4*13+1]  = search(verts, nv, p->b);
            c->poly4vData[i4*13+2]  = search(verts, nv, p->c);
            c->poly4vData[i4*13+3]  = search(verts, nv, p->d);
            c->poly4vData[i4*13+4]  = p->au;
            c->poly4vData[i4*13+5]  = p->av;
            c->poly4vData[i4*13+6]  = p->bu;
            c->poly4vData[i4*13+7]  = p->bv;
            c->poly4vData[i4*13+8]  = p->cu;
            c->poly4vData[i4*13+9]  = p->cv;
            c->poly4vData[i4*13+10] = p->du;
            c->poly4vData[i4*13+11] = p->dv;
            c->poly4vData[i4*13+12] = p->tex;
            i4++;
        }
    }
    c->texture = Mesh_getTexture(mesh);
    return c;
}

void MeshClone_destroy(MeshClone *self) {
    if (!self) return;
    free(self->vertexCoords);
    free(self->poly3vData);
    free(self->poly4vData);
    free(self);
}

Mesh *MeshClone_copy(const MeshClone *self) {
    int nv = self->vertexCoords_n / 3;
    Vertex **verts = (Vertex**) malloc(sizeof(Vertex*) * nv);
    for (int i = 0; i < nv; i++) {
        verts[i] = Vertex_new_xyz(self->vertexCoords[i*3],
                                  self->vertexCoords[i*3+1],
                                  self->vertexCoords[i*3+2]);
    }

    int n3 = self->poly3vData_n / 10;
    int n4 = self->poly4vData_n / 13;

    RenderObject **polys = (RenderObject**) malloc(sizeof(RenderObject*) * (n3 + n4));

    int idx = 0;
    for (int i = 0; i < n3; i++) {
        int a = self->poly3vData[i*10];
        int b = self->poly3vData[i*10+1];
        int c = self->poly3vData[i*10+2];
        int au = self->poly3vData[i*10+3], av = self->poly3vData[i*10+4];
        int bu = self->poly3vData[i*10+5], bv = self->poly3vData[i*10+6];
        int cu = self->poly3vData[i*10+7], cv = self->poly3vData[i*10+8];
        int tx = self->poly3vData[i*10+9];

        RenderObject *pl;
        if (!self->lighting) {
            Polygon3V *p = Polygon3V_new(verts[a], verts[b], verts[c],
                                         (int8_t)au,(int8_t)av,(int8_t)bu,(int8_t)bv,(int8_t)cu,(int8_t)cv);
            p->tex = tx; pl = &p->base;
        } else if (DirectX7_standartDrawmode == 9) {
            LightedPolygon3V *p = LightedPolygon3V_new(verts[a], verts[b], verts[c],
                (int8_t)au,(int8_t)av,(int8_t)bu,(int8_t)bv,(int8_t)cu,(int8_t)cv);
            p->base.tex = tx; pl = &p->base.base;
        } else {
            ColorLightedPolygon3V *p = ColorLightedPolygon3V_new(verts[a], verts[b], verts[c],
                (int8_t)au,(int8_t)av,(int8_t)bu,(int8_t)bv,(int8_t)cu,(int8_t)cv);
            p->base.tex = tx; pl = &p->base.base;
        }
        polys[idx++] = pl;
    }
    for (int i = 0; i < n4; i++) {
        int a = self->poly4vData[i*13];
        int b = self->poly4vData[i*13+1];
        int c = self->poly4vData[i*13+2];
        int d = self->poly4vData[i*13+3];
        int au=self->poly4vData[i*13+4], av=self->poly4vData[i*13+5];
        int bu=self->poly4vData[i*13+6], bv=self->poly4vData[i*13+7];
        int cu=self->poly4vData[i*13+8], cv=self->poly4vData[i*13+9];
        int du=self->poly4vData[i*13+10], dv=self->poly4vData[i*13+11];
        int tx = self->poly4vData[i*13+12];

        RenderObject *pl;
        if (!self->lighting) {
            Polygon4V *p = Polygon4V_new(verts[a], verts[b], verts[c], verts[d],
                (int8_t)au,(int8_t)av,(int8_t)bu,(int8_t)bv,(int8_t)cu,(int8_t)cv,(int8_t)du,(int8_t)dv);
            p->tex = tx; pl = &p->base;
        } else if (DirectX7_standartDrawmode == 9) {
            LightedPolygon4V *p = LightedPolygon4V_new(verts[a], verts[b], verts[c], verts[d],
                (int8_t)au,(int8_t)av,(int8_t)bu,(int8_t)bv,(int8_t)cu,(int8_t)cv,(int8_t)du,(int8_t)dv);
            p->base.tex = tx; pl = &p->base.base;
        } else {
            ColorLightedPolygon4V *p = ColorLightedPolygon4V_new(verts[a], verts[b], verts[c], verts[d],
                (int8_t)au,(int8_t)av,(int8_t)bu,(int8_t)bv,(int8_t)cu,(int8_t)cv,(int8_t)du,(int8_t)dv);
            p->base.tex = tx; pl = &p->base.base;
        }
        polys[idx++] = pl;
    }
    return Mesh_new_vpt(verts, nv, polys, idx, self->texture);
}
