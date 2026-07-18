#ifndef QE_HUD_LEVEL_SELECTION_H
#define QE_HUD_LEVEL_SELECTION_H
#include "base/selectable.h"
typedef struct Main Main;
typedef struct Menu Menu;
typedef struct LevelSelection { Selectable base; Main *main; Menu *menu; } LevelSelection;

LevelSelection *LevelSelection_new(Main *main, Menu *menu);
LevelSelection *LevelSelection_new_hud(Main *main, Menu *menu, void *hudInfo);
void            LevelSelection_free(LevelSelection *self);

#endif
