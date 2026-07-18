/*
 * items_engine.c — porte fiel de code/Gameplay/Inventory/ItemsEngine.java
 */
#include "items_engine.h"
#include "../../utils/inifile.h"
#include "../../utils/string_tools.h"
#include <stdlib.h>
#include <string.h>

IniFile **ItemsEngine_items = NULL;
int       ItemsEngine_itemCount = 0;

void ItemsEngine_init(void) {
    char *data = StringTools_getStringFromResource("/items.txt");
    if (!data) return;

    char **sections = NULL;
    int count = 0;
    StringTools_cutOnStrings_sep(data, '[', &sections, &count);

    ItemsEngine_items = (IniFile **)calloc(count, sizeof(IniFile *));
    ItemsEngine_itemCount = count;

    for (int i = 0; i < count; i++) {
        ItemsEngine_items[i] = IniFile_new(sections[i], 0);
    }

    if (sections) {
        for (int i = 0; i < count; i++) free(sections[i]);
        free(sections);
    }
    free(data);
}

int ItemsEngine_getItemId(const char *name) {
    if (!name) return -1;
    for (int i = 0; i < ItemsEngine_itemCount; i++) {
        const char *n = IniFile_get(ItemsEngine_items[i], "NAME");
        if (n && strcmp(n, name) == 0) return i;
    }
    return -1;
}
