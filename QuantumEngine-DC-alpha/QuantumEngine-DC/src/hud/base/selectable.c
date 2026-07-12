/*
 * selectable.c — porte fiel de code/HUD/Base/Selectable.java
 */
#include "selectable.h"
#include "font.h"
#include "../../utils/main.h"
#include "../../utils/canvas/my_canvas.h"

#include <stdlib.h>

void Selectable_set(Selectable *self, Font *font, char **items, int items_n,
                    const char *leftSoft, const char *rightSoft) {
    self->list = ItemList_new(items, items_n);
    ItemList_setFont(self->list, font);
    GUIScreen_setFont(&self->base, font);
    GUIScreen_setSoftKeysNames(&self->base, leftSoft, rightSoft);
}

void Selectable_set_ms(Selectable *self, Font *font, char **items, int items_n,
                       const char *leftSoft, const char *rightSoft, int *midSel, int ms_n) {
    self->list = ItemList_new_ms(items, items_n, font, midSel, ms_n);
    ItemList_setFont(self->list, font);
    GUIScreen_setFont(&self->base, font);
    GUIScreen_setSoftKeysNames(&self->base, leftSoft, rightSoft);
}

void Selectable_destroy(Selectable *self) {
    GUIScreen_destroy(&self->base);
    if (self->list) { ItemList_free(self->list); self->list = NULL; }
}

void Selectable_paint(MyCanvas *mc, Graphics *g) {
    Selectable *self = (Selectable*) mc;
    Font *f = Main_getFont();
    ItemList_draw(self->list, g, 0, Font_height(f),
                  MyCanvas_getWidth(mc), MyCanvas_getHeight(mc) - Font_height(f) * 2);
    GUIScreen_drawSoftKeys(&self->base, g);
}

/* Recursao explicita para pular midSel==1 (fiel ao Java). */
void Selectable_onKey2(GUIScreen *gs) {
    Selectable *self = (Selectable*) gs;
    ItemList_scrollUp(self->list);
    if (self->list->midSel != NULL && self->list->midSel[self->list->index]) {
        if (self->list->index != 0) { Selectable_onKey2(gs); return; }
        if (self->list->index == 0) { Selectable_onKey2(gs); return; }
    }
    MyCanvas_repaint((MyCanvas*) gs);
}

void Selectable_onKey8(GUIScreen *gs) {
    Selectable *self = (Selectable*) gs;
    ItemList_scrollDown(self->list);
    if (self->list->midSel != NULL && self->list->midSel[self->list->index]) {
        if (self->list->index != self->list->items_n - 1) { Selectable_onKey8(gs); return; }
        if (self->list->index == self->list->items_n - 1) { Selectable_onKey8(gs); return; }
    }
    MyCanvas_repaint((MyCanvas*) gs);
}

int   Selectable_itemIndex(const Selectable *s) { return ItemList_getIndex(s->list); }
void  Selectable_setItemIndex(Selectable *s, int i) { ItemList_setIndex(s->list, i); }
char **Selectable_getItems(const Selectable *s, int *out_n) { return ItemList_getItems(s->list, out_n); }
