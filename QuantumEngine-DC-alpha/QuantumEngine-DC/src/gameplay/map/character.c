/*
 * character.c — porte fiel de code/Gameplay/Map/Character.java
 */
#include "character.h"
#include <stdlib.h>
#include <string.h>

/* Dependencias externas */
extern int  House_getFloorHeight(House *house, int x, int z, int part);
extern int  House_getCeilingHeight(House *house, int x, int z, int part);
extern int  House_findPart(House *house, int x, int z, int currentPart);
extern bool House_isWall(House *house, int x, int z, int radius, int part);

#define GRAVITY 32
#define FP_SHIFT 14

void Character_init(Character *self, int radius, int height) {
    memset(self, 0, sizeof(Character));
    Matrix_setIdentity(&self->transform);
    Vector3D_set(&self->speed, 0, 0, 0);
    Vector3D_set(&self->speedNoFriction, 0, 0, 0);
    self->player_radius = radius;
    self->player_height = height;
    self->height.floor = 0;
    self->height.ceiling = 0;
    self->onFloor = false;
    self->fly = false;
    self->underRoof = false;
    self->collision = true;
    self->collidable = true;
    self->updatable = true;
    self->part = 0;
}

void Character_destroy(Character *self) {
    /* Nothing heap-allocated inside Character itself */
    (void)self;
}

void Character_collisionTest(Character *self, House *house) {
    if (!self->collision) return;

    int px = self->transform.m03;
    int py = self->transform.m13;
    int pz = self->transform.m23;

    /* Update part/room */
    int newPart = House_findPart(house, px, pz, self->part);
    if (newPart >= 0) {
        self->part = newPart;
    }

    /* Floor/ceiling heights */
    int floorH = House_getFloorHeight(house, px, pz, self->part);
    int ceilingH = House_getCeilingHeight(house, px, pz, self->part);
    self->height.floor = floorH;
    self->height.ceiling = ceilingH;

    /* Apply speed */
    px += self->speed.x + self->speedNoFriction.x;
    pz += self->speed.z + self->speedNoFriction.z;

    /* Wall collision */
    self->collided = false;
    if (House_isWall(house, px, pz, self->player_radius, self->part)) {
        self->collided = true;
        /* Try X only */
        int testX = self->transform.m03 + self->speed.x + self->speedNoFriction.x;
        int testZ = self->transform.m23;
        if (!House_isWall(house, testX, testZ, self->player_radius, self->part)) {
            px = testX;
            pz = testZ;
        } else {
            /* Try Z only */
            testX = self->transform.m03;
            testZ = self->transform.m23 + self->speed.z + self->speedNoFriction.z;
            if (!House_isWall(house, testX, testZ, self->player_radius, self->part)) {
                px = testX;
                pz = testZ;
            } else {
                /* No movement */
                px = self->transform.m03;
                pz = self->transform.m23;
            }
        }
    }

    /* Vertical: gravity and floor */
    if (!self->fly) {
        self->speed.y -= GRAVITY;
        py += self->speed.y;

        if (py <= floorH) {
            py = floorH;
            self->speed.y = 0;
            self->onFloor = true;
        } else {
            self->onFloor = false;
        }

        /* Ceiling */
        if (py + self->player_height > ceilingH) {
            py = ceilingH - self->player_height;
            self->speed.y = 0;
            self->underRoof = true;
        } else {
            self->underRoof = false;
        }
    } else {
        py += self->speed.y;
        self->onFloor = false;
    }

    /* Apply final position */
    self->transform.m03 = px;
    self->transform.m13 = py;
    self->transform.m23 = pz;

    /* Reset speedNoFriction each frame */
    Vector3D_set(&self->speedNoFriction, 0, 0, 0);
}

void Character_collisionTestStatic(Character *body1, Character *body2) {
    if (!body1->collidable || !body2->collidable) return;

    int dx = body1->transform.m03 - body2->transform.m03;
    int dz = body1->transform.m23 - body2->transform.m23;
    int dist2 = dx * dx + dz * dz;
    int minDist = body1->player_radius + body2->player_radius;

    if (dist2 < minDist * minDist && dist2 > 0) {
        /* Push bodies apart */
        /* Approximate: shift each by half the overlap along dx,dz */
        int dist = 1;
        /* Integer sqrt approximation */
        int tmp = dist2;
        while (tmp > 0) {
            dist++;
            tmp -= dist;
        }
        int overlap = minDist - dist;
        if (overlap > 0) {
            int pushX = (dx * overlap) / (dist * 2);
            int pushZ = (dz * overlap) / (dist * 2);
            body1->transform.m03 += pushX;
            body1->transform.m23 += pushZ;
            body2->transform.m03 -= pushX;
            body2->transform.m23 -= pushZ;
        }
    }
}

void Character_moveZ(Character *self, int amount) {
    /* Move along transform's forward direction (m20, m22) */
    self->speed.x += (self->transform.m20 * amount) >> FP_SHIFT;
    self->speed.z += (self->transform.m22 * amount) >> FP_SHIFT;
}

void Character_moveX(Character *self, int amount) {
    /* Move along transform's side direction (m00, m02) */
    self->speed.x += (self->transform.m00 * amount) >> FP_SHIFT;
    self->speed.z += (self->transform.m02 * amount) >> FP_SHIFT;
}

void Character_jump(Character *self, int force) {
    if (self->onFloor) {
        self->speed.y = force;
        self->onFloor = false;
    }
}

void Character_rotY(Character *self, int degrees) {
    Matrix_rotY(&self->transform, degrees);
}

void Character_drop(Character *self, int speed) {
    self->transform.m11 += speed;
    if (self->transform.m11 < 0) self->transform.m11 = 0;
}

void Character_dropSide(Character *self, int speed) {
    self->transform.m00 += speed;
    if (self->transform.m00 < 0) self->transform.m00 = 0;
}

void Character_setPosition(Character *self, int x, int y, int z) {
    Matrix_setPosition(&self->transform, x, y, z);
}

void Character_setPositionV(Character *self, Vector3D *pos) {
    Matrix_setPositionV(&self->transform, pos);
}

Matrix *Character_getTransform(Character *self) {
    return &self->transform;
}

Vector3D *Character_getSpeed(Character *self) {
    return &self->speed;
}

int Character_getRadius(Character *self) {
    return self->player_radius;
}

int Character_getHeight(Character *self) {
    return self->player_height;
}

int Character_getPart(Character *self) {
    return self->part;
}

int Character_getX(Character *self) {
    return self->transform.m03;
}

int Character_getY(Character *self) {
    return self->transform.m13;
}

int Character_getZ(Character *self) {
    return self->transform.m23;
}

bool Character_isCollider(Character *self) {
    return self->collision;
}

bool Character_isCollision(Character *self) {
    return self->collided;
}

bool Character_isOnFloor(Character *self) {
    return self->onFloor;
}

void Character_setCollision(Character *self, bool collision) {
    self->collision = collision;
}

void Character_setCollidable(Character *self, bool collidable) {
    self->collidable = collidable;
}

void Character_setUpdatable(Character *self, bool updatable) {
    self->updatable = updatable;
}

void Character_setFly(Character *self, bool fly) {
    self->fly = fly;
}

void Character_applyFriction(Character *self, float friction) {
    self->speed.x = (int)(self->speed.x * friction);
    self->speed.z = (int)(self->speed.z * friction);
}

void Character_reset(Character *self) {
    Vector3D_set(&self->speed, 0, 0, 0);
    Vector3D_set(&self->speedNoFriction, 0, 0, 0);
    self->onFloor = false;
    self->collision = true;
    self->collidable = true;
    self->updatable = true;
    self->oldFloorPoly = NULL;
}

void Character_jump2(Character *self, int height, float speed) {
    if (self->onFloor) {
        self->speed.y = (int)(height * speed);
        self->onFloor = false;
    }
}

void Character_setOnFloor(Character *self, bool f) {
    self->onFloor = f;
}

void Character_setSpeedZero(Character *self) {
    Vector3D_set(&self->speed, 0, 0, 0);
    Vector3D_set(&self->speedNoFriction, 0, 0, 0);
}

RenderObject *Character_getOldFloorPoly(Character *self) {
    return self->oldFloorPoly;
}

int64_t Character_distance(Character *a, Character *b) {
    int64_t dx = a->transform.m03 - b->transform.m03;
    int64_t dy = a->transform.m13 - b->transform.m13;
    int64_t dz = a->transform.m23 - b->transform.m23;
    return dx * dx + dy * dy + dz * dz;
}
