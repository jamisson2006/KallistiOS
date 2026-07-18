/*
 * gameini.c — porte fiel de code/utils/GameIni.java (Quantum Engine J2ME)
 */
#include "gameini.h"
#include "stringtools.h"

#include <stdlib.h>
#include <string.h>

/* Forward-declarado em main.h. Retorna NULL antes do idioma ser carregado. */
extern IniFile *Main_getGameText(void);

GameIni *GameIni_createFromResource(const char *file) {
    return GameIni_createFromResourceKeys(file, 0);
}

GameIni *GameIni_createFromResourceKeys(const char *file, int useGroups) {
    char *content = StringTools_getStringFromResource(file);
    if (!content) return NULL;
    GameIni *g = IniFile_new_fromStr(content, useGroups);
    free(content);
    return g;
}

void GameIni_createGroups(const char *file,
                          char ***out_names, GameIni ***out_groups, int *out_count) {
    /* No Java, apenas encaminha para IniFile.createGroups mas envolve
     * cada IniFile em GameIni (que em nosso port eh o mesmo tipo). */
    IniFile_createGroups(file, out_names, (IniFile***)out_groups, out_count);
}

char **GameIni_cutOnStrings(const char *str, char d, char d2, int *out_count) {
    if (!str) { *out_count = 0; return NULL; }
    char d3 = d;
    if (strchr(str, d2) != NULL) d3 = d2;
    return StringTools_cutOnStrings(str, d3, out_count);
}

int *GameIni_cutOnInts(const char *str, char d, char d2, int *out_count) {
    if (!str) { *out_count = 0; return NULL; }
    char d3 = d;
    if (strchr(str, d2) != NULL) d3 = d2;
    return GameIni_createPos(str, d3, out_count);
}

int *GameIni_createPos(const char *str, char d, int *out_count) {
    if (!str) { *out_count = 0; return NULL; }

    int n;
    char **tmp = StringTools_cutOnStrings(str, d, &n);
    int *out = (int*) malloc(sizeof(int) * (n > 0 ? n : 1));

    for (int i = 0; i < n; i++) {
        char *s = tmp[i];
        /* strip ';' das bordas (fiel ao while do Java) */
        while (*s == ';') s++;
        int len = (int) strlen(s);
        while (len > 0 && s[len-1] == ';') { s[len-1] = 0; len--; }
        out[i] = StringTools_parseInt(s);
    }

    StringTools_freeStrings(tmp, n);
    *out_count = n;
    return out;
}

const char *GameIni_getNoLang(GameIni *self, const char *key) {
    return IniFile_get(self, key);
}
const char *GameIni_getNoLangDef(GameIni *self, const char *key, const char *def) {
    return IniFile_getDef(self, key, def);
}
int GameIni_getIntNoLangDef(GameIni *self, const char *key, int def) {
    return IniFile_getIntDef(self, key, def);
}

/* Java:
 *   String out = super.get(key);
 *   if (out != null) return Main.getGameText()==null?out:Main.getGameText().getDef(out,out);
 *   else return def;
 */
const char *GameIni_getDef(GameIni *self, const char *key, const char *def) {
    const char *out = IniFile_get(self, key);
    if (out) {
        IniFile *txt = Main_getGameText();
        if (!txt) return out;
        return IniFile_getDef(txt, out, out);
    }
    return def;
}

const char *GameIni_get(GameIni *self, const char *key) {
    return GameIni_getDef(self, key, NULL);
}

const char *GameIni_getDef_g(GameIni *self, const char *group, const char *key, const char *def) {
    const char *out = IniFile_get_g(self, group, key);
    if (out) {
        IniFile *txt = Main_getGameText();
        if (!txt) return out;
        return IniFile_getDef(txt, out, out);
    }
    return def;
}

const char *GameIni_get_g(GameIni *self, const char *group, const char *key) {
    return GameIni_getDef_g(self, group, key, NULL);
}

int GameIni_startsWith(const char *str, char **list, int list_count) {
    for (int i = 0; i < list_count; i++) {
        const char *p = list[i];
        size_t pl = strlen(p);
        if (strncmp(str, p, pl) == 0) return i;
    }
    return -1;
}
