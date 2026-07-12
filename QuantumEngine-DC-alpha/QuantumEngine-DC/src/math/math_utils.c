/*
 * math_utils.c — porte fiel de code/Math/MathUtils.java (Quantum Engine J2ME)
 */
#include "math_utils.h"

#include <math.h>
#include <limits.h>

int MathUtils_randomCall = 0;

/* DirectX7.useAutoWMove externo (Rendering) — fiel ao Java. */
extern int DirectX7_useAutoWMove;

static inline int qe_abs(int a) { return a < 0 ? -a : a; }

int MathUtils_calcLightF(int nx, int ny, int nz, int sx, int sy, int sz, int f) {
    while (qe_abs(sx) > 30000 || qe_abs(sy) > 30000 || qe_abs(sz) > 30000) { sx/=2; sy/=2; sz/=2; }
    while (qe_abs(nx) > 30000 || qe_abs(ny) > 30000 || qe_abs(nz) > 30000) { nx/=2; ny/=2; nz/=2; }

    double nw = sqrt((double)(nx*nx + ny*ny + nz*nz));
    double lw = sqrt((double)(sx*sx + sy*sy + sz*sz));
    if (nw < 1) nw = 1;
    if (lw < 1) lw = 1;

    double l = ((nx*sx) + (ny*sy) + (nz*sz)) / (nw*lw);
    int v = (int)(l * 256 + f);
    if (v < 0) v = 0;
    v = v * 255 / (255 + f);
    if (v > 255) v = 255;
    return v;
}

int MathUtils_calcLight(int nx, int ny, int nz, int sx, int sy, int sz) {
    while (qe_abs(sx) > 30000 || qe_abs(sy) > 30000 || qe_abs(sz) > 30000) { sx/=2; sy/=2; sz/=2; }
    while (qe_abs(nx) > 30000 || qe_abs(ny) > 30000 || qe_abs(nz) > 30000) { nx/=2; ny/=2; nz/=2; }

    double nw = sqrt((double)(nx*nx + ny*ny + nz*nz));
    double lw = sqrt((double)(sx*sx + sy*sy + sz*sz));
    if (nw < 1) nw = 1;
    if (lw < 1) lw = 1;

    double l = ((nx*sx) + (ny*sy) + (nz*sz)) / (nw*lw);
    int v = (int)(l * 256);
    if (v < 0) v = 0;
    if (v > 255) v = 255;
    return v;
}

int MathUtils_microCalcLight(int nx, int ny, int nz, int sx, int sy, int sz) {
    while (sx > 30000 || sy > 30000 || sz > 30000 ||
           sx < -30000 || sy < -30000 || sz < -30000) {
        sx >>= 1; sy >>= 1; sz >>= 1;
    }
    double lw = sqrt((double)(sx*sx + sy*sy + sz*sz));
    if (lw < 1) lw = 1;
    int vec = (int)(((nx*sx) + (ny*sy) + (nz*sz)) / (lw * 4096) * 255);
    if (vec > 255) return 255;
    return vec;
}

int MathUtils_getAnglez(int x, int y, int x1, int y1) {
    int anglez = 0;
    int dist = qe_abs(x - x1) + qe_abs(y - y1);
    if (dist == 0) return 0;
    int xx = (x - x1) * 4000 / dist;
    int yy = (y - y1) * 4000 / dist;

    if (xx < 4000 && xx > 0 && yy > 0) xx = 0;
    if (xx > 0) anglez += xx * 360;
    if (xx < 0) anglez -= xx * 180;
    if (yy > 0) anglez += yy * 90;
    if (yy < 0) anglez -= yy * 270;

    anglez = anglez / 4000;
    anglez -= 90;
    return MathUtils_fixDegree(anglez);
}

int MathUtils_getAnglezHQ(int x, int y, int x1, int y1) {
    int anglez = 0;
    int dist = qe_abs(x - x1) + qe_abs(y - y1);
    if (dist == 0) return 0;
    int xx = (x - x1) * 4000 / dist;
    int yy = (y - y1) * 4000 / dist;

    if (xx < 4000 && xx > 0 && yy > 0) xx = 0;
    if (xx > 0) anglez += xx * 2048;
    if (xx < 0) anglez -= xx * 1024;
    if (yy > 0) anglez += yy * 512;
    if (yy < 0) anglez -= yy * 1536;

    anglez = anglez / 4000;
    anglez -= 512;
    return MathUtils_fixDegree2(anglez);
}

int MathUtils_fixDegree(int degree) {
    while (degree < 0)    degree += 360;
    while (degree >= 360) degree -= 360;
    return degree;
}
int MathUtils_fixDegree2(int degree) {
    while (degree < 0)     degree += 2048;
    while (degree >= 2048) degree -= 2048;
    return degree;
}

int MathUtils_mix(int a, int b, int i, int max) {
    if (i == 0)   return a;
    if (i == max) return b;
    return (a * (max - i) + b * i) / max;
}

int MathUtils_pLength3(int x, int y, int z) {
    if (!DirectX7_useAutoWMove) return 0;
    return (int) sqrt((double)(x*x + y*y + z*z));
}
int MathUtils_pLength2(int x, int y) {
    if (!DirectX7_useAutoWMove) return 0;
    return (int) sqrt((double)(x*x + y*y + (x + y)*(x + y) / 4));
}

int MathUtils_angleDistance(int a, int b) {
    int min = INT_MAX;
    for (int i = -360; i <= 360; i += 360) {
        int d = qe_abs(a + i - b);
        if (d < min) min = d;
    }
    return min;
}

int MathUtils_preudoRandom(long time, int val) {
    if (time < 0) time *= -1;
    time += MathUtils_randomCall * 255255L;
    time %= val;
    MathUtils_randomCall++;
    return (int) time;
}

int MathUtils_round(double d) {
    int dd = (int) d;
    double last = fabs(d) - fabs((double)dd);
    int ad = d > 0 ? 1 : -1;
    return dd + (last >= 0.5 ? ad : 0);
}
int MathUtils_ceil(double d) {
    int dd = (int) d;
    int ad = d > 0 ? 1 : -1;
    return dd + (floor(d) != d ? ad : 0);
}
