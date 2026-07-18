/* Teste de host: renderiza o layout do Splash e do Menu no Canvas
   e salva PPMs, para validar fonte + composicao antes do WSL. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../src/render/framebuffer.h"
#include "../src/render/font.h"

/* Reimplementa tr() e o layout do menu de forma minima p/ o teste,
   reusando as MESMAS funcoes de desenho (cv_*, font_*). */
static void save_ppm(const char*name, Canvas*c){
    FILE*f=fopen(name,"wb");
    fprintf(f,"P6\n%d %d\n255\n",c->w,c->h);
    for(int i=0;i<c->w*c->h;i++){
        unsigned char rgb[3]={COL_R(c->px[i]),COL_G(c->px[i]),COL_B(c->px[i])};
        fwrite(rgb,1,3,f);
    }
    fclose(f);
}

int main(void){
    font_init();
    Canvas c; c.w=CANVAS_W; c.h=CANVAS_H;
    c.px=malloc(sizeof(qcolor)*c.w*c.h);

    /* ---- Splash ---- */
    for(int y=0;y<c.h;y++){int v=8+(y*40)/c.h;cv_hline(&c,0,y,c.w,RGB(v,v,v+6));}
    int cx=c.w/2, cy=c.h/2;
    cv_rect(&c,cx-70,cy-34,140,44,RGB(60,120,200));
    font_set_scale(2); font_draw(&c,"QUANTUM",cx,cy-24,ANCHOR_HCENTER,1);
    font_set_scale(1); font_draw(&c,"E N G I N E",cx,cy-2,ANCHOR_HCENTER,0);
    font_draw(&c,"Dreamcast Port - Alpha",cx,cy+26,ANCHOR_HCENTER,2);
    save_ppm("splash.ppm",&c);

    /* ---- Menu ---- */
    for(int y=0;y<c.h;y++){int b=20+(y*30)/c.h;cv_hline(&c,0,y,c.w,RGB(6,10,b));}
    font_set_scale(2); font_draw(&c,"QUANTUM",cx,18,ANCHOR_HCENTER,1);
    font_set_scale(1); font_draw(&c,"ENGINE",cx,40,ANCHOR_HCENTER,0);
    const char*items[4]={"New Game","Help","Options","Exit"};
    int fh=font_height(), step=fh+4, total=4*step, posy=56+(c.h-96)/2-total/2;
    for(int i=0;i<4;i++){
        int col=(i==0)?1:0;
        if(i==0){int tw=font_width_of(items[i]);cv_fill_rect(&c,cx-tw/2-8,posy-2,tw+16,fh+4,RGB(28,44,70));}
        font_draw(&c,items[i],cx,posy,ANCHOR_HCENTER,col); posy+=step;
    }
    font_draw(&c,"Select",4,c.h-12,ANCHOR_LEFT,1);
    font_draw(&c,"Back",c.w-4,c.h-12,ANCHOR_RIGHT,0);
    save_ppm("menu.ppm",&c);

    printf("OK: splash.ppm e menu.ppm gerados\n");
    return 0;
}
