/*
 * weather_generator.c — porte fiel de code/utils/WeatherGenerator.java (Quantum Engine J2ME)
 * @author Romash
 */
#include "weather_generator.h"

#include <stdio.h>
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

/* --- Extern (a converter) --- */
extern void  Graphics_setColor(Graphics *g, int col);
extern void  Graphics_fillRect(Graphics *g, int x, int y, int w, int h);
extern void  Graphics_drawLine(Graphics *g, int x1, int y1, int x2, int y2);

extern int   DirectX7_fovY(const DirectX7 *g);
extern int   DirectX7_centreY(const DirectX7 *g);
extern int   DirectX7_distY(const DirectX7 *g);
extern int   DirectX7_height(const DirectX7 *g);
extern int  *DirectX7_getDisplay(const DirectX7 *g);

/* --- Random compatibility layer --- */
static inline int qe_next_int(int bound) {
    /* Java Random.nextInt(bound). rand() nao eh perfeito mas basta. */
    if (bound <= 0) return 0;
    return rand() % bound;
}

WeatherGenerator *WeatherGenerator_new(int scrwidth, int scrheight, int quality,
                                       int rcn, int rcf, int scn, int scf,
                                       int lighting, int *lightingTimes, int lt_len,
                                       int ss, int rs,
                                       int cenx, int ceny,
                                       int sspeed, int rspeed) {
    WeatherGenerator *w = (WeatherGenerator*) calloc(1, sizeof(WeatherGenerator));

    w->screenWidth  = scrwidth;
    w->screenHeight = scrheight;

    /* Java: new float[quality*scrwidth/240*scrheight/320*2]; */
    int n = quality * scrwidth / 240 * scrheight / 320 * 2;
    if (n < 2) n = 2;
    w->coords     = (float*) calloc(n, sizeof(float));
    w->info       = (int*)   calloc(n, sizeof(int));
    w->coords_len = n;
    w->info_len   = n;

    WeatherGenerator_clearBuffers(w);

    w->lighting     = lighting;
    w->lastLighting = qe_now_ms();
    w->lightingTimes     = lightingTimes;
    w->lightingTimes_len = lt_len;
    if (lightingTimes != NULL) WeatherGenerator_genNewLighting(w);

    w->rainColorNear = rcn;
    w->rainColorFar  = rcf;
    w->snowColorNear = scn;
    w->snowColorFar  = scf;

    w->snowSize = ss * scrheight / 320;
    if (w->snowSize < ss) w->snowSize = ss;
    w->rainSize = rs * scrheight / 320;
    if (w->rainSize < rs) w->rainSize = rs;

    w->centerX = cenx;
    w->centerY = ceny;
    w->snowSpeed = sspeed * scrheight / 320;
    w->rainSpeed = rspeed * scrheight / 320;
    w->generate = 0;
    return w;
}

void WeatherGenerator_free(WeatherGenerator *self) {
    if (!self) return;
    free(self->coords);
    free(self->info);
    free(self);
}

void WeatherGenerator_clearBuffers(WeatherGenerator *self) {
    for (int i = 0; i < self->coords_len; i++) self->coords[i] = 0.0f;
    for (int i = 0; i < self->info_len; i++)   self->info[i]   = 0;
}

void WeatherGenerator_createParticles(WeatherGenerator *self) {
    for (int i = 0; i < self->coords_len / 2; i++) {
        if (self->info[i * 2] == 0) {
            self->coords[i * 2 + 1] = (float) qe_next_int(self->screenHeight);
            self->coords[i * 2]     = (float) qe_next_int(self->screenWidth);
            self->info[i * 2]     = self->generate;
            self->info[i * 2 + 1] = qe_next_int(100);
        }
    }
}

void WeatherGenerator_moveX(WeatherGenerator *self, int x, int frameTime) {
    float xx = (float) x * 3.0f * frameTime / 50.0f;
    for (int i = 0; i < self->coords_len / 2; i++) {
        int depth = self->info[i * 2 + 1] / 22;
        if (depth <= 0) depth = 1;
        if (depth > 65) depth = 65;
        if (self->info[i * 2] != 0) self->coords[i * 2] += xx / depth;
    }
}

void WeatherGenerator_moveX2(WeatherGenerator *self, int x, int frameTime) {
    float xx = (float) x * 4.0f * frameTime / 50.0f;
    for (int i = 0; i < self->coords_len / 2; i++) {
        if (self->info[i * 2] != 0) self->coords[i * 2] += xx;
    }
}

void WeatherGenerator_moveY(WeatherGenerator *self, int y, int frameTime) {
    float yy = (float) y * 1.3f * 4.0f * frameTime / 50.0f;
    for (int i = 0; i < self->coords_len / 2; i++) {
        if (self->info[i * 2] != 0) self->coords[i * 2 + 1] += yy;
    }
}

void WeatherGenerator_moveY2(WeatherGenerator *self, int y, int frameTime) {
    float yy = (float) y * 1.3f * 3.0f * frameTime / 80.0f / 50.0f;
    for (int i = 0; i < self->coords_len / 2; i++) {
        int depth = self->info[i * 2 + 1] / 25;
        if (depth <= 0) depth = 1;
        if (depth > 65) depth = 65;
        if (self->info[i * 2] != 0) self->coords[i * 2 + 1] += yy / depth;
    }
}

void WeatherGenerator_move(WeatherGenerator *self, int z, int frameTime, int diry) {
    float zz = (float) z * 2.75f * frameTime / 50.0f;
    self->isZoom = 1;
    float mz = 1.0f + 0.05f * z / 5.0f * frameTime / 50.0f;
    int centerY2 = self->centerY * (90 + diry * 7 / 5) / 90;

    for (int i = 0; i < self->coords_len / 2; i++) {
        if (self->info[i * 2] != 0) {
            float x = self->coords[i * 2]     = (self->coords[i * 2] - self->centerX) * mz + self->centerX;
            float y = self->coords[i * 2 + 1] = (self->coords[i * 2 + 1] - centerY2) * mz + centerY2;

            self->info[i * 2 + 1] -= (int)(zz / 3.5f);

            if (x > self->screenWidth || x < 0 || y > self->screenHeight || y < 0) {
                self->coords[i * 2]     = (float) qe_next_int(self->screenWidth);
                self->coords[i * 2 + 1] = (float) qe_next_int(self->screenHeight);
                self->info[i * 2 + 1] = 100;
            }
        } else /* info[i*2] == 0 */ {
            self->coords[i * 2]     = (float) qe_next_int(self->screenWidth);
            self->coords[i * 2 + 1] = (float) qe_next_int(self->screenHeight);
            self->info[i * 2 + 1] = 100;
            self->info[i * 2]     = self->generate;
        }
    }
}

void WeatherGenerator_moveB(WeatherGenerator *self, int z, int frameTime, int diry) {
    float zz = (float) z * 2.75f * frameTime / 50.0f;
    float mz = 1.0f - 0.05f * z / 5.0f * frameTime / 50.0f;
    int centerY2 = self->centerY * (90 + diry * 7 / 5) / 90;

    for (int i = 0; i < self->coords_len / 2; i++) {
        if (self->info[i * 2] != 0) {
            self->coords[i * 2]     = (self->coords[i * 2] - self->centerX) * mz + self->centerX;
            self->coords[i * 2 + 1] = (self->coords[i * 2 + 1] - centerY2) * mz + centerY2;

            self->info[i * 2 + 1] += (int)(zz / 4.0f);

            if (self->info[i * 2 + 1] > 100) {
                if (qe_next_int(2) == 0) {
                    self->coords[i * 2]     = (float)(qe_next_int(2) * self->screenWidth);
                    self->coords[i * 2 + 1] = (float) qe_next_int(self->screenHeight);
                } else {
                    self->coords[i * 2]     = (float) qe_next_int(self->screenWidth);
                    self->coords[i * 2 + 1] = (float)(qe_next_int(2) * self->screenHeight);
                }
                self->info[i * 2 + 1] = 0;
            }
        }
    }
}

void WeatherGenerator_update(WeatherGenerator *self, int frameTime, int diry, int underRoof) {
    for (int i = 0; i < self->coords_len / 2; i++) {
        if (self->coords[i * 2 + 1] < -2)                self->coords[i * 2 + 1] += self->screenHeight - 1;
        if (self->coords[i * 2]     > self->screenWidth) self->coords[i * 2] = self->coords[i * 2] - self->screenWidth;
        if (self->coords[i * 2]     < -2)                self->coords[i * 2] = self->coords[i * 2] + self->screenWidth + 2;

        if (self->isZoom == 0) {
            if (self->coords[i * 2 + 1] > self->screenHeight) self->info[i * 2] = 0;
            if (self->coords[i * 2 + 1] > (self->screenHeight * (diry + 90) / 180) + self->screenHeight / 5
                && self->info[i * 2 + 1] > 40) { self->info[i * 2] = 0; }

            if (self->coords[i * 2] > self->screenWidth + 10) self->info[i * 2] = 0;
            if (self->coords[i * 2] < -12)                    self->info[i * 2] = 0;

            if (self->info[i * 2] == 0 && !underRoof) {
                if (!self->lastUnderRoof) {
                    self->coords[i * 2 + 1] = 0;
                    self->coords[i * 2]     = (float) qe_next_int(self->screenWidth);
                    self->info[i * 2]     = self->generate;
                    self->info[i * 2 + 1] = qe_next_int(100);
                } else {
                    self->coords[i * 2 + 1] = (float) qe_next_int(self->screenHeight);
                    self->coords[i * 2]     = (float) qe_next_int(self->screenWidth);
                    self->info[i * 2]     = self->generate;
                    self->info[i * 2 + 1] = 100;
                }
            }
        }

        if (self->info[i * 2] == 2) { /* snow */
            int rand_ = qe_next_int(3) - 1;
            self->info[i * 2 + 1] += rand_ * self->snowSpeed * frameTime / 50 / 4;
            int depth = self->info[i * 2 + 1];
            if (depth <= 0)  depth = 1;
            if (depth > 100) depth = 100;
            depth = 100 - depth;
            if (self->info[i * 2 + 1] > 100) self->info[i * 2 + 1] = 100;
            if (self->info[i * 2 + 1] < 0)   self->info[i * 2 + 1] = 0;

            self->coords[i * 2]     += (float)(rand_ * 2 * self->snowSpeed
                                     / ((100 - depth) / 50 + 1) * frameTime / 50 / 4);
            self->coords[i * 2 + 1] += (float)((self->snowSpeed + qe_next_int(4) / 2)
                                     * (depth / 40 + 2) / 2 * frameTime / 50);
        } else if (self->info[i * 2] == 1) { /* rain */
            int depth = self->info[i * 2 + 1];
            if (depth <= 0)  depth = 1;
            if (depth > 100) depth = 100;
            depth = 100 - depth;
            if (self->info[i * 2 + 1] > 100) self->info[i * 2 + 1] = 100;
            if (self->info[i * 2 + 1] < 0)   self->info[i * 2 + 1] = 0;
            float speed = (float) self->rainSpeed * (depth / 40 + 2) / 2 * frameTime / 50.0f;
            self->coords[i * 2]     -= speed / 4.0f;
            self->coords[i * 2 + 1] += speed;
        }
    }

    if (self->lighting && qe_now_ms() >= self->nextLighting) {
        self->lastLighting = qe_now_ms();
        WeatherGenerator_genNewLighting(self);
    }

    self->isZoom = 0;
    self->lastUnderRoof = underRoof;
}

void WeatherGenerator_genNewLighting(WeatherGenerator *self) {
    if (!self->lightingTimes || self->lightingTimes_len < 2) return;

    int range = self->lightingTimes[1] - self->lightingTimes[0];
    if (range <= 0) range = 1;
    self->nextLighting = self->lastLighting + qe_next_int(range) + self->lightingTimes[0];

    int oldLightingTwice = self->lightingTwice;
    self->lightingTwice = (qe_next_int(self->lightingTwice ? 5 : 2) == 0);
    if (self->lightingTwice)
        self->nextLighting = self->lastLighting + 200 + qe_next_int(175) - (oldLightingTwice ? 75 : 0);
}

int WeatherGenerator_mixColor(int col1, int col2, int mix) {
    int cr = (((col1 >> 16) & 0xff) * (100 - mix) + ((col2 >> 16) & 0xff) * mix);
    int cg = (((col1 >> 8)  & 0xff) * (100 - mix) + ((col2 >> 8)  & 0xff) * mix);
    int cb = (( col1        & 0xff) * (100 - mix) + ( col2        & 0xff) * mix);
    cr /= 100; cg /= 100; cb /= 100;
    return (cr << 16) | (cg << 8) | cb;
}

void WeatherGenerator_paint(WeatherGenerator *self, Graphics *g, int xx) {
    int light = (self->lighting && (qe_now_ms() - self->lastLighting) < 135);

    for (int i = 0; i < self->coords_len / 2; i++) {
        if (self->info[i * 2] == 2) {
            int depth = self->info[i * 2 + 1];
            int size = (self->snowSize * (100 - depth) + depth) / 100;
            if (depth < 0)   size = self->snowSize * (100 - depth) / 100;
            if (depth < 0)   depth = 0;
            if (depth > 100) depth = 100;
            int col = WeatherGenerator_mixColor(self->snowColorNear, self->snowColorFar, depth);
            if (light) col = 0xffffff;
            Graphics_setColor(g, col);
            int x = (int) self->coords[i * 2];
            int y = (int) self->coords[i * 2 + 1];

            if (size > 1)  Graphics_fillRect(g, x - size / 2, y - size / 2 + xx, size - 1, size - 1);
            if (size <= 1) Graphics_drawLine(g, x, y + xx, x, y + xx);

        } else if (self->info[i * 2] == 1) {
            int depth = self->info[i * 2 + 1];
            int size = (self->rainSize * (100 - depth) + depth) / 100;
            if (depth < 0)   size = self->rainSize * (100 - depth) / 100;
            if (depth < 0)   depth = 0;
            if (depth > 100) depth = 100;
            int col = WeatherGenerator_mixColor(self->rainColorNear, self->rainColorFar, depth);
            if (light) col = 0xffffff;
            Graphics_setColor(g, col);
            int x = (int) self->coords[i * 2];
            int y = (int) self->coords[i * 2 + 1];

            Graphics_drawLine(g, x, y + xx, x - size / 4, y + xx + size);
        }
    }

    Graphics_setColor(g, 0xffffff);
}

void WeatherGenerator_lightingEffect(WeatherGenerator *self, DirectX7 *g3d, float rotateX) {
    int fovY   = DirectX7_fovY(g3d);
    int centreY= DirectX7_centreY(g3d);
    int distY  = DirectX7_distY(g3d);
    int height = DirectX7_height(g3d);
    (void) height;

    int horizonY = (int)(self->screenHeight / 2 - QE_WEATHER_HORIZON_LINE * self->screenHeight / fovY
                                                 + rotateX * self->screenHeight / fovY);
    horizonY = (horizonY - centreY) * distY / (11 + distY) + centreY;

    int ySize = QE_WEATHER_HORIZON_SIZE * DirectX7_height(g3d) / fovY;

    if (horizonY >= self->screenHeight) return;

    int y = horizonY;
    if (y < 0) y = 0;

    int *scr = DirectX7_getDisplay(g3d);

    int x1, x2, col, scrcol;
    for (; y < self->screenHeight; y++) {
        x1 = y * self->screenWidth;
        x2 = x1 + self->screenWidth;
        col = (y - horizonY) * 255 / ySize;
        if (col > 100) col = 100;
        col = (col & 0xfe) * 0x010101;

        while (x1 < x2) {
            scrcol = (scr[x1] & 0xFEFEFE) + col;
            scr[x1] = scrcol | (((scrcol >> 8) & 0x010101) * 0xFF);
            x1++;
        }
    }
}
