/* stub mínimo de kos.h só para teste de host da renderização */
#ifndef STUB_KOS_H
#define STUB_KOS_H
#include <stdint.h>
#include <stdlib.h>
typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
static inline uint64 timer_ms_gettime64(void){return 0;}
#define DM_640x480 0
#define PM_RGB565 0
static inline void vid_set_mode(int a,int b){(void)a;(void)b;}
#endif
#define INIT_DEFAULT 0
#define KOS_INIT_FLAGS(x)
#define KOS_INIT_ROMDISK(x)
