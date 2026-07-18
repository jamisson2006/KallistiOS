/*
 * weather_generator.h — porte fiel de code/utils/WeatherGenerator.java (Quantum Engine J2ME)
 * @author Romash
 */
#ifndef QE_UTILS_WEATHER_GENERATOR_H
#define QE_UTILS_WEATHER_GENERATOR_H

#include <stdint.h>

/* g3d = DirectX7 (a converter) — mantemos ponteiro opaco. */
typedef struct DirectX7 DirectX7;
/* Handle Graphics do J2ME — no port DC, wrapper de framebuffer. */
typedef struct Graphics  Graphics;

typedef struct WeatherGenerator {
    int8_t generate;   /* 0=nada, 1=chuva, 2=neve */
    float *coords;     /* pares (x,y) */
    int   *info;       /* pares (type,depth) */
    int    coords_len; /* length total do array */
    int    info_len;

    int screenWidth, screenHeight;
    int rainColorNear, rainColorFar;
    int snowColorNear, snowColorFar;
    int lighting;                 /* boolean */
    int lightingTwice;            /* boolean */
    int64_t lastLighting, nextLighting;
    int *lightingTimes;           /* array de 2 ints [min,max]; ownership do chamador */
    int lightingTimes_len;

    int snowSize, rainSize;
    int centerX, centerY;
    int snowSpeed, rainSpeed;
    int isZoom;                   /* boolean */
    int lastUnderRoof;            /* boolean */

    /* Constantes Java: final */
    /* horizonLine = -5; horizonSize = 90; */
} WeatherGenerator;

WeatherGenerator *WeatherGenerator_new(int scrwidth, int scrheight, int quality,
                                       int rcn, int rcf, int scn, int scf,
                                       int lighting, int *lightingTimes, int lt_len,
                                       int ss, int rs,
                                       int cenx, int ceny,
                                       int sspeed, int rspeed);

void WeatherGenerator_free(WeatherGenerator *self);

void WeatherGenerator_clearBuffers(WeatherGenerator *self);
void WeatherGenerator_createParticles(WeatherGenerator *self);
void WeatherGenerator_moveX(WeatherGenerator *self, int x, int frameTime);
void WeatherGenerator_moveX2(WeatherGenerator *self, int x, int frameTime);
void WeatherGenerator_moveY(WeatherGenerator *self, int y, int frameTime);
void WeatherGenerator_moveY2(WeatherGenerator *self, int y, int frameTime);
void WeatherGenerator_move(WeatherGenerator *self, int z, int frameTime, int diry);
void WeatherGenerator_moveB(WeatherGenerator *self, int z, int frameTime, int diry);
void WeatherGenerator_update(WeatherGenerator *self, int frameTime, int diry, int underRoof);
void WeatherGenerator_genNewLighting(WeatherGenerator *self);
void WeatherGenerator_paint(WeatherGenerator *self, Graphics *g, int xx);
int  WeatherGenerator_mixColor(int col1, int col2, int mix);
void WeatherGenerator_lightingEffect(WeatherGenerator *self, DirectX7 *g3d, float rotateX);

/* Constantes: */
#define QE_WEATHER_HORIZON_LINE (-5)
#define QE_WEATHER_HORIZON_SIZE (90)

#endif
