/*
 * item_list.c — porte fiel de code/HUD/Base/ItemList.java
 */
#include "item_list.h"
#include "font.h"
#include "../../utils/main.h"

#include <stdlib.h>

ItemList *ItemList_new_font(char **items, int n, Font *font) {
    ItemList *l = (ItemList*) calloc(1, sizeof(ItemList));
    l->items = items; l->items_n = n; l->font = font;
    return l;
}
ItemList *ItemList_new_ms(char **items, int n, Font *font, int *midSel, int ms_n) {
    ItemList *l = ItemList_new_font(items, n, font);
    l->midSel = midSel; l->midSel_n = ms_n;
    return l;
}
ItemList *ItemList_new(char **items, int n) {
    ItemList *l = (ItemList*) calloc(1, sizeof(ItemList));
    l->items = items; l->items_n = n;
    return l;
}
void ItemList_free(ItemList *l) { free(l); }

void ItemList_setFont(ItemList *l, Font *font) { l->font = font; }
int  ItemList_getHeight(const ItemList *l) { return l->items_n * (Font_height(l->font) + 3); }

void ItemList_draw(ItemList *l, Graphics *g, int x, int y, int w, int h) {
    int fontHeight = Font_height(l->font);
    int stepY = fontHeight + 3;
    int max = h / stepY;
    int less = l->items_n <= max;
    int posy = y;
    int i = 0;
    if (less) posy += h / 2 - l->items_n * stepY / 2;
    else {
        i = l->index - max / 2;
        if (i < 0) i = 0;
        if (i >= l->items_n - max) i = l->items_n - max;
    }
    for (; i < l->items_n; i++) {
        if (posy < y) continue;
        if (posy + fontHeight > y + h) break;
        const char *str = l->items[i];
        int x2 = w / 2 - Font_widthOf(l->font, str) / 2 + x;
        int ms = (l->midSel != NULL && i < l->midSel_n && l->midSel[i]);
        if (l->left && !ms) x2 = x;
        int col = (l->redact && i == l->index) ? 2 : ((i == l->index || ms) ? 1 : 0);
        Font_drawStringCol(l->font, g, str, x2, posy, 0, col);
        posy += stepY;
    }
}

void ItemList_drawBck(ItemList *l, Graphics *g, int x, int y, int w, int h) {
    (void) x; (void) w;
    int fontHeight = Font_height(l->font);
    int stepY = fontHeight + 3;
    int max = h / stepY;
    int less = l->items_n <= max;
    int posy = y;
    int i = 0;
    if (less) posy += h / 2 - l->items_n * stepY / 2;
    else {
        i = l->index - max / 2;
        if (i < 0) i = 0;
        if (i >= l->items_n - max) i = l->items_n - max;
    }
    int posy2 = posy;
    for (; i < l->items_n; i++) {
        if (posy2 < y) continue;
        if (posy2 + fontHeight > y + h) break;
        posy2 += stepY;
    }
    Main_drawBckDialog(g, posy, posy2 - 3);
}

int ItemList_getPosY(const ItemList *l, int h) {
    int stepY = Font_height(l->font) + 3;
    int max = h / stepY;
    int less = l->items_n <= max;
    int posy = 0;
    int i = 0;
    if (less) posy += h / 2 - l->items_n * stepY / 2;
    else {
        i = l->index - max / 2;
        if (i < 0) i = 0;
        if (i >= l->items_n - max) i = l->items_n - max;
    }
    return posy - i * stepY;
}

void ItemList_scrollDown(ItemList *l) { l->index++; l->index %= l->items_n; }
void ItemList_scrollUp  (ItemList *l) { l->index--; if (l->index < 0) l->index = l->items_n - 1; }

int   ItemList_getIndex(const ItemList *l) { return l->index; }
void  ItemList_setIndex(ItemList *l, int i) { l->index = i; }
const char *ItemList_getCurrentItem(const ItemList *l) { return l->items[l->index]; }
char **ItemList_getItems(const ItemList *l, int *out_n) { *out_n = l->items_n; return l->items; }
