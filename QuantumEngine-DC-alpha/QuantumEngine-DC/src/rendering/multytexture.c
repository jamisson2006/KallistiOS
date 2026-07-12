/*
 * multytexture.c — porte fiel de code/Rendering/MultyTexture.java
 */
#include "multytexture.h"
#include "texture.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../utils/asset.h"
#include "../utils/gameini.h"
#include "../utils/stringtools.h"

static char *qe_trim_dup(const char *s) {
    while (*s && isspace((unsigned char)*s)) s++;
    int len = (int) strlen(s);
    while (len > 0 && isspace((unsigned char) s[len - 1])) len--;
    char *o = (char*) malloc(len + 1);
    memcpy(o, s, len); o[len] = 0;
    return o;
}

MultyTexture *MultyTexture_new_empty(void) { return (MultyTexture*) calloc(1, sizeof(MultyTexture)); }

MultyTexture *MultyTexture_new_size(int i) {
    MultyTexture *m = MultyTexture_new_empty();
    m->textures   = (Texture**) calloc(i > 0 ? i : 1, sizeof(Texture*));
    m->textures_n = i;
    return m;
}
MultyTexture *MultyTexture_new_single(Texture *tex) {
    MultyTexture *m = MultyTexture_new_size(1);
    m->textures[0] = tex;
    return m;
}

MultyTexture *MultyTexture_new_files(const char *files, int perspectiveCorrect) {
    MultyTexture *m = MultyTexture_new_empty();
    int nt;
    char **texList = GameIni_cutOnStrings(files, ',', ';', &nt);
    m->textures   = (Texture**) calloc(nt > 0 ? nt : 1, sizeof(Texture*));
    m->textures_n = nt;

    for (int i = 0; i < nt; i++) {
        int na;
        char **anims = StringTools_cutOnStrings(texList[i], ':', &na);
        if (na == 1) {
            m->textures[i] = (Texture*) Asset_getTexture(anims[0]);
            Texture_setPerspectiveCorrection(m->textures[i], perspectiveCorrect);
        } else {
            m->textures[i] = (Texture*) Asset_getTexture(anims[1]);
            char *t = qe_trim_dup(anims[0]);
            m->textures[i]->animation_speed = StringTools_parseFloat(t);
            free(t);
            m->textures[i]->animMIP_n = na - 1;
            m->textures[i]->animMIP   = (Texture**) calloc(na - 1, sizeof(Texture*));
            for (int x = 1; x < na; x++) m->textures[i]->animMIP[x - 1] = (Texture*) Asset_getTexture(anims[x]);
            Texture_setPerspectiveCorrection(m->textures[i], perspectiveCorrect);
        }
        StringTools_freeStrings(anims, na);
    }
    StringTools_freeStrings(texList, nt);
    return m;
}

MultyTexture *MultyTexture_new_list(char **texList, int list_n) {
    MultyTexture *m = MultyTexture_new_empty();
    m->textures   = (Texture**) calloc(list_n > 0 ? list_n : 1, sizeof(Texture*));
    m->textures_n = list_n;

    for (int i = 0; i < list_n; i++) {
        fprintf(stderr, "Loading Texture:%d\n", i);
        int ni;
        char **texInfo = GameIni_cutOnStrings(texList[i], ',', ';', &ni);

        int na;
        char **animation = StringTools_cutOnStrings(texInfo[0], ':', &na);
        char *chosen = qe_trim_dup(animation[na > 1 ? 1 : 0]);
        m->textures[i] = (Texture*) Asset_getTexture(chosen);
        free(chosen);
        if (na > 1) {
            char *t = qe_trim_dup(animation[0]);
            m->textures[i]->animation_speed = StringTools_parseFloat(t);
            free(t);
            m->textures[i]->animMIP_n = na - 1;
            m->textures[i]->animMIP   = (Texture**) calloc(na - 1, sizeof(Texture*));
            for (int j = 1; j < na; j++) {
                char *tm = qe_trim_dup(animation[j]);
                m->textures[i]->animMIP[j - 1] = (Texture*) Asset_getTexture(tm);
                free(tm);
            }
        }
        StringTools_freeStrings(animation, na);
        Texture_setPerspectiveCorrection(m->textures[i], 1);

        if (ni >= 2) {
            char *tl = qe_trim_dup(texInfo[1]);
            for (char *p = tl; *p; p++) *p = (char) tolower((unsigned char) *p);
            if (strcmp(tl, "std") != 0) {
                m->textures[i]->drawmode = (int8_t) StringTools_parseInt(texInfo[1]);
            }
            free(tl);
        }
        if (ni >= 3) m->textures[i]->addsz = StringTools_parseInt(texInfo[2]);
        if (ni >= 4) Texture_setPerspectiveCorrection(m->textures[i], StringTools_parseInt(texInfo[3]) == 1);
        if (ni >= 5) m->textures[i]->castShadow = StringTools_parseInt(texInfo[4]) == 1;
        if (ni >= 6) m->textures[i]->collision  = StringTools_parseInt(texInfo[5]) == 1;

        StringTools_freeStrings(texInfo, ni);
    }
    return m;
}

void MultyTexture_free(MultyTexture *self) {
    if (!self) return;
    free(self->textures);
    free(self);
}

void MultyTexture_updateAnimation(MultyTexture *self) {
    for (int i = 0; i < self->textures_n; i++) Texture_updateAnimation(self->textures[i]);
}

Texture *MultyTexture_texture(MultyTexture *self, int idx) {
    if (idx < 0 || idx >= self->textures_n) idx = 0;
    return self->textures[idx];
}
