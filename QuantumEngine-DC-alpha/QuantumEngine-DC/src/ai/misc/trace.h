/*
 * trace.h — porte fiel de code/AI/misc/Trace.java
 */
#ifndef QE_AI_MISC_TRACE_H
#define QE_AI_MISC_TRACE_H

#include "../../gameplay/objects/game_object.h"
#include "../../math/vector3d.h"

typedef struct Polygon4V Polygon4V;
typedef struct Vertex    Vertex;
typedef struct Texture   Texture;
typedef struct DirectX7  DirectX7;
typedef struct House     House;
typedef struct Player    Player;
typedef struct GameScreen GameScreen;

typedef struct Trace {
    GameObject  base;
    Polygon4V  *pol4;
    Vertex     *pos;
    int         f;
    Vector3D    vec;
    Texture    *tex;
} Trace;

Trace *Trace_new(int fx, int fy, int fz,
                 Vector3D *v1, Vector3D *v2, Vector3D *v3, Vector3D *v4,
                 Vector3D *size);
void   Trace_render(Trace *self, DirectX7 *g3d, int x1, int y1, int x2, int y2);
void   Trace_activate(Trace *self, House *house, Player *player, GameScreen *gs);

#endif
