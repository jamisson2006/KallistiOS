/* ============================================================
 *  host_collision_test.c - Teste funcional (host) da colisao
 *
 *  Monta um chao (quad y=0, normal -Y) em memoria e verifica:
 *   1. HeightComputer: altura em (0,500,0) deve ser 0
 *   2. SphereCast: esfera (0,80,0) r=100 empurrada para y=100
 *   3. RayCast: raio (0,500,0) dir (0,-1000,0) acerta a 500
 *   4. invSqrt: constante do Java (1597463007)
 *
 *  build: gcc -Isrc -Ivendor -std=gnu11 tests/host_collision_test.c
 *         + fontes de math3d, render3d/directx7 e collision, com -lm
 * ============================================================ */
#include <stdio.h>
#include <string.h>
#include "../src/collision/raycast.h"
#include "../src/collision/spherecast.h"
#include "../src/collision/heightcomputer.h"
#include "../src/math3d/mathutils2.h"
#include "../src/render/raw_image.h"

/* ---- stubs de plataforma (o teste nao carrega PNG nem loga) ---- */
int qfps_frameTime = 50;
void log_warn(const char *fmt, ...) { (void)fmt; }
void log_info(const char *fmt, ...) { (void)fmt; }
RawImage *ri_load(const char *res_path) { (void)res_path; return 0; }
void ri_free(RawImage *ri) { (void)ri; }

static int fails = 0;
#define CHECK(cond, msg) do { \
    if (cond) printf("  OK   %s\n", msg); \
    else { printf("  FAIL %s\n", msg); fails++; } \
} while (0)

int main(void) {
    /* ---------- chao: quad em y=0, 2000x2000, normal (0,-4096,0) ----------
     * winding (a,b,c,d) escolhido como o loader .3d produz para um chao:
     * a=(-1000,0,1000) b=(1000,0,1000) c=(1000,0,-1000) d=(-1000,0,-1000) */
    /* winding consistente com calcNormal => ny = -4096 (chao) */
    Vertex3 verts[4] = {
        { -1000, 0, -1000, 0, 0, 0 },
        {  1000, 0, -1000, 0, 0, 0 },
        {  1000, 0,  1000, 0, 0, 0 },
        { -1000, 0,  1000, 0, 0, 0 },
    };
    Polygon4V quad;
    memset(&quad, 0, sizeof quad);
    quad.a = &verts[0]; quad.b = &verts[1];
    quad.c = &verts[2]; quad.d = &verts[3];
    quad.nx = 0; quad.ny = -4096; quad.nz = 0;
    quad.tex = 0;

    Mesh mesh;
    memset(&mesh, 0, sizeof mesh);
    mesh.vertices = verts; mesh.nverts = 4;
    mesh.p4 = &quad;       mesh.np4 = 1;
    mesh.p3 = 0;           mesh.np3 = 0;
    mesh.texture = 0;

    printf("== HeightComputer ==\n");
    int32_t h = hc_compute_height_xyz(&mesh, 0, 500, 0);
    printf("  altura em (0,500,0) = %d (esperado 0)\n", h);
    CHECK(h == 0, "computeHeight acha o chao em y=0");

    h = hc_compute_height_xyz(&mesh, 5000, 500, 0);
    CHECK(h == INT32_MIN, "fora do quad devolve Integer.MIN_VALUE");

    Height hh;
    height_init(&hh);
    v3_set(height_get_position(&hh), 250, 500, -250);
    hc_compute_height(&mesh, &hh);
    CHECK(height_get_height(&hh) == 0, "altura dentro do quad (250,-250) = 0");
    CHECK(height_get_polygon(&hh).ptr == &quad, "Height.polygon aponta o quad");
    CHECK(height_get_centre_x(&hh) == 0 && height_get_centre_z(&hh) == 0,
          "centro do poligono (0,0)");

    printf("== SphereCast ==\n");
    Vector3D pos; v3_set(&pos, 0, 80, 0);
    int col = sc_sphere_cast(&mesh, &pos, 100);
    printf("  pos apos cast = (%d,%d,%d) (esperado 0,100,0)\n", pos.x, pos.y, pos.z);
    CHECK(col == 1, "esfera penetrando o chao colide");
    CHECK(pos.x == 0 && pos.y == 100 && pos.z == 0,
          "esfera empurrada para fora ate y=100");

    v3_set(&pos, 0, 500, 0);
    col = sc_sphere_cast(&mesh, &pos, 100);
    CHECK(col == 0 && pos.y == 500, "esfera longe do chao nao colide");

    printf("== RayCast ==\n");
    Ray ray; ray_init(&ray);
    CHECK(ray.onlyCollidable == 1 && ray.numRoom == -1 &&
          ray.distance == INT32_MAX, "new Ray(): defaults do Java");

    v3_set(&ray.start, 0, 500, 0);
    v3_set(&ray.dir, 0, -1000, 0);
    rc_ray_cast(&mesh, &ray);
    printf("  distance=%d colPoint=(%d,%d,%d)\n", ray.distance,
           ray.collisionPoint.x, ray.collisionPoint.y, ray.collisionPoint.z);
    CHECK(ray.collision == 1, "raio para baixo acerta o chao");
    CHECK(ray.distance == 500, "distancia = 500");
    CHECK(ray.collisionPoint.y == 0, "ponto de colisao em y=0");
    CHECK(ray.triangle.ptr == &quad && ray.triangle.kind == QE_POLY_4V,
          "triangle aponta o quad");

    ray_reset(&ray);
    v3_set(&ray.start, 0, 500, 0);
    v3_set(&ray.dir, 0, 1000, 0);        /* para CIMA: nao pode acertar */
    rc_ray_cast(&mesh, &ray);
    CHECK(ray.collision == 0, "raio para cima nao acerta (backface)");

    ray_reset(&ray);
    v3_set(&ray.start, 0, 500, 0);
    v3_set(&ray.dir, 0, -100, 0);        /* curto demais (t>4096) */
    rc_ray_cast(&mesh, &ray);
    CHECK(ray.collision == 0, "raio curto nao alcanca (t>4096 sem infinity)");

    ray.infinity = 1;                     /* com infinity alcanca */
    rc_ray_cast(&mesh, &ray);
    CHECK(ray.collision == 1 && ray.distance == 500, "com infinity alcanca a 500");

    printf("== superFastRayCast ==\n");
    Ray fray; ray_init(&fray);
    v3_set(&fray.start, 100, 500, 100);
    v3_set(&fray.dir, 0, -2000, 0);
    rc_super_fast_ray_cast(&mesh, &fray);
    CHECK(fray.collision == 1 && fray.distance == 500,
          "superFast acerta o chao a 500");

    printf("== MathUtils2 ==\n");
    /* invSqrt do Java: invSqrt(4.0f) com magic 1597463007.
     * Java: bits(4.0f)=0x40800000; i=1597463007-0x20400000=0x3EF75A86;
     * val=0.48311...; ret = val*(1.5-2*val*val) = 0.499154...       */
    float is = mu2_inv_sqrt_f(4.0f);
    printf("  invSqrt(4)=%.6f (esperado ~0.4991)\n", is);
    CHECK(is > 0.498f && is < 0.5f, "invSqrt bate com a formula do Java");

    Vector3D a = { 0, 0, 0 }, b = { 1000, 0, 0 }, c = { 0, 0, 1000 };
    Vector3D nor;
    mu2_calc_normal(&nor, &a, &b, &c);
    printf("  normal=(%d,%d,%d)\n", nor.x, nor.y, nor.z);
    CHECK(nor.x == 0 && nor.z == 0 && (nor.y == 4096 || nor.y == -4096),
          "calcNormal em fp12 (|n|=4096)");

    Vector3D p = { 100, 0, 100 };
    CHECK(mu2_distance_to_line(&p, &a, &b) == 10001,
          "distanceToLine^2 = 10001 (truncamento fp14 igual ao Java)");

    printf("\n%s (%d falhas)\n", fails ? "*** FALHOU ***" : "TODOS OS TESTES PASSARAM", fails);
    return fails ? 1 : 0;
}
