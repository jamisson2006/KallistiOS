/*
 * fps.h — porte fiel de code/utils/FPS.java (Quantum Engine J2ME)
 * @author Roman Lahin
 */
#ifndef QE_UTILS_FPS_H
#define QE_UTILS_FPS_H

#include <stdint.h>

/* Todos "static" no Java = variaveis globais em C */
extern int   FPS_frameTime;      /* FPS sync and stuff */
extern int64_t FPS_previousFrame;
extern int64_t FPS_lastFpsTest;  /* Last fps test (fps should update every 1000ms) */
extern int   FPS_frames;         /* Frames in second counter */
extern int   FPS_currentTime;    /* running total time counter (Java: int) */
extern int   FPS_fps;            /* Frames per second */

void FPS_reset(void);
void FPS_miniReset(void);
void FPS_frame(void);

#endif
