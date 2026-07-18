/*
 * fps.c — porte fiel de code/utils/FPS.java (Quantum Engine J2ME)
 * @author Roman Lahin
 */
#include "fps.h"

#ifdef _arch_dreamcast
#include <arch/timer.h>
static inline int64_t qe_current_ms(void) {
    /* timer_ms_gettime64 retorna ms desde o boot do KOS */
    return (int64_t) timer_ms_gettime64();
}
#else
#include <sys/time.h>
static inline int64_t qe_current_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return (int64_t)tv.tv_sec * 1000LL + (int64_t)(tv.tv_usec / 1000);
}
#endif

int     FPS_frameTime      = 0;
int64_t FPS_previousFrame  = 0;
int64_t FPS_lastFpsTest    = 0;
int     FPS_frames         = 0;
int     FPS_currentTime    = 0;
int     FPS_fps            = 0;

void FPS_reset(void) {
    FPS_lastFpsTest = qe_current_ms();
    FPS_currentTime = FPS_frames = FPS_fps = 0;
    FPS_lastFpsTest = FPS_previousFrame = 0;
    FPS_frameTime = 15;
}

void FPS_miniReset(void) {
    FPS_lastFpsTest = FPS_previousFrame = 0;
    FPS_frames = 30; /* maybe? */
}

void FPS_frame(void) {
    if (FPS_previousFrame != 0)
        FPS_frameTime = (int)(qe_current_ms() - FPS_previousFrame);
    FPS_previousFrame = qe_current_ms();

    FPS_frames++;
    FPS_currentTime += FPS_frameTime;

    if (FPS_previousFrame - FPS_lastFpsTest >= 1000) { /* Fps meter */
        FPS_fps = FPS_frames;
        FPS_frames = 0;
        FPS_lastFpsTest = FPS_previousFrame;
    }
}
