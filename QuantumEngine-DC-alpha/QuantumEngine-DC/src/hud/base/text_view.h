/*
 * text_view.h — porte fiel de code/HUD/Base/TextView.java
 * @author DDDENISSS
 *
 * Quebra texto em linhas cabendo em uma largura w. Suporta '*' como
 * quebra manual. Scroll vertical via yOffset.
 */
#ifndef QE_HUD_BASE_TEXT_VIEW_H
#define QE_HUD_BASE_TEXT_VIEW_H

typedef struct Font     Font;
typedef struct Graphics Graphics;

typedef struct TextView {
    char   **lines;      /* array de linhas heap */
    int      lines_n;
    int      lines_cap;
    Font    *font;
    int      w, h;
    int      center;
    int      yOffset;
} TextView;

#define QE_TEXT_VIEW_INDENT 3

TextView *TextView_new(const char *str, int w, int h, Font *font);
void      TextView_free(TextView *self);

/* Quebra str em linhas cabendo em w usando font, appendando ao array. */
void      TextView_createLines_static(const char *txt, TextView *tv);
void      TextView_createLines(const char *txt, char ***out_lines, int *out_n, int *out_cap,
                                Font *font, int w);

void      TextView_addString(TextView *self, const char *str);
void      TextView_setString(TextView *self, const char *str);

void      TextView_paint(TextView *self, Graphics *g, int x, int y);
void      TextView_move (TextView *self, int dy);

Font     *TextView_getFont       (const TextView *self);
int       TextView_getCountString(const TextView *self);
int       TextView_getLineHeight (const TextView *self);
int       TextView_getTextHeight (const TextView *self);
int       TextView_getY          (const TextView *self);
void      TextView_setY          (TextView *self, int y);
int       TextView_getCenter     (const TextView *self);
void      TextView_setCenter     (TextView *self, int c);
int       TextView_getWidth      (const TextView *self);
int       TextView_getHeight     (const TextView *self);
void      TextView_setHeight     (TextView *self, int h);

#endif
