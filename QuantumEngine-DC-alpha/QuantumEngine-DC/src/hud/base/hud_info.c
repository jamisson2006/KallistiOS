#include "hud_info.h"
#include <stdlib.h>

HUDInfo *HUDInfo_new(int money, int *ammo, int ammo_n) {
    HUDInfo *h = (HUDInfo*) calloc(1, sizeof(HUDInfo));
    h->money  = money;
    h->ammo   = ammo;
    h->ammo_n = ammo_n;
    return h;
}
void HUDInfo_free(HUDInfo *h) { free(h); }
