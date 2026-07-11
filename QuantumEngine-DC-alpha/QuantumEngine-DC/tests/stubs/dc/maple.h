#ifndef STUB_MAPLE_H
#define STUB_MAPLE_H
#define MAPLE_FUNC_CONTROLLER 1
typedef struct maple_device { int x; } maple_device_t;
static inline maple_device_t* maple_enum_type(int u,int f){(void)u;(void)f;return 0;}
static inline void* maple_dev_status(maple_device_t*d){(void)d;return 0;}
#endif
