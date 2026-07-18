/*
 * speed_test.c — porte de code/Rendering/SpeedTest.java
 */
#include "speed_test.h"
#include "texturing_affine.h"

void SpeedTest_paintAffine_0(DirectX7 *g3d, Texture *texture,
    Vertex *a, int au, int av,
    Vertex *b, int bu, int bv,
    Vertex *c, int cu, int cv) {
    /* Delega ao TexturingAffine (implementacao fiel de mode 0). */
    TexturingAffine_paintAffine_0(g3d, texture, a, au, av, b, bu, bv, c, cu, cv);
}
