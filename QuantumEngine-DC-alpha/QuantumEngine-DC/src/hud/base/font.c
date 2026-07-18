/*
 * font.c — porte fiel de code/HUD/Base/Font.java
 */
#include "font.h"
#include "../../utils/inifile.h"
#include "../../utils/stringtools.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* externals — Graphics/Image proxies (a converter em modulo de renderizacao 2D). */
extern Image *Image_createImage(const char *file);
extern Image *Image_createRegion(Image *src, int x, int y, int w, int h, int transform);
extern int    Image_getHeight(Image *img);
extern int    Image_getWidth(Image *img);
extern void   Image_getRGB(Image *img, int *dst, int off, int stride, int x, int y, int w, int h);

extern void Graphics_drawRegion(Graphics *g, Image *img, int xs, int ys, int w, int h, int transform, int xd, int yd, int anchor);
extern void Graphics_setColor(Graphics *g, int c);
extern void Graphics_drawChar(Graphics *g, int ch, int x, int y, int anchor);
extern int  Graphics_fontCharWidth(Graphics *g, int ch);
extern int  Graphics_fontHeight(Graphics *g);

/* Anchor bits do MIDP 2.0 */
#define QE_ANCHOR_HCENTER 1
#define QE_ANCHOR_VCENTER 2
#define QE_ANCHOR_LEFT    4
#define QE_ANCHOR_RIGHT   8
#define QE_ANCHOR_TOP     16
#define QE_ANCHOR_BOTTOM  32
#define QE_ANCHOR_BASELINE 64

static int indexOf(const Font *self, int ch) {
    for (int i = 0; i < self->chars_n; i++) if (self->chars[i] == ch) return i;
    return -1;
}

Font *Font_new(const char *file) {
    Font *f = (Font*) calloc(1, sizeof(Font));
    IniFile *str = IniFile_createFromResource(file);
    if (!str) return f;

    const char *var2    = IniFile_get(str, "IMG");
    const char *sel_font= IniFile_get(str, "SELECTED_IMG");
    const char *act_font= IniFile_get(str, "ACTIVE_IMG");
    if (!act_font) act_font = var2;
    if (!sel_font) sel_font = var2;

    f->selimg = Image_createImage(sel_font);
    f->actimg = Image_createImage(act_font);

    const char *chars = IniFile_get(str, "CHARS");
    if (chars) {
        f->chars_n = (int) strlen(chars);
        f->chars   = (int*) malloc(sizeof(int) * f->chars_n);
        for (int i = 0; i < f->chars_n; i++) f->chars[i] = (unsigned char) chars[i];
    }

    const char *coords_s = IniFile_get(str, "COORDS");
    if (coords_s) {
        f->img = Image_createImage(var2);
        f->coords = StringTools_cutOnInts(coords_s, ',', &f->coords_n);
        f->space  = IniFile_getInt(str, "SPACE");
    } else {
        Image *tImg = Image_createImage(var2);
        f->img    = Image_createRegion(tImg, 0, 1, Image_getWidth(tImg), Image_getHeight(tImg) - 1, 0);
        f->selimg = Image_createRegion(f->selimg, 0, 1, Image_getWidth(f->selimg), Image_getHeight(f->selimg) - 1, 0);
        f->actimg = Image_createRegion(f->actimg, 0, 1, Image_getWidth(f->actimg), Image_getHeight(f->actimg) - 1, 0);

        int w = Image_getWidth(tImg);
        int *line = (int*) malloc(sizeof(int) * w);
        Image_getRGB(tImg, line, 0, w, 0, 0, w, 1);

        /* Coleta coords a partir de separadores (col == -16777216 = 0xFF000000). */
        int cap = 32, cnt = 0;
        int *coords = (int*) malloc(sizeof(int) * cap);
        coords[cnt++] = 0;
        for (int i = 0; i < w; i++) {
            if (line[i] == -16777216) {
                if (cnt >= cap) { cap *= 2; coords = (int*) realloc(coords, sizeof(int) * cap); }
                coords[cnt++] = i;
            }
        }
        if (cnt >= cap) { cap *= 2; coords = (int*) realloc(coords, sizeof(int) * cap); }
        coords[cnt++] = Image_getWidth(f->img);
        f->coords   = coords;
        f->coords_n = cnt;
        free(line);

        f->space = 0;
        for (int i = 0; i < f->chars_n; i++) f->space += Font_charWidth(f, f->chars[i]);
        if (f->chars_n > 0) f->space /= f->chars_n;
    }
    IniFile_free(str);
    return f;
}

void Font_free(Font *self) {
    if (!self) return;
    free(self->coords);
    free(self->chars);
    free(self);
}

int Font_getCode(const Font *self, int ch) { return indexOf(self, ch); }

int Font_charWidth(const Font *self, int ch) {
    if (ch == ' ') return self->space;
    int code = indexOf(self, ch);
    if (code == -1) return self->space;
    int b = self->coords[code];
    int e = self->coords[code + 1];
    return e - b;
}

void Font_setY(Font *self, int val) { (void) val; self->y_src = 0; }

void Font_drawString(Font *self, Graphics *g, const char *str, int x, int y, int anchor) {
    Font_drawStringCol(self, g, str, x, y, anchor, 0);
}

void Font_drawStringCol(Font *self, Graphics *g, const char *str, int x, int y, int anchor, int col) {
    Image *im = self->img;
    if (col == 1) im = self->selimg;
    if (col == 2) im = self->actimg;

    int var8 = x, var9 = y;
    int strlen_i = (int) strlen(str);
    int w = Font_widthOfG(self, str, g);
    int h = Font_height(self);

    if (anchor & QE_ANCHOR_RIGHT)    var8 = x - w;
    if (anchor & QE_ANCHOR_BOTTOM)   var9 = y - h;
    if (anchor & QE_ANCHOR_HCENTER)  var8 -= w >> 1;
    if (anchor & QE_ANCHOR_VCENTER)  var9 -= h >> 1;
    if (anchor & QE_ANCHOR_BASELINE) var9 -= h + 1;

    int imgH = Image_getHeight(self->img);
    int ysrcH = self->y_src * imgH;

    for (int i = 0; i < strlen_i; i++) {
        int ch = (unsigned char) str[i];
        int idx = indexOf(self, ch);
        if (ch == 32 && idx == -1) {
            var8 += self->space;
        } else if (idx == -1) {
            Graphics_setColor(g, 0xffffff);
            Graphics_drawChar(g, ch, var8, var9 + imgH, QE_ANCHOR_BOTTOM | QE_ANCHOR_LEFT);
            var8 += Graphics_fontCharWidth(g, ch);
        } else {
            int var11 = self->coords[idx];
            int var17 = self->coords[idx + 1] - var11;
            Graphics_drawRegion(g, im, var11, ysrcH, var17, imgH, 0, var8, var9, 0);
            var8 += var17;
        }
    }
}

int Font_widthOfCharG(const Font *self, int ch, Graphics *g) {
    int idx = indexOf(self, ch);
    if (ch == 32 && idx == -1) return self->space;
    if (idx == -1) return Graphics_fontCharWidth(g, ch);
    return self->coords[idx + 1] - self->coords[idx];
}

int Font_widthOfChar(const Font *self, int ch) {
    int idx = indexOf(self, ch);
    if (ch == 32 && idx == -1) return self->space;
    if (idx == -1) return self->space;
    return self->coords[idx + 1] - self->coords[idx];
}

int Font_widthOfG(const Font *self, const char *str, Graphics *g) {
    int w = 0, len = (int) strlen(str);
    for (int i = 0; i < len; i++) w += Font_widthOfCharG(self, (unsigned char) str[i], g);
    return w;
}
int Font_widthOf(const Font *self, const char *str) {
    int w = 0, len = (int) strlen(str);
    for (int i = 0; i < len; i++) w += Font_widthOfChar(self, (unsigned char) str[i]);
    return w;
}

int Font_height(const Font *self) { return Image_getHeight(self->img); }

int Font_height2(const Font *self, const char *str, Graphics *g) {
    int stdh = Image_getHeight(self->img);
    int len = (int) strlen(str);
    for (int i = 0; i < len; i++) {
        int ch = (unsigned char) str[i];
        if (indexOf(self, ch) == -1 && ch != 32) {
            int h = Graphics_fontHeight(g);
            if (h > stdh) return h;
        }
    }
    return stdh;
}
