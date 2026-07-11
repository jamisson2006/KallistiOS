/* Testa o pipeline 3D fiel: renderiza a cena de teste e salva PPM. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/math3d/matrix.h"
#include "../src/render3d/directx7.h"
#include "../src/render3d/polygon.h"
#include "../src/render3d/renderbuffer.h"
#include "../src/render/framebuffer.h"

#define W 320
#define H 240
#define GRID_N 8
#define CELL 1024

static RawImage *checker(qcolor c1, qcolor c2) {
    RawImage *ri = ri_create(64, 64);
    for (int y = 0; y < 64; y++)
        for (int x = 0; x < 64; x++)
            ri->px[y*64+x] = (((x>>3)^(y>>3))&1) ? c1 : c2;
    return ri;
}

int main(void) {
    mtx_static_init();

    qcolor *display = malloc(sizeof(qcolor)*W*H);
    for (int i = 0; i < W*H; i++) display[i] = (i < W*H/2) ? RGB(24,32,56) : RGB(10,10,14);

    DirectX7 g3d;
    dx7_init(&g3d, W, H, display);
    dx7_fDist = 12000;

    Texture3 *tf = tex3_from_rawimage(checker(RGB(70,80,90), RGB(40,46,54)));
    Texture3 *tw = tex3_from_rawimage(checker(RGB(140,90,60), RGB(96,60,40)));

    /* geometria: chao + 1 cubo */
    static Vertex3 world[512], xf[512];
    static Polygon4V quads[256];
    int nv = 0, nq = 0;

    int base = nv;
    for (int gz = 0; gz <= GRID_N; gz++)
        for (int gx = 0; gx <= GRID_N; gx++) {
            world[nv].x = (gx-GRID_N/2)*CELL;
            world[nv].y = 0;
            world[nv].z = (gz-GRID_N/2)*CELL;
            nv++;
        }
    for (int gz = 0; gz < GRID_N; gz++)
        for (int gx = 0; gx < GRID_N; gx++) {
            int i00 = base + gz*(GRID_N+1)+gx, i10=i00+1, i01=i00+GRID_N+1, i11=i01+1;
            p4v_init(&quads[nq], &xf[i00], &xf[i10], &xf[i11], &xf[i01],
                     0,0, 255,0, 255,255, 0,255);
            quads[nq].fog = 12;
            nq++;
        }
    int cube_q0 = nq;
    { /* cubo em (2*CELL, 0, 2*CELL) */
        int cx = 2*CELL, cz = 2*CELL, s = CELL/2, vb = nv;
        const int off[8][3] = {{-1,0,-1},{1,0,-1},{1,0,1},{-1,0,1},
                               {-1,2,-1},{1,2,-1},{1,2,1},{-1,2,1}};
        for (int i = 0; i < 8; i++) {
            world[nv].x = cx+off[i][0]*s; world[nv].y = off[i][1]*s; world[nv].z = cz+off[i][2]*s;
            nv++;
        }
        static const int f[6][4] = {{4,5,1,0},{6,7,3,2},{7,4,0,3},{5,6,2,1},{7,6,5,4},{0,1,2,3}};
        for (int i = 0; i < 6; i++) {
            p4v_init(&quads[nq], &xf[vb+f[i][0]], &xf[vb+f[i][1]],
                     &xf[vb+f[i][2]], &xf[vb+f[i][3]], 0,0,255,0,255,255,0,255);
            quads[nq].fog = 12;
            nq++;
        }
    }

    /* camera: player em (0, 700, -1500) olhando p/ frente */
    Camera cam; cam_init(&cam);
    cam.x = cam.y = cam.z = 0; cam.smoothSteps = 1;
    cam.rotX = 10; cam.rotY = 25;

    Matrix pos; mtx_identity(&pos);
    mtx_set_position(&pos, 0, 700, -1500);
    cam_set(&cam, &pos, 0, 0, 0, 0);
    dx7_set_camera(&g3d, cam_get(&cam));

    Matrix ident; mtx_identity(&ident);
    Matrix *fin = dx7_compute_final(&g3d, &ident);
    for (int i = 0; i < nv; i++) {
        xf[i] = world[i];
        vx_transform(&xf[i], fin);
        vx_project(&xf[i], &g3d);
    }

    RenderObjectBuffer rob; rob_init(&rob);
    int vis = 0;
    for (int i = 0; i < nq; i++) {
        int before = rob.size;
        rob_add_p4v(&rob, &quads[i], (i < cube_q0) ? tf : tw, 0, 0, W, H);
        if (rob.size > before) vis++;
    }
    printf("quads visiveis: %d de %d\n", vis, nq);
    rob_render(&rob, &g3d);

    FILE *f = fopen("scene3d.ppm", "wb");
    fprintf(f, "P6\n%d %d\n255\n", W, H);
    for (int i = 0; i < W*H; i++) {
        unsigned char rgb[3] = {COL_R(display[i]), COL_G(display[i]), COL_B(display[i])};
        fwrite(rgb, 1, 3, f);
    }
    fclose(f);
    printf("OK: scene3d.ppm\n");
    return 0;
}
