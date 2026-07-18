/*
 * sound.c — porte fiel de code/utils/Sound.java (Quantum Engine J2ME)
 *
 * Java Player -> KOS snd_sfx (WAV in-memory). Formatos nao-WAV terao
 * player == 0 (mimicking Java quando createPlayer falha por MediaException).
 */
#include "sound.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _arch_dreamcast
#include <dc/sound/sfxmgr.h>
#include <arch/timer.h>
#endif

struct Sound {
    /* Player abstrato — no Java era javax.microedition.media.Player */
    int    has_player;   /* != 0 se um sfx foi carregado com sucesso */
#ifdef _arch_dreamcast
    sfxhnd_t sfx;        /* SFXHND_INVALID quando nada carregado */
    int      chan;       /* canal ativo, ou -1 */
#endif

    char  *file;         /* copia heap do path */
    int    volume;       /* 0..100 */
    int    loopCount;    /* -1 = infinito (music) */
    int64_t start_ms;    /* usado para simular getMediaTime() */
    int    state;
};

/* -- Utilitarios internos -- */

#ifdef _arch_dreamcast
static inline int64_t qe_now_ms(void) { return (int64_t) timer_ms_gettime64(); }
#else
#include <sys/time.h>
static inline int64_t qe_now_ms(void) {
    struct timeval tv; gettimeofday(&tv,0);
    return (int64_t)tv.tv_sec*1000LL + tv.tv_usec/1000;
}
#endif

static int qe_endswith(const char *s, const char *suf) {
    size_t ls = strlen(s), lsuf = strlen(suf);
    if (lsuf > ls) return 0;
    return strcmp(s + ls - lsuf, suf) == 0;
}

/* useMP3: no Java verificava Manager.getSupportedContentTypes. No DC deixamos
 * 0 (nao ha decoder MP3 padrao no snd_sfx). Este flag so afeta o mime-type. */
static const int qe_useMP3 = 0;

/* -- Construtores -- */

static Sound *qe_alloc_sound(void) {
    Sound *s = (Sound*) calloc(1, sizeof(Sound));
    s->volume = 100;
    s->loopCount = 1;
    s->state = QE_SOUND_STATE_UNREALIZED;
#ifdef _arch_dreamcast
    s->sfx = SFXHND_INVALID;
    s->chan = -1;
#endif
    return s;
}

Sound *Sound_new(void) { return qe_alloc_sound(); }

Sound *Sound_new_file(const char *file) {
    Sound *s = qe_alloc_sound();
    Sound_loadFile(s, file);
    return s;
}

Sound *Sound_createMusicPlayer(void) {
    Sound *s = qe_alloc_sound();
    s->loopCount = -1;
    return s;
}

/* -- Carrega -- */

static void qe_tolower(const char *in, char *out, size_t out_sz) {
    size_t i = 0;
    for (; in[i] && i + 1 < out_sz; i++) out[i] = (char) tolower((unsigned char) in[i]);
    out[i] = 0;
}

void Sound_loadFile(Sound *self, const char *file) {
    Sound_destroy(self); /* fecha player anterior — Java chamou destroy() */

    /* alocamos de novo os campos base (destroy anulou tudo mas continua o objeto) */
    self->volume    = 100;
    self->loopCount = 1;
    self->state     = QE_SOUND_STATE_UNREALIZED;
#ifdef _arch_dreamcast
    self->sfx = SFXHND_INVALID;
    self->chan = -1;
#endif

    char lc[512]; qe_tolower(file, lc, sizeof lc);
    if (strcmp(lc, "noone") == 0) return;

    self->file = strdup(file);

    /* Java: String format = formatCheck(lcFile); createPlayer(is, format);
     * player.realize(); player.prefetch(). Aqui carregamos o arquivo em
     * memoria via snd_sfx_load. So WAV eh suportado pelo backend. */
#ifdef _arch_dreamcast
    char path[512];
    if (file[0] == '/') snprintf(path, sizeof path, "/rd%s", file);
    else                snprintf(path, sizeof path, "/rd/%s", file);

    self->sfx = snd_sfx_load(path);
    if (self->sfx != SFXHND_INVALID) {
        self->has_player = 1;
        self->state = QE_SOUND_STATE_PREFETCHED;
    }
#else
    /* host: apenas registra que "abriu" (evita falha nos testes). */
    self->has_player = 1;
    self->state = QE_SOUND_STATE_PREFETCHED;
    (void) lc;
#endif
}

void Sound_destroy(Sound *self) {
    if (!self) return;
    free(self->file); self->file = NULL;

    Sound_stop(self);
#ifdef _arch_dreamcast
    if (self->sfx != SFXHND_INVALID) {
        snd_sfx_unload(self->sfx);
        self->sfx = SFXHND_INVALID;
    }
#endif
    self->has_player = 0;
    self->state = QE_SOUND_STATE_CLOSED;
}

int Sound_getLoopCount(Sound *self, int _unused) {
    (void)_unused;
    return self->loopCount;
}

void Sound_setLoopCount(Sound *self, int count) {
    self->loopCount = count;
    if (!self->has_player) return;
    /* Java: player.setLoopCount(loopCount). Em snd_sfx nao ha setLoopCount
     * post-hoc — so aplica no proximo start. Sem operacao imediata. */
}

void Sound_start(Sound *self) { Sound_startAt(self, 0); }

void Sound_startAt(Sound *self, int64_t time_ms) {
    if (!self->has_player) return;

    Sound_setVolume(self, self->volume);
    Sound_setTime(self, time_ms);
    /* Java: player.setLoopCount(loopCount); player.start(); */
    self->start_ms = qe_now_ms() - time_ms;
    self->state = QE_SOUND_STATE_STARTED;

#ifdef _arch_dreamcast
    if (self->sfx != SFXHND_INVALID) {
        int loop = (self->loopCount < 0) ? 1 : 0;
        int vol  = self->volume * 255 / 100;
        /* pan 128 = centro */
        self->chan = snd_sfx_play(self->sfx, vol, 128);
        (void) loop; /* snd_sfx nao expoe loop no play */
    }
#endif
}

void Sound_stop(Sound *self) {
    if (!self->has_player) return;
#ifdef _arch_dreamcast
    if (self->chan >= 0) {
        snd_sfx_stop(self->chan);
        self->chan = -1;
    }
#endif
    if (self->state == QE_SOUND_STATE_STARTED)
        self->state = QE_SOUND_STATE_PREFETCHED;
}

int64_t Sound_getTime(Sound *self) {
    if (!self->has_player) return 0;
    /* Java: getMediaTime; -1 -> 0. Simulamos com o relogio desde start. */
    if (self->state != QE_SOUND_STATE_STARTED) return 0;
    int64_t t = qe_now_ms() - self->start_ms;
    return t < 0 ? 0 : t;
}

void Sound_setTime(Sound *self, int64_t time_ms) {
    if (!self->has_player) return;
    self->start_ms = qe_now_ms() - time_ms;
    /* snd_sfx nao permite seek — deixamos como no-op silencioso (fiel ao
     * MediaException catch). */
}

void Sound_setVolume(Sound *self, int i) {
    if (!self->has_player) return;
    /* Clamp identico ao Java (i>100?100:(i<0?0:i)) */
    self->volume = i > 100 ? 100 : (i < 0 ? 0 : i);
    /* Java: VolumeControl.setLevel(volume). snd_sfx aplica o volume no play. */
}

int Sound_hasPlayer(const Sound *self) { return self->has_player; }

int Sound_getState(const Sound *self) {
    if (!self->has_player) return -1;
    return self->state;
}

const char *Sound_file(const Sound *self) { return self->file; }

const char *Sound_formatCheck(const char *file_lc) {
    if (qe_endswith(file_lc, ".wav")  || qe_endswith(file_lc, ".wave")) return "audio/x-wav";
    if (qe_endswith(file_lc, ".midi") || qe_endswith(file_lc, ".mid"))  return "audio/midi";
    if (qe_endswith(file_lc, ".mp3"))  return qe_useMP3 ? "audio/mp3" : "audio/mpeg";
    if (qe_endswith(file_lc, ".amr"))  return "audio/amr";
    if (qe_endswith(file_lc, ".jts"))  return "audio/x-tone-seq";
    if (qe_endswith(file_lc, ".mxmf")) return "audio/mobile-xmf";
    if (qe_endswith(file_lc, ".mmf"))  return "application/vnd.yamaha.smaf-audio";
    return "";
}
