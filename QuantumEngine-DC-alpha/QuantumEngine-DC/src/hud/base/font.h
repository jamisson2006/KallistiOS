/*
 * font.h — porte fiel de code/HUD/Base/Font.java
 *
 * Renderiza texto usando uma fonte em textura (PNG). Suporta 3 imagens
 * (normal / selected / active) e mapeamento manual via coords em ini,
 * ou auto-deteccao a partir de linha superior separadora (col == -16777216).
 */
#ifndef QE_HUD_BASE_FONT_H
#define QE_HUD_BASE_FONT_H

typedef struct Image    Image;
typedef struct Graphics Graphics;

typedef struct Font {
    int     space;      /* largura do espaco */
    Image  *img, *selimg, *actimg;
    int    *coords;
    int     coords_n;
    int    *chars;      /* Java: char[] — armazenamos code points como int */
    int     chars_n;
    int     y_src;
} Font;

Font *Font_new(const char *file);
void  Font_free(Font *self);

int   Font_getCode  (const Font *self, int ch);
int   Font_charWidth(const Font *self, int ch);
void  Font_setY     (Font *self, int val);

/* drawString: anchor bits — 8=RIGHT, 32=BOTTOM, 1=HCENTER, 2=VCENTER,
 * 64=BASELINE. col: 0=img, 1=selimg, 2=actimg. */
void  Font_drawString    (Font *self, Graphics *g, const char *str, int x, int y, int anchor);
void  Font_drawStringCol (Font *self, Graphics *g, const char *str, int x, int y, int anchor, int col);

int   Font_widthOfCharG  (const Font *self, int ch, Graphics *g);
int   Font_widthOfChar   (const Font *self, int ch);
int   Font_widthOfG      (const Font *self, const char *str, Graphics *g);
int   Font_widthOf       (const Font *self, const char *str);
int   Font_height        (const Font *self);
int   Font_height2       (const Font *self, const char *str, Graphics *g);

#endif
