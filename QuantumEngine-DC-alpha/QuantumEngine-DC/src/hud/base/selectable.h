/*
 * selectable.h — porte fiel de code/HUD/Base/Selectable.java
 *
 * Extensão de GUIScreen que embute um ItemList com navegacao 2/8 e wrap.
 */
#ifndef QE_HUD_BASE_SELECTABLE_H
#define QE_HUD_BASE_SELECTABLE_H

#include "../gui_screen.h"
#include "item_list.h"

typedef struct Selectable {
    GUIScreen  base;
    ItemList  *list;
} Selectable;

/* Java: set(font, list_vector, leftSoft, rightSoft) — recebe items[] pre-copiado. */
void Selectable_set    (Selectable *self, Font *font, char **items, int items_n,
                        const char *leftSoft, const char *rightSoft);
void Selectable_set_ms (Selectable *self, Font *font, char **items, int items_n,
                        const char *leftSoft, const char *rightSoft, int *midSel, int ms_n);

void Selectable_destroy(Selectable *self);
void Selectable_paint  (MyCanvas *mc, Graphics *g);
void Selectable_onKey2 (GUIScreen *gs);
void Selectable_onKey8 (GUIScreen *gs);

int  Selectable_itemIndex (const Selectable *self);
void Selectable_setItemIndex(Selectable *self, int i);
char **Selectable_getItems(const Selectable *self, int *out_n);

#endif
