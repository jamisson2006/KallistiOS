/*
 * texturing_experiments.c — porte fiel de code/Rendering/TexturingExperiments.java
 */
#include "texturing_experiments.h"
#include "vertex.h"
#include <stdlib.h>
#include <string.h>

typedef struct Line {
    int y;
    int x_start, x_end;
    int u_start, u_end;
    int v_start, v_end;
    int dx_start, dx_end;
    int du_start, du_end;
    int dv_start, dv_end;
} Line;

static Line *g_lines = NULL;
static int   g_lines_cap = 0;

static Vertex *g_vertsBuffer[4];
static int     g_uTexCoords[4];
static int     g_vTexCoords[4];

void TexturingExperiments_renderQuad(DirectX7 *g3d, Texture *tex,
    Vertex *a, int au, int av,
    Vertex *b, int bu, int bv,
    Vertex *c, int cu, int cv,
    Vertex *d, int du, int dv) {
    g_vertsBuffer[0] = a; g_uTexCoords[0] = au; g_vTexCoords[0] = av;
    g_vertsBuffer[1] = b; g_uTexCoords[1] = bu; g_vTexCoords[1] = bv;
    g_vertsBuffer[2] = c; g_uTexCoords[2] = cu; g_vTexCoords[2] = cv;
    g_vertsBuffer[3] = d; g_uTexCoords[3] = du; g_vTexCoords[3] = dv;
    TexturingExperiments_renderPolygon(g3d, tex, g_vertsBuffer, 4, g_uTexCoords, g_vTexCoords);
}

void TexturingExperiments_renderPolygon(DirectX7 *g3d, Texture *tex,
    Vertex **verts, int nverts, int *uTex, int *vTex) {
    (void) g3d; (void) tex; (void) uTex; (void) vTex;

    if (g_lines_cap < nverts) {
        g_lines_cap = nverts;
        g_lines = (Line*) realloc(g_lines, sizeof(Line) * g_lines_cap);
    }

    Vertex **srtVerts = (Vertex**) malloc(sizeof(Vertex*) * nverts);
    memcpy(srtVerts, verts, sizeof(Vertex*) * nverts);

    /* bubble sort por sy — fiel ao Java */
    for (int size = nverts - 1; size >= 1; size--) {
        for (int i = 1; i <= size; i++) {
            if (srtVerts[i-1]->sy > srtVerts[i]->sy) {
                Vertex *tmp = srtVerts[i-1]; srtVerts[i-1] = srtVerts[i]; srtVerts[i] = tmp;
            }
        }
    }

    /* Java: for(int i=0;i<verts.length;i++) {} — loop vazio no original;
     * preservado. */
    for (int i = 0; i < nverts; i++) { (void) i; }

    free(srtVerts);
}
