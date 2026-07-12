/*
 * hud_info.h — porte fiel de code/HUD/Base/HUDInfo.java
 */
#ifndef QE_HUD_BASE_HUD_INFO_H
#define QE_HUD_BASE_HUD_INFO_H

typedef struct HUDInfo {
    int   money;
    int  *ammo;      /* Java: int[] final */
    int   ammo_n;    /* comprimento do array */
} HUDInfo;

HUDInfo *HUDInfo_new(int money, int *ammo, int ammo_n);
void     HUDInfo_free(HUDInfo *self);

#endif
