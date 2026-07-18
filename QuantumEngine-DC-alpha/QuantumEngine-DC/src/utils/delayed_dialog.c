/*
 * delayed_dialog.c — porte fiel de code/utils/DelayedDialog.java (Quantum Engine J2ME)
 */
#include "delayed_dialog.h"

#include <stdlib.h>
#include <string.h>

#ifdef _arch_dreamcast
#include <arch/timer.h>
static inline int64_t qe_now_ms(void) { return (int64_t) timer_ms_gettime64(); }
#else
#include <sys/time.h>
static inline int64_t qe_now_ms(void) {
    struct timeval tv; gettimeofday(&tv,0);
    return (int64_t)tv.tv_sec*1000LL + tv.tv_usec/1000;
}
#endif

/* --- assinaturas externas de modulos ainda por converter --- */
/* GameScreen expõe: customMessage, customMessagePause, customMessageEndTime,
 * lines (vetor), font, width, showDialog(). */
extern void GameScreen_setCustomMessage(GameScreen *gs, const char *msg);
extern void GameScreen_setCustomMessagePause(GameScreen *gs, int paused);
extern void GameScreen_setCustomMessageEndTime(GameScreen *gs, int64_t t);
extern void GameScreen_clearLines(GameScreen *gs);
extern void GameScreen_showDialog(GameScreen *gs, const char *msg);
extern void TextView_createLines(const char *msg, void *lines_vec, void *font, int width);
extern void *GameScreen_lines(GameScreen *gs);
extern void *GameScreen_font(GameScreen *gs);
extern int   GameScreen_width(GameScreen *gs);

DelayedDialog *DelayedDialog_new(int64_t mto, int mt, const char *msg, int64_t tts) {
    DelayedDialog *d = (DelayedDialog*) calloc(1, sizeof(DelayedDialog));
    d->messageTimeOut = mto;
    d->messageType    = mt;
    d->message        = msg ? strdup(msg) : NULL;
    d->timeToShow     = tts;
    d->lastCheck      = -1LL;
    d->time           = 0LL;
    return d;
}

void DelayedDialog_free(DelayedDialog *self) {
    if (!self) return;
    free(self->message);
    free(self);
}

static void qe_activate(DelayedDialog *self, GameScreen *gs) {
    if (self->messageType > 0) {
        GameScreen_setCustomMessage(gs, self->message);
        GameScreen_setCustomMessagePause(gs, 0);
        GameScreen_setCustomMessageEndTime(gs, qe_now_ms() + self->messageTimeOut);
        if (self->messageType == 2) {
            GameScreen_setCustomMessagePause(gs, 1);
            GameScreen_setCustomMessageEndTime(gs, self->messageTimeOut);
        }
        GameScreen_clearLines(gs);
        TextView_createLines(self->message, GameScreen_lines(gs),
                             GameScreen_font(gs), GameScreen_width(gs));
    }
    if (self->messageType == 0) {
        GameScreen_showDialog(gs, self->message);
    }
}

int DelayedDialog_update(DelayedDialog *self, GameScreen *gs) {
    int64_t now = qe_now_ms();
    if (self->lastCheck == -1LL) self->lastCheck = now;
    self->time += (now - self->lastCheck);

    if (self->time >= self->timeToShow) {
        qe_activate(self, gs);
        return 1;
    }

    self->lastCheck = qe_now_ms();
    return 0;
}
