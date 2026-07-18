/*
 * video_player.c — porte de code/HUD/VideoPlayer.java (skeleton para DC)
 */
#include "video_player.h"
#include "../utils/main.h"

#include <stdlib.h>
#include <string.h>

VideoPlayer *VideoPlayer_new(Main *main, const char *file, MyCanvas *original) {
    VideoPlayer *v = (VideoPlayer*) calloc(1, sizeof(VideoPlayer));
    v->main     = main;
    v->file     = file ? strdup(file) : NULL;
    v->original = original;
    /* Sem backend de video no port DC: retorna imediatamente ao original. */
    Main_setCurrent(original);
    return v;
}

void VideoPlayer_free(VideoPlayer *v) {
    if (!v) return;
    free(v->file);
    free(v);
}
