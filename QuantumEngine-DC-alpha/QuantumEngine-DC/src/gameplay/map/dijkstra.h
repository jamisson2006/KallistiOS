/*
 * dijkstra.h — porte fiel de code/Gameplay/Map/Dijkstra.java
 */
#ifndef QE_GAMEPLAY_MAP_DIJKSTRA_H
#define QE_GAMEPLAY_MAP_DIJKSTRA_H

typedef struct House House;

typedef struct Dijkstra {
    int8_t **ways;
    int      roomCount;
} Dijkstra;

Dijkstra *Dijkstra_new(House *home);
void      Dijkstra_free(Dijkstra *d);
int       Dijkstra_getNext(Dijkstra *d, int start, int finish);
int       Dijkstra_getPrevious(Dijkstra *d, int start, int finish);

#endif
