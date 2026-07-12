/*
 * house_creator.h — porte fiel de code/Gameplay/Map/HouseCreator.java
 */
#ifndef QE_GAMEPLAY_MAP_HOUSE_CREATOR_H
#define QE_GAMEPLAY_MAP_HOUSE_CREATOR_H

#include <stdbool.h>

typedef struct House House;
typedef struct Mesh  Mesh;

/*
 * Build a House from an array of room meshes.
 * Detects portals via shared vertices, builds neighbours matrix,
 * applies lightmapping.
 */
House *HouseCreator_create(Mesh **meshes, int meshCount, bool noPortals,
                           const char *lightdataFile, bool optimizeLevel);

#endif
