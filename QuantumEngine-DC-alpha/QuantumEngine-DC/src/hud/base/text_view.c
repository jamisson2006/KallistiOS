/*
 * text_view.c — porte fiel de code/HUD/Base/TextView.java
 */
#include "text_view.h"
#include "font.h"

#include <stdlib.h>
#include <string.h>

extern int  Graphics_getClipX(Graphics *g);
extern int  Graphics_getClipY(Graphics *g);
extern int  Graphics_getClipWidth(Graphics *g);
extern int  Graphics_getClipHeight(Graphics *g);
extern void Graphics_setClip(Graphics *g, int x, int y, int w, int h);

static inline int qe_max(int a, int b) { return a > b ? a : b; }
static inline int qe_min(int a, int b) { return a < b ? a : b; }

static void append_line(char ***arr, int *n, int *cap, const char *s, int len) {
    if (*n >= *cap) {
        *cap = *cap ? *cap * 2 : 8;
        *arr = (char**) realloc(*arr, sizeof(char*) * *cap);
    }
    char *o = (char*) malloc(len + 1);
    memcpy(o, s, len); o[len] = 0;
    (*arr)[(*n)++] = o;
}

void TextView_createLines(const char *txt, char ***out_lines, int *out_n, int *out_cap,
                          Font *font, int w) {
    int wordWidth = 0;
    int wordStart = 0;
    int lastSpace = -1;
    int len = (int) strlen(txt);

    int i = 0;
    while (i < len) {
        int ch = (unsigned char) txt[i];
        if (ch == ' ') lastSpace = i;
        int wordEnd = -1;

        if (ch == '*') {
            wordEnd = i;
            i++;
        } else if (wordWidth + Font_charWidth(font, ch) > w) {
            if (lastSpace != -1) { i = lastSpace + 1; wordEnd = lastSpace; }
            else                 { wordEnd = i; }
        }

        if (wordEnd != -1) {
            append_line(out_lines, out_n, out_cap, txt + wordStart, wordEnd - wordStart);
            wordWidth = 0;
            wordStart = i;
        } else {
            wordWidth += Font_charWidth(font, ch);
            i++;
        }
    }
    if (wordStart < len) append_line(out_lines, out_n, out_cap, txt + wordStart, len - wordStart);
}

void TextView_createLines_static(const char *txt, TextView *tv) {
    TextView_createLines(txt, &tv->lines, &tv->lines_n, &tv->lines_cap, tv->font, tv->w);
}

TextView *TextView_new(const char *str, int w, int h, Font *font) {
    TextView *tv = (TextView*) calloc(1, sizeof(TextView));
    tv->font = font;
    tv->w = w; tv->h = h;
    if (str) TextView_createLines_static(str, tv);
    return tv;
}

void TextView_free(TextView *tv) {
    if (!tv) return;
    for (int i = 0; i < tv->lines_n; i++) free(tv->lines[i]);
    free(tv->lines);
    free(tv);
}

void TextView_addString(TextView *tv, const char *str) { TextView_createLines_static(str, tv); }

void TextView_setString(TextView *tv, const char *str) {
    for (int i = 0; i < tv->lines_n; i++) free(tv->lines[i]);
    tv->lines_n = 0;
    TextView_createLines_static(str, tv);
}

void TextView_paint(TextView *tv, Graphics *g, int x, int y) {
    int clipX = Graphics_getClipX(g);
    int clipY = Graphics_getClipY(g);
    int clipW = Graphics_getClipWidth(g);
    int clipH = Graphics_getClipHeight(g);
    Graphics_setClip(g, qe_max(clipX, x), qe_max(clipY, y),
                        qe_min(clipW, tv->w), qe_min(clipH, tv->h));

    int stepY = TextView_getLineHeight(tv);
    int posY = tv->yOffset;
    for (int i = 0; i < tv->lines_n; i++) {
        if (posY + stepY >= 0) {
            if (posY > tv->h) break;
            const char *str = tv->lines[i];
            int posX = tv->center ? (tv->w - Font_widthOf(tv->font, str)) >> 1 : 0;
            Font_drawString(tv->font, g, str, posX + x, posY + y, 0);
        }
        posY += stepY;
    }
    Graphics_setClip(g, clipX, clipY, clipW, clipH);
}

void TextView_move(TextView *tv, int dy) {
    tv->yOffset += dy;
    int textHeight = TextView_getTextHeight(tv);
    if (textHeight > tv->h) {
        if (tv->yOffset > 0) tv->yOffset = 0;
        if (tv->yOffset + textHeight < tv->h) tv->yOffset = tv->h - textHeight;
    } else {
        if (tv->yOffset < 0) tv->yOffset = 0;
        if (tv->yOffset + textHeight > tv->h) tv->yOffset = tv->h - textHeight;
    }
    if (textHeight + tv->yOffset < tv->h && tv->yOffset > 0) tv->yOffset = 0;
}

Font *TextView_getFont(const TextView *tv)        { return tv->font; }
int   TextView_getCountString(const TextView *tv) { return tv->lines_n; }
int   TextView_getLineHeight(const TextView *tv)  { return Font_height(tv->font) + QE_TEXT_VIEW_INDENT; }
int   TextView_getTextHeight(const TextView *tv)  { return TextView_getLineHeight(tv) * tv->lines_n - QE_TEXT_VIEW_INDENT; }
int   TextView_getY(const TextView *tv)           { return tv->yOffset; }
void  TextView_setY(TextView *tv, int y)          { tv->yOffset = y; }
int   TextView_getCenter(const TextView *tv)      { return tv->center; }
void  TextView_setCenter(TextView *tv, int c)     { tv->center = c; }
int   TextView_getWidth(const TextView *tv)       { return tv->w; }
int   TextView_getHeight(const TextView *tv)      { return tv->h; }
void  TextView_setHeight(TextView *tv, int h)     { tv->h = h; }
