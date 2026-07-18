/*
 * video_player.h — porte de code/HUD/VideoPlayer.java
 *
 * Reproducao de video MP4/3GP via javax.microedition.media.Player no Java.
 * No Dreamcast nao ha decoder de video nativo em KOS; o modulo eh portado
 * como skeleton — ao chamar play, apenas retorna imediatamente `original`
 * para a tela anterior (comportamento seguro). Backend real (via
 * libmpeg2/ffmpeg no port DC) fica para fase seguinte.
 */
#ifndef QE_HUD_VIDEO_PLAYER_H
#define QE_HUD_VIDEO_PLAYER_H

typedef struct MyCanvas MyCanvas;
typedef struct Main     Main;

typedef struct VideoPlayer {
    char     *file;
    Main     *main;
    MyCanvas *original;
} VideoPlayer;

VideoPlayer *VideoPlayer_new(Main *main, const char *file, MyCanvas *original);
void         VideoPlayer_free(VideoPlayer *self);

#endif
