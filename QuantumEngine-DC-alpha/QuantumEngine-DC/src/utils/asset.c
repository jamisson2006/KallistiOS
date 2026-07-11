/*
 * asset.c — porte fiel de code/utils/Asset.java (Quantum Engine J2ME)
 *
 * Simula o Hashtable Java com uma tabela linear de par (key, entry). Cada
 * entry conhece seu "kind" para poder chamar destroy() apropriado.
 */
#include "asset.h"
#include "gameini.h"
#include "stringtools.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* --- assinaturas de modulos externos (a serem convertidos depois) --- */
/* Loader real vive em Room (Meshes/loader). Aqui declaramos so a assinatura. */
extern Mesh **Room_loadMeshes(const char *file, float sX, float sY, float sZ, int *out_count);
extern void   Mesh_destroy(Mesh *self);
extern Texture *Texture_createTexture(const char *file);
extern RawImage *RawImage_createRawImage(const char *file);
extern RawImage *RawImage_createMipRawImage(RawImage *rimg);
extern int      RawImage_scale(const RawImage *self);
extern int      RawImage_isPalette(const RawImage *self);
extern void     RawImage_setScale(RawImage *self, int scale);
extern void    *RawImage_img(const RawImage *self);
extern void     RawImage_setImg(RawImage *self, void *img);
extern int      RawImage_w(const RawImage *self);
extern void     RawImage_setW(RawImage *self, int w);
extern int      RawImage_h(const RawImage *self);
extern void     RawImage_setH(RawImage *self, int h);
extern Morphing *Morphing_new(int16_t **anim, int rows, int cols, Mesh *clone);
extern int16_t **Morphing_create(Mesh **meshes, int nm, int start, int end, int *out_rows, int *out_cols);
extern MeshImage *MeshImage_new(Mesh *mesh, Morphing *m);
extern Texture **Mesh_textures(Mesh *self, int *out_count);
extern void Texture_setDrawMode(Texture *t, int8_t mode);

/* Externais em Main (converteremos main.c) */
extern int  Main_mipMapping;
extern int  Main_isSounds;
extern int  Main_sounds;
extern int  Main_isFootsteps;
extern int  Main_footsteps;

/* --- Imports do utils (ja convertidos aqui) --- */
extern int *StringTools_cutOnInts(const char *str, char d, int *cnt); /* usado em Fringe */

/* --- Tabela de entries --- */

typedef enum {
    QE_ASSET_MESH,        /* Mesh*  */
    QE_ASSET_MESHES,      /* Mesh** com count */
    QE_ASSET_ANIMATION,   /* int16_t** com rows/cols */
    QE_ASSET_MORPHING,    /* Morphing* */
    QE_ASSET_MESHIMAGE,   /* MeshImage* */
    QE_ASSET_RAWIMAGE,    /* RawImage* */
    QE_ASSET_MIPS,        /* RawImage[3] heap */
    QE_ASSET_SOUND        /* Sound* */
} qe_asset_kind;

typedef struct {
    char *key;
    qe_asset_kind kind;
    void *ptr;    /* generico */
    int   count;  /* MESHES: length; ANIMATION: rows; MIPS: 3 */
    int   count2; /* ANIMATION: cols */
} qe_asset_entry;

static qe_asset_entry *g_table = NULL;
static int g_count = 0, g_cap = 0;
static int g_repeat = 0; /* fiel ao Java (usado apenas em clear()) */

static void qe_grow_table(void) {
    if (g_count + 1 <= g_cap) return;
    g_cap = g_cap ? g_cap * 2 : 32;
    g_table = (qe_asset_entry*) realloc(g_table, sizeof(qe_asset_entry) * g_cap);
}

static qe_asset_entry *qe_find_asset(const char *key) {
    for (int i = 0; i < g_count; i++)
        if (strcmp(g_table[i].key, key) == 0) return &g_table[i];
    return NULL;
}

static void qe_put_asset(const char *key, qe_asset_kind kind, void *ptr, int c1, int c2) {
    qe_grow_table();
    qe_asset_entry *e = &g_table[g_count++];
    e->key    = strdup(key);
    e->kind   = kind;
    e->ptr    = ptr;
    e->count  = c1;
    e->count2 = c2;
}

/* helper de key: gera "PFX_file_sX_sY_sZ" (usando %g para reproduzir o
 * "Float.toString" do Java o mais fielmente possivel). */
static void qe_make_key(char *buf, size_t sz, const char *pfx,
                        const char *file, float sX, float sY, float sZ) {
    snprintf(buf, sz, "%s%s_%g_%g_%g", pfx, file, sX, sY, sZ);
}

/* --- Interface publica --- */

void Asset_clear(void) {
    /* try/catch Java: iteramos e chamamos destroy no que couber. */
    for (int i = 0; i < g_count; i++) {
        qe_asset_entry *e = &g_table[i];
        switch (e->kind) {
        case QE_ASSET_SOUND:  Sound_destroy((Sound*) e->ptr);                  break;
        case QE_ASSET_MESH:   Mesh_destroy((Mesh*) e->ptr);                    break;
        case QE_ASSET_MESHES: {
            Mesh **arr = (Mesh**) e->ptr;
            for (int xx = 0; xx < e->count; xx++) Mesh_destroy(arr[xx]);
            free(arr);
        } break;
        default: /* Java so destruia Sound e Mesh; o resto ficava para GC. */
            break;
        }
        free(e->key);
    }
    g_count = 0;
    g_repeat = 0; /* fiel */
}

int Asset_desizeSomething(void) {
    RawImage *chosen = NULL;
    /* extern impl declarada acima em ImageResize (converteremos ja abaixo) */
    extern void  *ImageResize_cubic2XHorDesize_v(void *img, int w, int h);
    extern void  *ImageResize_cubic2XVertDesize_v(void *img, int w, int h);

    for (int i = 0; i < g_count; i++) {
        qe_asset_entry *e = &g_table[i];
        if (e->kind != QE_ASSET_RAWIMAGE) continue;
        if (strncmp(e->key, "RIMG_", 5) != 0) continue;

        RawImage *tr = (RawImage*) e->ptr;
        int sc = RawImage_scale(tr);
        if (sc == 0 || !RawImage_isPalette(tr)) continue;

        if (!chosen) chosen = tr;
        else if (sc > RawImage_scale(chosen)) chosen = tr;
    }

    if (chosen && RawImage_scale(chosen) > 0) {
        int sc = RawImage_scale(chosen);
        int w = RawImage_w(chosen), h = RawImage_h(chosen);
        void *img = RawImage_img(chosen);

        if (sc == 1) {
            RawImage_setImg(chosen, ImageResize_cubic2XHorDesize_v(img, w, h));
            RawImage_setW(chosen, w / 2);
            RawImage_setScale(chosen, 0);
        } else {
            RawImage_setImg(chosen, ImageResize_cubic2XVertDesize_v(img, w, h));
            RawImage_setH(chosen, h / 2);
            RawImage_setScale(chosen, 1);
        }
        return 1;
    }
    return 0;
}

/* --- getMeshes --- */

Mesh **Asset_getMeshes(const char *file, float sX, float sY, float sZ, int *out_count) {
    char key[256]; qe_make_key(key, sizeof key, "MESH_", file, sX, sY, sZ);
    qe_asset_entry *e = qe_find_asset(key);
    if (e) { *out_count = e->count; return (Mesh**) e->ptr; }

    int n;
    Mesh **mesh = Room_loadMeshes(file, sX, sY, sZ, &n);
    qe_put_asset(key, QE_ASSET_MESHES, mesh, n, 0);
    *out_count = n;
    return mesh;
}

Mesh *Asset_getMeshCloneDynamic(const char *file, float sX, float sY, float sZ) {
    int n;
    Mesh **arr = Room_loadMeshes(file, sX, sY, sZ, &n);
    Mesh *m = n > 0 ? arr[0] : NULL;
    /* Java devolve apenas [0] e ignora o resto (leak intencional no J2ME). */
    return m;
}

Mesh *Asset_getMeshClone(const char *file, float sX, float sY, float sZ) {
    char key[256]; qe_make_key(key, sizeof key, "MESHCLONE_", file, sX, sY, sZ);
    qe_asset_entry *e = qe_find_asset(key);
    if (e) return (Mesh*) e->ptr;

    int n;
    Mesh **arr = Room_loadMeshes(file, sX, sY, sZ, &n);
    Mesh *m = n > 0 ? arr[0] : NULL;
    qe_put_asset(key, QE_ASSET_MESH, m, 0, 0);
    return m;
}

int16_t **Asset_getAnimation(const char *file, float sX, float sY, float sZ,
                             int *out_rows, int *out_cols) {
    char key[256]; qe_make_key(key, sizeof key, "ANIMATION_", file, sX, sY, sZ);
    qe_asset_entry *e = qe_find_asset(key);
    if (e) { *out_rows = e->count; *out_cols = e->count2; return (int16_t**) e->ptr; }

    int nm;
    Mesh **meshes = Asset_getMeshes(file, sX, sY, sZ, &nm);
    int rows, cols;
    int16_t **anim = Morphing_create(meshes, nm, 0, nm, &rows, &cols);
    qe_put_asset(key, QE_ASSET_ANIMATION, anim, rows, cols);
    *out_rows = rows; *out_cols = cols;
    return anim;
}

Morphing *Asset_getMorphing(const char *file, float sX, float sY, float sZ) {
    char key[256]; qe_make_key(key, sizeof key, "MORPHING_", file, sX, sY, sZ);
    qe_asset_entry *e = qe_find_asset(key);
    if (e) return (Morphing*) e->ptr;

    int rows, cols;
    int16_t **anim = Asset_getAnimation(file, sX, sY, sZ, &rows, &cols);
    Mesh *clone = Asset_getMeshClone(file, sX, sY, sZ);
    Morphing *m = Morphing_new(anim, rows, cols, clone);
    qe_put_asset(key, QE_ASSET_MORPHING, m, 0, 0);
    return m;
}

MeshImage *Asset_getMeshImage(const char *file, float sX, float sY, float sZ) {
    char key[256]; qe_make_key(key, sizeof key, "MESHIMAGE_", file, sX, sY, sZ);
    qe_asset_entry *e = qe_find_asset(key);
    if (e) return (MeshImage*) e->ptr;

    MeshImage *m = MeshImage_new(
        Asset_getMeshCloneDynamic(file, sX, sY, sZ),
        Asset_getMorphing(file, sX, sY, sZ));
    qe_put_asset(key, QE_ASSET_MESHIMAGE, m, 0, 0);
    return m;
}

MeshImage *Asset_getMeshImageDynamic(const char *file, float sX, float sY, float sZ) {
    return MeshImage_new(
        Asset_getMeshClone(file, sX, sY, sZ),
        Asset_getMorphing(file, sX, sY, sZ));
}

Texture *Asset_getTexture(const char *file) {
    Texture *t = Texture_createTexture(file);
    /* Java: texture.mip = (RawImage[])table.get("MIPMAPS_" + file);
     * O mip fica na propria Texture — expomos via setter no modulo Texture. */
    char key[256]; snprintf(key, sizeof key, "MIPMAPS_%s", file);
    qe_asset_entry *e = qe_find_asset(key);
    if (e) {
        extern void Texture_setMip(Texture *t, RawImage **mip, int n);
        Texture_setMip(t, (RawImage**) e->ptr, e->count);
    }
    return t;
}

RawImage *Asset_getRawImage(const char *file) {
    char key[256]; snprintf(key, sizeof key, "RIMG_%s", file);
    qe_asset_entry *e = qe_find_asset(key);
    RawImage *r = e ? (RawImage*) e->ptr : NULL;

    if (r == NULL) {
        r = RawImage_createRawImage(file);
        qe_put_asset(key, QE_ASSET_RAWIMAGE, r, 0, 0);

        if (Main_mipMapping) {
            RawImage *m1 = RawImage_createMipRawImage(r);
            if (m1) {
                char k1[256]; snprintf(k1, sizeof k1, "RIMGMIP1_%s", file);
                qe_put_asset(k1, QE_ASSET_RAWIMAGE, m1, 0, 0);

                /* Segundo mip: Java repete createMipRawImage(rImg) — bug fiel
                 * do original, deixamos igual. */
                RawImage *m2 = RawImage_createMipRawImage(r);
                if (m2) {
                    char k2[256]; snprintf(k2, sizeof k2, "RIMGMIP2_%s", file);
                    qe_put_asset(k2, QE_ASSET_RAWIMAGE, m2, 0, 0);
                } else {
                    m2 = m1;
                }
                RawImage **mips = (RawImage**) malloc(sizeof(RawImage*) * 3);
                mips[0] = r; mips[1] = m1; mips[2] = m2;

                char km[256]; snprintf(km, sizeof km, "MIPMAPS_%s", file);
                qe_put_asset(km, QE_ASSET_MIPS, mips, 3, 0);
            }
        }
    }
    return r;
}

Texture *Asset_getTextureNM(const char *file) {
    return Texture_createTexture(file);
}

Sound *Asset_getSound(const char *file) {
    if (Main_sounds == 0 || !Main_isSounds) return NULL;

    char key[256]; snprintf(key, sizeof key, "SND_%s", file);
    qe_asset_entry *e = qe_find_asset(key);
    if (e) return (Sound*) e->ptr;

    Sound *s = Sound_new_file(file);
    qe_put_asset(key, QE_ASSET_SOUND, s, 0, 0);
    return s;
}

Sound *Asset_getFootsteps(const char *file) {
    if (Main_footsteps == 0 || !Main_isFootsteps) return NULL;

    char key[256]; snprintf(key, sizeof key, "SND_%s", file);
    qe_asset_entry *e = qe_find_asset(key);
    if (e) return (Sound*) e->ptr;

    Sound *s = Sound_new_file(file);
    qe_put_asset(key, QE_ASSET_SOUND, s, 0, 0);
    return s;
}

/* --- applyMeshEffects --- */

void Asset_applyMeshEffectsArr(Mesh **meshes, int count, const char *effects) {
    (void) count;
    if (!meshes) return;
    Asset_applyMeshEffects(meshes[0], effects);
}

void Asset_applyMeshEffects(Mesh *mesh, const char *effects) {
    if (effects == NULL) return;

    int nmods;
    char **mods = GameIni_cutOnStrings(effects, ',', ';', &nmods);

    int ntex;
    Texture **texs = Mesh_textures(mesh, &ntex);
    if (texs == NULL) { StringTools_freeStrings(mods, nmods); return; }

    int length = ntex;
    if (nmods < length) length = nmods;

    for (int i = 0; i < length; i++) {
        if (texs[i] != NULL && mods[i] != NULL) {
            Texture_setDrawMode(texs[i], (int8_t) StringTools_parseInt(mods[i]));
        }
    }
    StringTools_freeStrings(mods, nmods);
}
