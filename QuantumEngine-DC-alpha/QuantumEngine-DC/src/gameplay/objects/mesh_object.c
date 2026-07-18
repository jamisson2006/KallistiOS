/*
 * mesh_object.c — porte fiel de code/Gameplay/Objects/MeshObject.java
 */
#include "mesh_object.h"
#include "../../math/matrix.h"
#include <stdlib.h>
#include <string.h>

/* Dependencias externas */
extern void Morphing_update(Morphing *m, float speed);
extern void Morphing_render(Morphing *m);
extern Matrix *Morphing_getTransform(Morphing *m);
extern void BoundingBox_update(BoundingBox *bb, Matrix *transform);
extern bool BoundingBox_sphereTest(BoundingBox *bb, int x, int y, int z, int radius);
extern int  BoundingBox_computeHeight(BoundingBox *bb, int x, int z);
extern bool BoundingBox_rayCast(BoundingBox *bb, Vector3D *origin, Vector3D *dir);
extern int  Character_getX(Character *ch);
extern int  Character_getY(Character *ch);
extern int  Character_getZ(Character *ch);

void MeshObject_init(MeshObject *self, Morphing *morphing, MultyTexture *tex, BoundingBox *bbox) {
    memset(self, 0, sizeof(MeshObject));
    self->morphing = morphing;
    self->texture = tex;
    self->bbox = bbox;
    self->animSpeed = 1.0f;
    self->animType = MESH_OBJECT_ANIM_LOOP;
    self->state = MESH_OBJECT_STATE_STAY;
    self->liftState = MESH_OBJECT_STAY_BEGIN;
    self->liftPoseIndex = 0;
    self->liftFrame = 0;
    self->poses = NULL;
    self->posesCount = 0;
    self->timeToMove = NULL;
    self->liftCycled = false;
    self->liftReUse = false;
    self->disactivateOnEnd = false;
    self->liftCanBePaused = false;
    self->liftRotateToMove = false;
    self->liftSmoothMove = 0;
    self->playerFollowLift = NULL;
    self->addsz = 0;
    self->precCol = false;
    self->ignoreWeaponRayCast = false;
    self->realtimeLighting = false;
    self->targetDirX = 0;
    self->targetDirZ = 0;
    self->smoothDirSpeed = 0;
}

void MeshObject_destroy(MeshObject *self) {
    if (!self) return;
    if (self->poses) {
        free(self->poses);
        self->poses = NULL;
    }
    if (self->timeToMove) {
        free(self->timeToMove);
        self->timeToMove = NULL;
    }
}

void MeshObject_activate(MeshObject *self) {
    switch (self->animType) {
        case MESH_OBJECT_ANIM_OPEN:
            if (self->state == MESH_OBJECT_STATE_STAY) {
                self->state = MESH_OBJECT_STATE_OPEN;
                self->liftState = MESH_OBJECT_DRIVE;
                self->liftFrame = 0;
            }
            break;
        case MESH_OBJECT_ANIM_OPEN_CLOSE:
            if (self->state == MESH_OBJECT_STATE_STAY) {
                self->state = MESH_OBJECT_STATE_OPEN;
                self->liftState = MESH_OBJECT_DRIVE;
                self->liftFrame = 0;
            } else if (self->state == MESH_OBJECT_STATE_OPEN) {
                self->state = MESH_OBJECT_STATE_CLOSE;
                self->liftState = MESH_OBJECT_DRIVE_BACK;
                self->liftFrame = 0;
            }
            break;
        default:
            break;
    }
}

void MeshObject_getNewPos(MeshObject *self, Vector3D *out, int frame, int poseFrom, int poseTo, int totalTime) {
    if (totalTime <= 0) {
        Vector3D_setV(out, &self->poses[poseTo]);
        return;
    }
    int dx = self->poses[poseTo].x - self->poses[poseFrom].x;
    int dy = self->poses[poseTo].y - self->poses[poseFrom].y;
    int dz = self->poses[poseTo].z - self->poses[poseFrom].z;

    out->x = self->poses[poseFrom].x + (dx * frame) / totalTime;
    out->y = self->poses[poseFrom].y + (dy * frame) / totalTime;
    out->z = self->poses[poseFrom].z + (dz * frame) / totalTime;
}

void MeshObject_update(MeshObject *self) {
    /* Morphing animation */
    if (self->morphing) {
        Morphing_update(self->morphing, self->animSpeed);
    }

    /* Lift movement */
    if (self->poses == NULL || self->posesCount == 0) return;

    if (self->liftState == MESH_OBJECT_DRIVE || self->liftState == MESH_OBJECT_DRIVE_BACK) {
        int poseFrom, poseTo, totalTime;

        if (self->liftState == MESH_OBJECT_DRIVE) {
            poseFrom = self->liftPoseIndex;
            poseTo = self->liftPoseIndex + 1;
            if (poseTo >= self->posesCount) poseTo = self->posesCount - 1;
        } else {
            poseFrom = self->liftPoseIndex;
            poseTo = self->liftPoseIndex - 1;
            if (poseTo < 0) poseTo = 0;
        }

        totalTime = self->timeToMove[self->liftPoseIndex];

        self->liftFrame++;

        Vector3D newPos;
        MeshObject_getNewPos(self, &newPos, self->liftFrame, poseFrom, poseTo, totalTime);

        /* Apply position to transform */
        Matrix *transform = Morphing_getTransform(self->morphing);
        if (transform) {
            Matrix_setPosition(transform, newPos.x, newPos.y, newPos.z);
        }

        /* Player follow lift */
        if (self->playerFollowLift) {
            self->playerFollowLift->x = newPos.x - self->poses[poseFrom].x;
            self->playerFollowLift->y = newPos.y - self->poses[poseFrom].y;
            self->playerFollowLift->z = newPos.z - self->poses[poseFrom].z;
        }

        /* Check if arrived */
        if (self->liftFrame >= totalTime) {
            self->liftFrame = 0;
            if (self->liftState == MESH_OBJECT_DRIVE) {
                self->liftPoseIndex++;
                if (self->liftPoseIndex >= self->posesCount - 1) {
                    self->liftPoseIndex = self->posesCount - 1;
                    self->liftState = MESH_OBJECT_STAY_END;
                    if (self->liftCycled) {
                        self->liftState = MESH_OBJECT_DRIVE_BACK;
                    }
                    if (self->disactivateOnEnd) {
                        self->state = MESH_OBJECT_STATE_STAY;
                    }
                }
            } else { /* DRIVE_BACK */
                self->liftPoseIndex--;
                if (self->liftPoseIndex <= 0) {
                    self->liftPoseIndex = 0;
                    self->liftState = MESH_OBJECT_STAY_BEGIN;
                    if (self->liftCycled && self->liftReUse) {
                        self->liftState = MESH_OBJECT_DRIVE;
                    }
                    self->state = MESH_OBJECT_STATE_STAY;
                }
            }
        }
    }

    /* Smooth direction interpolation */
    if (self->smoothDirSpeed > 0 && self->morphing) {
        Matrix *transform = Morphing_getTransform(self->morphing);
        if (transform) {
            int curDirX = transform->m20;
            int curDirZ = transform->m22;
            int diffX = self->targetDirX - curDirX;
            int diffZ = self->targetDirZ - curDirZ;
            if (diffX != 0 || diffZ != 0) {
                curDirX += diffX / self->smoothDirSpeed;
                curDirZ += diffZ / self->smoothDirSpeed;
                Matrix_setDir(transform, curDirX, transform->m21, curDirZ);
            }
        }
    }
}

void MeshObject_lookAt(MeshObject *self, int x, int z, int speed) {
    self->targetDirX = x;
    self->targetDirZ = z;
    self->smoothDirSpeed = speed;
}

void MeshObject_lookAtDirect(MeshObject *self, int x, int z) {
    if (!self->morphing) return;
    Matrix *transform = Morphing_getTransform(self->morphing);
    if (transform) {
        Matrix_setDir(transform, x, transform->m21, z);
    }
}

bool MeshObject_sphereCollisionTest(MeshObject *self, Character *body, int radius) {
    if (!self->bbox) return false;
    int bx = Character_getX(body);
    int by = Character_getY(body);
    int bz = Character_getZ(body);
    return BoundingBox_sphereTest(self->bbox, bx, by, bz, radius);
}

int MeshObject_computeHeight(MeshObject *self, int x, int z) {
    if (!self->bbox) return 0;
    return BoundingBox_computeHeight(self->bbox, x, z);
}

bool MeshObject_rayCast(MeshObject *self, Vector3D *origin, Vector3D *dir) {
    if (!self->bbox) return false;
    if (self->ignoreWeaponRayCast) return false;
    return BoundingBox_rayCast(self->bbox, origin, dir);
}

void MeshObject_setPoses(MeshObject *self, Vector3D *poses, int count, int *timeToMove) {
    self->poses = poses;
    self->posesCount = count;
    self->timeToMove = timeToMove;
}

void MeshObject_setAnimType(MeshObject *self, int type) {
    self->animType = type;
}

void MeshObject_setAnimSpeed(MeshObject *self, float speed) {
    self->animSpeed = speed;
}
