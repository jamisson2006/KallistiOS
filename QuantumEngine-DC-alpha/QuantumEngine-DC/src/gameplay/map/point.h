/*
 * point.h — porte fiel de code/Gameplay/Map/Point.java
 */
#ifndef QE_GAMEPLAY_MAP_POINT_H
#define QE_GAMEPLAY_MAP_POINT_H

#include <math.h>

typedef struct Point {
    int x, y;
} Point;

static inline int Point_distance(Point *a, Point *b) {
    int dx = a->x - b->x;
    int dy = a->y - b->y;
    return (int)sqrt((double)dx * dx + (double)dy * dy);
}

#endif
