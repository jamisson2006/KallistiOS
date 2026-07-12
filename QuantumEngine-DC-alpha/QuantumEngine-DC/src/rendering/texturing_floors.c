/*
 * texturing_floors.c — porte de code/Rendering/TexturingFloors.java
 *
 * Esta fase delega ao dispatcher afim (que ja tem modo 0 fiel). Ver
 * STATUS_CONVERSAO.md — as otimizacoes de floor (varredura por linha
 * horizontal com u/v perspectiva-correta calculada uma vez por linha)
 * serao portadas em fase seguinte.
 */
#include "texturing_floors.h"
#include "texturing_affine.h"

void TexturingFloors_paint(DirectX7 *g3d, Texture *texture,
    Vertex *a, int au, int av,
    Vertex *b, int bu, int bv,
    Vertex *c, int cu, int cv,
    int fogc, int dmode, int qz, int q,
    int al, int bl, int cl,
    int ag, int bg, int cg,
    int ab, int bb, int cb,
    int nx, int ny, int nz) {
    (void) qz; (void) q;
    TexturingAffine_paint(g3d, texture, a, au, av, b, bu, bv, c, cu, cv,
        fogc, dmode, 0,
        al, bl, cl, ag, bg, cg, ab, bb, cb,
        nx, ny, nz);
}
