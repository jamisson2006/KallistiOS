/*
 * qfps.c — porte fiel de code/utils/QFPS.java (Quantum Engine J2ME)
 * @author Roman
 */
#include "qfps.h"

void QFPS_frame(void) {
    FPS_frame();
    if (FPS_frameTime < 1)         FPS_frameTime = 1;
    else if (FPS_frameTime > 200)  FPS_frameTime = 200;
}
