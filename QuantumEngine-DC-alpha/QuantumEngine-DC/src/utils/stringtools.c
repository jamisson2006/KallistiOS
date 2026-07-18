/*
 * stringtools.c — porte fiel de code/utils/StringTools.java (Quantum Engine J2ME)
 */
#include "stringtools.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
 * Java abre recursos via getResourceAsStream(file). No port DC (KOS), os
 * recursos moram no romdisk montado em /rd/. Para paths comecando com '/'
 * traduzimos para "/rd" + path — fielmente ao mesmo esquema em src/core/res
 * do restante do engine. */
static void qe_res_path(const char *file, char *out, size_t out_sz) {
    if (file[0] == '/') snprintf(out, out_sz, "/rd%s", file);
    else                snprintf(out, out_sz, "/rd/%s", file);
}

char *StringTools_getStringFromResource(const char *file) {
    char path[512];
    qe_res_path(file, path, sizeof path);

    FILE *fp = fopen(path, "rb");
    if (!fp) {
        fprintf(stderr, "Reading txt error: %s\n", path);
        return NULL;
    }

    /* Buffer crescente. */
    size_t cap = 256, len = 0;
    char *buf = (char*) malloc(cap);
    if (!buf) { fclose(fp); return NULL; }

    int ch;
    while ((ch = fgetc(fp)) != EOF) {
        if (ch == '\r') continue;
        if (ch >= 192 && ch <= 255) ch = ch + 848; /* windows 1251 to utf8 */

        /* No Java, StringBuffer.append((char)ch) escreve UTF-16; ao serializar
         * .toString() vira UTF-8 se necessario. Aqui escrevemos direto UTF-8. */
        char utf8[4];
        int nb;
        if (ch < 0x80)        { utf8[0] = (char)ch; nb = 1; }
        else if (ch < 0x800)  { utf8[0] = (char)(0xC0|(ch>>6));  utf8[1] = (char)(0x80|(ch&0x3F)); nb = 2; }
        else                  { utf8[0] = (char)(0xE0|(ch>>12)); utf8[1] = (char)(0x80|((ch>>6)&0x3F)); utf8[2] = (char)(0x80|(ch&0x3F)); nb = 3; }

        while (len + nb + 1 > cap) {
            cap *= 2;
            char *nb2 = (char*) realloc(buf, cap);
            if (!nb2) { free(buf); fclose(fp); return NULL; }
            buf = nb2;
        }
        for (int i = 0; i < nb; i++) buf[len++] = utf8[i];
    }
    buf[len] = 0;
    fclose(fp);
    return buf;
}

/* trim in-place (retorna novo ptr dentro de s, altera final) */
static char *qe_trim(char *s) {
    while (*s && isspace((unsigned char)*s)) s++;
    size_t l = strlen(s);
    while (l > 0 && isspace((unsigned char)s[l-1])) s[--l] = 0;
    return s;
}

/* Java StringTools.fragments(): corta em pedacos delimitados por 'd',
 * pulando tokens vazios. Analogo a addElement quando start<end. */
static char **qe_fragments(const char *text, char divider, int *count) {
    int cap = 8, n = 0;
    char **arr = (char**) malloc(sizeof(char*) * cap);
    if (!arr) { *count = 0; return NULL; }

    int len = (int) strlen(text);
    int start = 0;
    while (start < len) {
        int end = start;
        while (end < len && text[end] != divider) end++;

        if (start < end) {
            if (n >= cap) {
                cap *= 2;
                char **na = (char**) realloc(arr, sizeof(char*) * cap);
                if (!na) { for (int i=0;i<n;i++) free(arr[i]); free(arr); *count=0; return NULL; }
                arr = na;
            }
            int flen = end - start;
            char *frag = (char*) malloc(flen + 1);
            memcpy(frag, text + start, flen);
            frag[flen] = 0;
            arr[n++] = frag;
        }
        start = end + 1;
    }

    *count = n;
    return arr;
}

char **StringTools_cutOnStrings(const char *str, char d, int *out_count) {
    if (str == NULL) { *out_count = 0; return NULL; }
    return qe_fragments(str, d, out_count);
}

void StringTools_freeStrings(char **arr, int count) {
    if (!arr) return;
    for (int i = 0; i < count; i++) free(arr[i]);
    free(arr);
}

int *StringTools_cutOnInts(const char *str, char d, int *out_count) {
    if (str == NULL) { *out_count = 0; return NULL; }
    int n; char **frags = qe_fragments(str, d, &n);
    int *out = (int*) malloc(sizeof(int) * (n > 0 ? n : 1));
    for (int i = 0; i < n; i++) out[i] = StringTools_parseInt(frags[i]);
    StringTools_freeStrings(frags, n);
    *out_count = n;
    return out;
}

float *StringTools_cutOnFloats(const char *str, char d, int *out_count) {
    if (str == NULL) { *out_count = 0; return NULL; }
    int n; char **frags = qe_fragments(str, d, &n);
    float *out = (float*) malloc(sizeof(float) * (n > 0 ? n : 1));
    for (int i = 0; i < n; i++) out[i] = StringTools_parseFloat(frags[i]);
    StringTools_freeStrings(frags, n);
    *out_count = n;
    return out;
}

float StringTools_parseFloat(const char *val) {
    char *dup = strdup(val); char *t = qe_trim(dup);
    float f = (float) atof(t);
    free(dup);
    return f;
}

int StringTools_parseInt(const char *val) {
    char *dup = strdup(val); char *t = qe_trim(dup);
    int v = atoi(t);
    free(dup);
    return v;
}

int StringTools_cleverParseInt(const char *val) {
    char *dup = strdup(val); char *t = qe_trim(dup);
    char *dot = strchr(t, '.');
    if (dot) *dot = 0; /* floating point -> integer */
    int v = atoi(t);
    free(dup);
    return v;
}

int8_t StringTools_parseByte(const char *val) {
    return (int8_t) StringTools_parseInt(val);
}

int64_t StringTools_parseLong(const char *val) {
    char *dup = strdup(val); char *t = qe_trim(dup);
    int64_t v = (int64_t) strtoll(t, NULL, 10);
    free(dup);
    return v;
}

int StringTools_isNumeric(const char *s) {
    if (s == NULL) return 0;
    char *dup = strdup(s); char *t = qe_trim(dup);
    int ok = 1, len = (int) strlen(t);
    for (int i = 0; i < len; i++) {
        char ch = t[i];
        if (ch != '-' && !isdigit((unsigned char)ch)) { ok = 0; break; }
    }
    free(dup);
    return ok;
}

char *StringTools_deleteNonNumeric(const char *s) {
    if (!s) return NULL;
    /* corta prefixos nao [-0-9] */
    int len = (int) strlen(s);
    int start = 0;
    while (start < len && !isdigit((unsigned char)s[start]) && s[start] != '-') start++;
    int end = len;
    while (end > start && !isdigit((unsigned char)s[end-1])) end--;
    int nlen = end - start;
    char *out = (char*) malloc(nlen + 1);
    memcpy(out, s + start, nlen);
    out[nlen] = 0;
    return out;
}

int StringTools_getRGB(const char *rgbs, char div) {
    int n;
    int *rgb = StringTools_cutOnInts(rgbs, div, &n);
    int col = (rgb[0] << 16) | (rgb[1] << 8) | rgb[2];
    free(rgb);
    return col;
}

int StringTools_parseXMLColor(const char *argb_in) {
    /* Java toUpperCase + loop de i=1 (pula o '#') */
    int len = (int) strlen(argb_in);
    int col = 0;
    for (int i = 1; i < len; i++) {
        char ch = (char) toupper((unsigned char) argb_in[i]);
        int val;
        if (isdigit((unsigned char)ch)) val = ch - '0';
        else val = ch - 'A' + 10;
        col |= val << ((6 - i) * 4);
    }
    return col;
}
