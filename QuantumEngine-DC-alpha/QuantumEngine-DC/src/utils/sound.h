/*
 * sound.h — porte fiel de code/utils/Sound.java (Quantum Engine J2ME)
 *
 * O Java usava javax.microedition.media.Player. No Dreamcast usamos o AICA
 * via KOS (snd/sound.h + snd_sfx) para efeitos, e o mesmo Sound eh usado
 * como "music player" (loopCount=-1) — o gerente de musica externo decide
 * o backend real (OGG streaming) ao chamar loadFile.
 */
#ifndef QE_UTILS_SOUND_H
#define QE_UTILS_SOUND_H

#include <stdint.h>

typedef struct Sound Sound;

/* Java: Player states — devolvemos codigos equivalentes. */
#define QE_SOUND_STATE_UNREALIZED 100  /* Player.UNREALIZED */
#define QE_SOUND_STATE_REALIZED   200
#define QE_SOUND_STATE_PREFETCHED 300
#define QE_SOUND_STATE_STARTED    400
#define QE_SOUND_STATE_CLOSED     0

/* Construtores */
Sound *Sound_new(void);
Sound *Sound_new_file(const char *file);
Sound *Sound_createMusicPlayer(void); /* loopCount = -1 */

void   Sound_destroy(Sound *self);

void   Sound_loadFile(Sound *self, const char *file);

int    Sound_getLoopCount(Sound *self, int _unused_countArg); /* Java bizarro: getLoopCount(int) devolve o campo */
void   Sound_setLoopCount(Sound *self, int count);

void   Sound_start(Sound *self);
void   Sound_startAt(Sound *self, int64_t time_ms);
void   Sound_stop(Sound *self);

int64_t Sound_getTime(Sound *self);
void    Sound_setTime(Sound *self, int64_t time_ms);

void   Sound_setVolume(Sound *self, int i);
int    Sound_hasPlayer(const Sound *self);
int    Sound_getState(const Sound *self);

/* file passado pelo chamador (heap-alocado no ctor). */
const char *Sound_file(const Sound *self);

/* Escolhe mime-type Java pelo sufixo do arquivo (mantido para paridade). */
const char *Sound_formatCheck(const char *file_lc);

#endif
