/*
 * inifile.c — porte fiel de code/utils/IniFile.java (Quantum Engine J2ME)
 */
#include "inifile.h"
#include "stringtools.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ---- helpers ---- */

static void qe_grow(IniFile *self) {
    if (self->count + 1 <= self->capacity) return;
    self->capacity = self->capacity ? self->capacity * 2 : 8;
    self->entries = (qe_ini_entry*) realloc(self->entries, sizeof(qe_ini_entry) * self->capacity);
}

static qe_ini_entry *qe_find(IniFile *self, const char *key) {
    for (int i = 0; i < self->count; i++)
        if (strcmp(self->entries[i].key, key) == 0) return &self->entries[i];
    return NULL;
}

/* Java Hashtable.put substitui valor; aqui reproduzimos. */
static void qe_put_string(IniFile *self, const char *key, const char *val) {
    qe_ini_entry *e = qe_find(self, key);
    if (e) {
        if (e->kind == QE_INI_STRING) free(e->v.s);
        else if (e->kind == QE_INI_TABLE) IniFile_free(e->v.t);
        e->kind = QE_INI_STRING;
        e->v.s = strdup(val);
        return;
    }
    qe_grow(self);
    e = &self->entries[self->count++];
    e->key = strdup(key);
    e->kind = QE_INI_STRING;
    e->v.s = strdup(val);
}

static void qe_put_table(IniFile *self, const char *key, IniFile *tbl) {
    qe_ini_entry *e = qe_find(self, key);
    if (e) {
        if (e->kind == QE_INI_STRING) free(e->v.s);
        else if (e->kind == QE_INI_TABLE) IniFile_free(e->v.t);
        e->kind = QE_INI_TABLE;
        e->v.t = tbl;
        return;
    }
    qe_grow(self);
    e = &self->entries[self->count++];
    e->key = strdup(key);
    e->kind = QE_INI_TABLE;
    e->v.t = tbl;
}

/* strip leading/trailing space in a new heap string */
static char *qe_trim_dup(const char *s, int len) {
    while (len > 0 && isspace((unsigned char)*s)) { s++; len--; }
    while (len > 0 && isspace((unsigned char)s[len-1])) len--;
    char *out = (char*) malloc(len + 1);
    memcpy(out, s, len);
    out[len] = 0;
    return out;
}

/* ---- ctors ---- */

IniFile *IniFile_new(void) {
    IniFile *f = (IniFile*) calloc(1, sizeof(IniFile));
    f->owns_entries = 1;
    return f;
}

IniFile *IniFile_new_fromStr(const char *str, int useGroups) {
    IniFile *f = IniFile_new();
    int n;
    char **lines = StringTools_cutOnStrings(str, '\n', &n);
    IniFile_set(f, lines, n, useGroups);
    StringTools_freeStrings(lines, n);
    return f;
}

IniFile *IniFile_new_fromLines(char **lines, int nlines, int useGroups) {
    IniFile *f = IniFile_new();
    IniFile_set(f, lines, nlines, useGroups);
    return f;
}

IniFile *IniFile_createFromResource(const char *file) {
    return IniFile_createFromResourceKeys(file, 0);
}

IniFile *IniFile_createFromResourceKeys(const char *file, int useGroups) {
    char *content = StringTools_getStringFromResource(file);
    if (!content) return NULL;
    IniFile *f = IniFile_new_fromStr(content, useGroups);
    free(content);
    return f;
}

/* ---- static createGroups (versao com sub-inifile por [SECTION]) ---- */

void IniFile_createGroups(const char *file,
                          char ***out_names, IniFile ***out_groups, int *out_count) {
    char *content = StringTools_getStringFromResource(file);
    if (!content) { *out_names = NULL; *out_groups = NULL; *out_count = 0; return; }

    int nlines;
    char **lines = StringTools_cutOnStrings(content, '\n', &nlines);

    int cap = 8, cnt = 0;
    char    **names  = (char**)    malloc(sizeof(char*) * cap);
    IniFile **groups = (IniFile**) malloc(sizeof(IniFile*) * cap);
    IniFile  *cur = NULL;

    for (int i = 0; i < nlines; i++) {
        char *line = lines[i];
        int llen = (int) strlen(line);
        if (llen <= 0) continue;
        if (line[0] == '#' || line[0] == ';') continue;

        if (line[0] == '[') {
            if (cnt >= cap) {
                cap *= 2;
                names  = (char**)    realloc(names,  sizeof(char*) * cap);
                groups = (IniFile**) realloc(groups, sizeof(IniFile*) * cap);
            }
            /* substring(1, len-1) — remove '[' e ']' finais */
            int nl = llen - 2;
            if (nl < 0) nl = 0;
            char *nm = (char*) malloc(nl + 1);
            memcpy(nm, line + 1, nl);
            nm[nl] = 0;
            names[cnt]  = nm;
            cur = IniFile_new();
            groups[cnt] = cur;
            cnt++;
        } else {
            char *eq = strchr(line, '=');
            if (eq && cur) {
                int keylen = (int)(eq - line);
                char *k = qe_trim_dup(line, keylen);
                char *v = qe_trim_dup(eq + 1, llen - keylen - 1);
                qe_put_string(cur, k, v);
                free(k); free(v);
            }
        }
    }

    StringTools_freeStrings(lines, nlines);
    free(content);

    *out_names = names;
    *out_groups = groups;
    *out_count = cnt;
}

void IniFile_freeGroupsResult(char **names, IniFile **groups, int count) {
    for (int i = 0; i < count; i++) {
        free(names[i]);
        IniFile_free(groups[i]);
    }
    free(names);
    free(groups);
}

/* ---- set (parser principal) ---- */

void IniFile_set(IniFile *self, char **lines, int nlines, int useGroups) {
    IniFile *current = self;

    for (int i = 0; i < nlines; i++) {
        char *line = lines[i];
        int llen = (int) strlen(line);
        if (llen <= 0) continue;
        if (line[0] == '#' || line[0] == ';') continue;

        if (line[0] == '[' && useGroups) {
            int nl = llen - 2; if (nl < 0) nl = 0;
            char *group = (char*) malloc(nl + 1);
            memcpy(group, line + 1, nl);
            group[nl] = 0;

            IniFile *sub = IniFile_new();
            qe_put_table(self, group, sub); /* Java: hashtable.put(group, currentGroup) */
            current = sub;
            free(group);
        } else {
            char *eq = strchr(line, '=');
            if (eq) {
                int keylen = (int)(eq - line);
                char *k = qe_trim_dup(line, keylen);
                char *v = qe_trim_dup(eq + 1, llen - keylen - 1);
                qe_put_string(current, k, v);
                free(k); free(v);
            }
        }
    }
}

/* ---- destroy ---- */

void IniFile_free(IniFile *self) {
    if (!self) return;
    if (self->owns_entries) {
        for (int i = 0; i < self->count; i++) {
            free(self->entries[i].key);
            if (self->entries[i].kind == QE_INI_STRING) free(self->entries[i].v.s);
            else if (self->entries[i].kind == QE_INI_TABLE) IniFile_free(self->entries[i].v.t);
        }
        free(self->entries);
    }
    free(self);
}

/* ---- save ---- */

void IniFile_save(IniFile *self, FILE *stream) {
    for (int i = 0; i < self->count; i++) {
        qe_ini_entry *e = &self->entries[i];
        if (e->kind == QE_INI_TABLE) {
            fputc('[', stream);
            fputs(e->key, stream);
            fputs("]\n", stream);
            IniFile_save(e->v.t, stream);
        } else {
            fputs(e->key, stream);
            fputc('=', stream);
            fputs(e->v.s, stream);
            fputc('\n', stream);
        }
    }
}

/* ---- inspect ---- */

char **IniFile_keys(IniFile *self, int *out_count) {
    char **out = (char**) malloc(sizeof(char*) * (self->count > 0 ? self->count : 1));
    for (int i = 0; i < self->count; i++) out[i] = strdup(self->entries[i].key);
    *out_count = self->count;
    return out;
}

IniFile **IniFile_hashtables(IniFile *self, int *out_count) {
    IniFile **out = (IniFile**) malloc(sizeof(IniFile*) * (self->count > 0 ? self->count : 1));
    int n = 0;
    for (int i = 0; i < self->count; i++)
        if (self->entries[i].kind == QE_INI_TABLE)
            out[n++] = self->entries[i].v.t;
    *out_count = n;
    return out;
}

int IniFile_groupExists(IniFile *self, const char *group) {
    return qe_find(self, group) != NULL;
}

/* ---- put ---- */

void IniFile_put(IniFile *self, const char *key, const char *value) {
    qe_put_string(self, key, value);
}

void IniFile_putGroup(IniFile *self, const char *group, const char *key, const char *value) {
    qe_ini_entry *e = qe_find(self, group);
    if (e && e->kind == QE_INI_TABLE) {
        qe_put_string(e->v.t, key, value);
    } else {
        IniFile *n = IniFile_new();
        qe_put_string(n, key, value);
        qe_put_table(self, group, n);
    }
}

/* ---- get em grupo ---- */

const char *IniFile_get_g(IniFile *self, const char *group, const char *key) {
    qe_ini_entry *e = qe_find(self, group);
    if (e && e->kind == QE_INI_TABLE) {
        qe_ini_entry *e2 = qe_find(e->v.t, key);
        if (e2 && e2->kind == QE_INI_STRING) return e2->v.s;
    }
    return NULL;
}

const char *IniFile_getDef_g(IniFile *self, const char *group, const char *key, const char *def) {
    const char *v = IniFile_get_g(self, group, key);
    return v ? v : def;
}

int8_t IniFile_getByte_g(IniFile *self, const char *g, const char *k) {
    return StringTools_parseByte(IniFile_get_g(self, g, k));
}
float IniFile_getFloat_g(IniFile *self, const char *g, const char *k) {
    return StringTools_parseFloat(IniFile_get_g(self, g, k));
}
float IniFile_getFloatDef_g(IniFile *self, const char *g, const char *k, float def) {
    const char *v = IniFile_get_g(self, g, k);
    return v ? StringTools_parseFloat(v) : def;
}
int IniFile_getInt_g(IniFile *self, const char *g, const char *k) {
    return StringTools_parseInt(IniFile_get_g(self, g, k));
}
int IniFile_getIntDef_g(IniFile *self, const char *g, const char *k, int def) {
    const char *v = IniFile_get_g(self, g, k);
    return v ? StringTools_parseInt(v) : def;
}
int64_t IniFile_getLong_g(IniFile *self, const char *g, const char *k) {
    return StringTools_parseLong(IniFile_get_g(self, g, k));
}

/* ---- get top-level ---- */

const char *IniFile_get(IniFile *self, const char *key) {
    qe_ini_entry *e = qe_find(self, key);
    return (e && e->kind == QE_INI_STRING) ? e->v.s : NULL;
}
const char *IniFile_getDef(IniFile *self, const char *key, const char *def) {
    const char *v = IniFile_get(self, key);
    return v ? v : def;
}
int8_t IniFile_getByte(IniFile *self, const char *k)   { return StringTools_parseByte(IniFile_get(self, k)); }
float  IniFile_getFloat(IniFile *self, const char *k)  { return StringTools_parseFloat(IniFile_get(self, k)); }
float  IniFile_getFloatDef(IniFile *self, const char *k, float def) {
    const char *v = IniFile_get(self, k);
    return v ? StringTools_parseFloat(v) : def;
}
int    IniFile_getInt(IniFile *self, const char *k)    { return StringTools_parseInt(IniFile_get(self, k)); }
int    IniFile_getIntDef(IniFile *self, const char *k, int def) {
    const char *v = IniFile_get(self, k);
    return v ? StringTools_parseInt(v) : def;
}
int64_t IniFile_getLong(IniFile *self, const char *k)  { return StringTools_parseLong(IniFile_get(self, k)); }
