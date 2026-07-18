/*
 * item_list.h — porte fiel de code/HUD/Base/ItemList.java
 */
#ifndef QE_HUD_BASE_ITEM_LIST_H
#define QE_HUD_BASE_ITEM_LIST_H

typedef struct Font     Font;
typedef struct Graphics Graphics;

typedef struct ItemList {
    Font   *font;
    char  **items;
    int     items_n;
    int    *midSel;    /* boolean[] Java */
    int     midSel_n;
    int     left;
    int     redact;
    int     index;
} ItemList;

ItemList *ItemList_new_font  (char **items, int n, Font *font);
ItemList *ItemList_new_ms    (char **items, int n, Font *font, int *midSel, int ms_n);
ItemList *ItemList_new       (char **items, int n);
void      ItemList_free(ItemList *self);

void      ItemList_setFont(ItemList *self, Font *font);
int       ItemList_getHeight(const ItemList *self);
void      ItemList_draw    (ItemList *self, Graphics *g, int x, int y, int w, int h);
void      ItemList_drawBck (ItemList *self, Graphics *g, int x, int y, int w, int h);
int       ItemList_getPosY (const ItemList *self, int h);

void      ItemList_scrollDown(ItemList *self);
void      ItemList_scrollUp  (ItemList *self);

int       ItemList_getIndex(const ItemList *self);
void      ItemList_setIndex(ItemList *self, int i);
const char *ItemList_getCurrentItem(const ItemList *self);
char    **ItemList_getItems(const ItemList *self, int *out_n);

#endif
