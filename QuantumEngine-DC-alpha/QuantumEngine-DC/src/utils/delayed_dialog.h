/*
 * delayed_dialog.h — porte fiel de code/utils/DelayedDialog.java (Quantum Engine J2ME)
 * @author Roman Lahin
 */
#ifndef QE_UTILS_DELAYED_DIALOG_H
#define QE_UTILS_DELAYED_DIALOG_H

#include <stdint.h>

typedef struct GameScreen GameScreen;

typedef struct DelayedDialog {
    int64_t  messageTimeOut; /* -1L default */
    int      messageType;    /* 0 = dialog, 1 = timed msg, 2 = paused msg */
    char    *message;        /* heap; livre com DelayedDialog_free */
    int64_t  timeToShow;
    int64_t  lastCheck;      /* -1L default */
    int64_t  time;
} DelayedDialog;

DelayedDialog *DelayedDialog_new(int64_t mto, int mt, const char *msg, int64_t tts);
void           DelayedDialog_free(DelayedDialog *self);

/* Retorna 1 quando disparou (activateDialog chamada). */
int  DelayedDialog_update(DelayedDialog *self, GameScreen *gs);

#endif
