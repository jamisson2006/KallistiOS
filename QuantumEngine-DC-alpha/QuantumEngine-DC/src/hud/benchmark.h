#ifndef QE_HUD_BENCHMARK_H
#define QE_HUD_BENCHMARK_H
#include "gui_screen.h"
#include <stdint.h>

typedef struct Main     Main;
typedef struct Menu     Menu;
typedef struct DirectX7 DirectX7;
typedef struct Texture  Texture;
typedef struct Vertex   Vertex;

typedef struct Benchmark {
    GUIScreen  base;
    Main      *main;
    Menu      *menu;
    DirectX7  *g3d;
    Texture   *tex;
    Vertex    *a, *b, *c;
    long long  AffTime, PersTime, PersTimeFloor;
    int8_t     rendermode;
} Benchmark;

Benchmark *Benchmark_new(Main *main, Menu *menu);
void       Benchmark_free(Benchmark *self);
#endif
