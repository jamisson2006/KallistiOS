/*
 * speed_test.h — porte de code/Rendering/SpeedTest.java
 *
 * Modulo de benchmark comparativo dos rasterizadores. No Dreamcast eh
 * mais util o benchmark real do FPS na HUD/Benchmark. Este arquivo
 * preserva a API mas delega para TexturingAffine.
 */
#ifndef QE_RENDERING_SPEED_TEST_H
#define QE_RENDERING_SPEED_TEST_H

typedef struct Vertex   Vertex;
typedef struct Texture  Texture;
typedef struct DirectX7 DirectX7;

void SpeedTest_paintAffine_0(DirectX7 *g3d, Texture *texture,
    Vertex *a, int au, int av,
    Vertex *b, int bu, int bv,
    Vertex *c, int cu, int cv);

#endif
