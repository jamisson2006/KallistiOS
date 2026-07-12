/*
 * math_utils.h — porte fiel de code/Math/MathUtils.java (Quantum Engine J2ME)
 */
#ifndef QE_MATH_MATH_UTILS_H
#define QE_MATH_MATH_UTILS_H

#include <stdint.h>

extern int MathUtils_randomCall;

int MathUtils_calcLightF(int nx, int ny, int nz, int sx, int sy, int sz, int f);
int MathUtils_calcLight (int nx, int ny, int nz, int sx, int sy, int sz);
int MathUtils_microCalcLight(int nx, int ny, int nz, int sx, int sy, int sz);

int MathUtils_getAnglez  (int x, int y, int x1, int y1);
int MathUtils_getAnglezHQ(int x, int y, int x1, int y1);

int MathUtils_fixDegree (int degree);
int MathUtils_fixDegree2(int degree);

int MathUtils_mix(int a, int b, int i, int max);

int MathUtils_pLength3 (int x, int y, int z);
int MathUtils_pLength2 (int x, int y);

int MathUtils_angleDistance(int a, int b);
int MathUtils_preudoRandom(long time, int val);
int MathUtils_round(double d);
int MathUtils_ceil (double d);

#endif
