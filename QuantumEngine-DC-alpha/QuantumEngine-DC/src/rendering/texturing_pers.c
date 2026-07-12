/*
 * texturing_pers.c — porte de code/Rendering/TexturingPers.java (dispatcher + fallback)
 *
 * Esta fase porta:
 *   - o dispatcher paint() com o mesmo tratamento de w/h e ajuste de UV;
 *   - o fallback ao modo 0 quando modos avancados nao estao portados
 *     (paridade com o comportamento Java quando fogQ==0).
 *
 * Os subprograms paintPersSub_*, painters de "modo N" da versao Java
 * (2266 linhas de rasterizador com correcao de perspectiva) serao
 * portados em fases seguintes — hoje, a chamada delega para
 * TexturingAffine (que ja tem modo 0 e 9 fieis).
 *
 * A separacao "afim vs. perspectiva" acontece a nivel de MODULOS externos
 * (Polygon3V/4V ja escolhem qual chamar) — para efeitos de correcao visual,
 * o fallback afim renderiza corretamente porem sem correcao de perspectiva.
 */
#include "texturing_pers.h"
#include "texturing_affine.h"
#include "vertex.h"
#include "texture.h"
#include "raw_image.h"
#include "directx7.h"
#include "../utils/main.h"

int TexturingPers_move = 150;

extern int DeveloperMenu_renderPolygonsOverwrite;

void TexturingPers_paint(DirectX7 *g3d, Texture *texture,
    Vertex *a, int au, int av,
    Vertex *b, int bu, int bv,
    Vertex *c, int cu, int cv,
    int fogc, int dmode, int qz, int q,
    int al, int bl, int cl,
    int ag, int bg, int cg,
    int ab, int bb, int cb,
    int nx, int ny, int nz) {
    (void) qz; (void) q;

    if (DeveloperMenu_renderPolygonsOverwrite) {
        TexturingAffine_paintOverwrite(g3d, a, b, c);
        return;
    }
    if (!texture->rImg->alphaMixing && texture->drawmode == 4) dmode = 0;

    if (texture->rImg->w != 256) {
        if ((texture->mip != NULL && texture->mip[0]->w < 255) ||
            (texture->mip == NULL && texture->rImg->w < 255)) {
            if (au == 255) au = 256;
            if (bu == 255) bu = 256;
            if (cu == 255) cu = 256;
        }
        au = (au * texture->rImg->w) >> 8;
        bu = (bu * texture->rImg->w) >> 8;
        cu = (cu * texture->rImg->w) >> 8;
    }
    if (texture->rImg->h != 256) {
        if ((texture->mip != NULL && texture->mip[0]->h < 256) ||
            (texture->mip == NULL && texture->rImg->h < 256)) {
            if (av == 255) av = 256;
            if (bv == 255) bv = 256;
            if (cv == 255) cv = 256;
        }
        av = (av * texture->rImg->h) >> 8;
        bv = (bv * texture->rImg->h) >> 8;
        cv = (cv * texture->rImg->h) >> 8;
    }

    /* Delega a TexturingAffine — dispatcher completo, com os mesmos
     * fallbacks fieis do original quando o modo especifico ainda nao
     * foi portado nesta fase. Ver STATUS_CONVERSAO.md. */
    TexturingAffine_paint(g3d, texture, a, au, av, b, bu, bv, c, cu, cv,
        fogc, dmode, 0,
        al, bl, cl,
        ag, bg, cg,
        ab, bb, cb,
        nx, ny, nz);
}
