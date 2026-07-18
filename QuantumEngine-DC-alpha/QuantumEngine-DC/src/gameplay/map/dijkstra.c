/*
 * dijkstra.c — porte fiel de code/Gameplay/Map/Dijkstra.java
 */
#include "dijkstra.h"
#include "house.h"
#include "edge.h"
#include "qitem.h"
#include "point.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>

static int dis_rooms(Room *r1, Room *r2) {
    int cx1 = (Room_getMinX(r1) + Room_getMaxX(r1)) / 2;
    int cz1 = (Room_getMinZ(r1) + Room_getMaxZ(r1)) / 2;
    int cx2 = (Room_getMinX(r2) + Room_getMaxX(r2)) / 2;
    int cz2 = (Room_getMinZ(r2) + Room_getMaxZ(r2)) / 2;
    int dx = cx2 - cx1;
    int dz = cz2 - cz1;
    return (int)sqrt((double)dx * dx + (double)dz * dz);
}

typedef struct EdgeList {
    Edge *edges;
    int   count;
} EdgeList;

static QItem poll_min(QItem *items, int *count) {
    int minIdx = 0;
    for (int i = 1; i < *count; i++) {
        if (items[i].dis < items[minIdx].dis) minIdx = i;
    }
    QItem result = items[minIdx];
    items[minIdx] = items[*count - 1];
    (*count)--;
    return result;
}

static void shortestPaths(EdgeList *edges, int numRooms, int start, int *prio, int *pred) {
    for (int i = 0; i < numRooms; i++) {
        pred[i] = -1;
        prio[i] = INT_MAX;
    }
    prio[start] = 0;

    QItem *q = (QItem *)malloc(numRooms * numRooms * sizeof(QItem));
    int qCount = 0;
    q[qCount].dis = 0;
    q[qCount].u = start;
    qCount++;

    while (qCount > 0) {
        QItem cur = poll_min(q, &qCount);
        if (cur.dis != prio[cur.u]) continue;
        EdgeList *el = &edges[cur.u];
        for (int i = 0; i < el->count; i++) {
            int v = el->edges[i].t;
            int nprio = prio[cur.u] + el->edges[i].cost;
            if (prio[v] > nprio) {
                prio[v] = nprio;
                pred[v] = cur.u;
                q[qCount].dis = nprio;
                q[qCount].u = v;
                qCount++;
            }
        }
    }
    free(q);
}

static int8_t **calcWays(House *home) {
    int numRooms = House_getRoomCount(home);
    Room **rooms = House_getRooms(home);

    EdgeList *edges = (EdgeList *)calloc(numRooms, sizeof(EdgeList));
    for (int y = 0; y < numRooms; y++) {
        edges[y].edges = (Edge *)malloc(numRooms * sizeof(Edge));
        edges[y].count = 0;
        for (int x = 0; x < numRooms; x++) {
            if (y != x && House_isNear(home, x, y)) {
                int d = dis_rooms(rooms[x], rooms[y]);
                edges[y].edges[edges[y].count].t = x;
                edges[y].edges[edges[y].count].cost = d;
                edges[y].count++;
            }
        }
    }

    int8_t **ways = (int8_t **)malloc(numRooms * sizeof(int8_t *));
    int *prio = (int *)malloc(numRooms * sizeof(int));
    int *pred = (int *)malloc(numRooms * sizeof(int));

    for (int y = 0; y < numRooms; y++) {
        ways[y] = (int8_t *)malloc(numRooms * sizeof(int8_t));
        shortestPaths(edges, numRooms, y, prio, pred);
        for (int x = 0; x < numRooms; x++) {
            ways[y][x] = (int8_t)pred[x];
        }
    }

    free(prio);
    free(pred);
    for (int i = 0; i < numRooms; i++) free(edges[i].edges);
    free(edges);

    return ways;
}

Dijkstra *Dijkstra_new(House *home) {
    Dijkstra *d = (Dijkstra *)calloc(1, sizeof(Dijkstra));
    d->roomCount = House_getRoomCount(home);
    d->ways = calcWays(home);
    return d;
}

void Dijkstra_free(Dijkstra *d) {
    if (!d) return;
    if (d->ways) {
        for (int i = 0; i < d->roomCount; i++) free(d->ways[i]);
        free(d->ways);
    }
    free(d);
}

int Dijkstra_getNext(Dijkstra *d, int start, int finish) {
    return Dijkstra_getPrevious(d, finish, start);
}

int Dijkstra_getPrevious(Dijkstra *d, int start, int finish) {
    return d->ways[start][finish];
}
