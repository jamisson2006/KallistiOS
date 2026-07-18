/*
 * height_computer.h — porte fiel de code/Collision/HeightComputer.java
 */
#ifndef QE_COLLISION_HEIGHT_COMPUTER_H
#define QE_COLLISION_HEIGHT_COMPUTER_H

#include <stdbool.h>
#include "height.h"

typedef struct Mesh   Mesh;
typedef struct Matrix Matrix;

bool HeightComputer_isPointAABBCollision(int x, int z, int minx, int maxx, int minz, int maxz);

int  HeightComputer_computeHeightSimple(Mesh *mesh, int x, int y, int z);

void HeightComputer_computeHeight(Mesh *mesh, Height *height);
void HeightComputer_computeHeightMatrix(Mesh *mesh, Matrix *matrix, Height *height);
void HeightComputer_computeHeightMatrixFull(Mesh *mesh, Matrix *matrix, Height *height, bool updatePos);

#endif
