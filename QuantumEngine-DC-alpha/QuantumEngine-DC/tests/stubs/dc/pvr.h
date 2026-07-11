/* stub mínimo de dc/pvr.h */
#ifndef STUB_PVR_H
#define STUB_PVR_H
#include <stdint.h>
typedef void* pvr_ptr_t;
typedef struct { int a[16]; } pvr_init_params_t;
typedef struct { int a[16]; } pvr_poly_cxt_t;
typedef struct { int a[16]; } pvr_poly_hdr_t;
typedef struct { int flags; float x,y,z,u,v; uint32_t argb,oargb; } pvr_vertex_t;
#define PVR_BINSIZE_16 0
#define PVR_BINSIZE_0 0
#define PVR_LIST_OP_POLY 0
#define PVR_TXRFMT_RGB565 0
#define PVR_TXRFMT_NONTWIDDLED 0
#define PVR_FILTER_NONE 0
#define PVR_CMD_VERTEX 0
#define PVR_CMD_VERTEX_EOL 1
static inline int  pvr_init(pvr_init_params_t*p){(void)p;return 0;}
static inline void pvr_shutdown(void){}
static inline pvr_ptr_t pvr_mem_malloc(size_t n){return malloc(n);}
static inline void pvr_mem_free(pvr_ptr_t p){free(p);}
static inline void pvr_txr_load(void*a,pvr_ptr_t b,uint32_t n){(void)a;(void)b;(void)n;}
static inline void pvr_poly_cxt_txr(pvr_poly_cxt_t*c,int a,int b,int w,int h,pvr_ptr_t t,int f){(void)c;(void)a;(void)b;(void)w;(void)h;(void)t;(void)f;}
static inline void pvr_poly_compile(pvr_poly_hdr_t*h,pvr_poly_cxt_t*c){(void)h;(void)c;}
static inline void pvr_list_begin(int a){(void)a;}
static inline void pvr_list_finish(void){}
static inline void pvr_prim(void*p,size_t n){(void)p;(void)n;}
static inline void pvr_wait_ready(void){}
static inline void pvr_scene_begin(void){}
static inline void pvr_scene_finish(void){}
#endif
