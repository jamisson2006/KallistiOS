/*
 * bot.c — porte fiel de code/AI/Bot.java
 */
#include "bot.h"
#include "misc/blood.h"
#include "misc/trace.h"
#include "../collision/ray.h"
#include "../math/matrix.h"
#include "../rendering/vertex.h"
#include "../rendering/renderobject.h"
#include "../rendering/meshes/polygon4v.h"
#include <stdlib.h>
#include <math.h>

typedef struct Scene        Scene;
typedef struct House        House;
typedef struct Portal       Portal;
typedef struct DirectX7     DirectX7;
typedef struct Mesh         Mesh;

extern int      QFPS_frameTime;
extern bool     Main_blood;

extern Scene   *Scene_getHouse_asScene(Scene *s); /* placeholder */
extern House   *Scene_getHouse(Scene *scene);
extern void    *House_getObjects(House *house);
extern int      House_getObjectsCount(House *house);
extern void     House_rayCast(House *house, int part, Ray *ray, bool recalc);
extern void     House_addObject(House *house, void *obj);
extern bool     House_isNear(House *house, int p1, int p2);

extern void    *Room_getPortals(void *room, int *out_n);
extern int      Room_getId(void *room);
extern void   **House_getRooms(House *house);

extern Vertex **Portal_getVertices(Portal *portal, int *out_n);
extern int      Portal_getRoom(Portal *portal);
extern int      Portal_getRoomId(Portal *portal);

extern int64_t  qe_current_ms(void);

extern int      MathUtils_fixDegree(int d);
extern int      MathUtils_getAnglez(int x1, int z1, int x2, int z2);

static inline bool RenderObject_isPolygon4V(RenderObject *ro) {
    return ro->vt == Polygon4V_vt();
}
static inline Polygon4V *Polygon4V_cast(RenderObject *ro) {
    return (Polygon4V *)ro;
}

bool Bot_raycastTargetVisibility = false;
static Ray s_ray;
static Vector3D s_tmpVec;
static Vector3D s_side;
static Vector3D s_up;
static Vector3D s_dir;

void Bot_init(Bot *self) {
    self->blood = Blood_new((GameObject *)self);
    self->bloodSpeed.x = 0; self->bloodSpeed.y = 0; self->bloodSpeed.z = 0;
    self->bloodPos.x = 0; self->bloodPos.y = 0; self->bloodPos.z = 0;
    self->bloodwall = NULL;
    self->fraction = 0;
    self->deathFall = 1;
    self->visiblityCheck = false;
    self->hasBlood = true;
}

void Bot_set(Bot *self, Vector3D *pos) {
    Character_reset(self->base.character);
    Matrix *t = Character_getTransform(self->base.character);
    Matrix_setPosition(t, pos->x, pos->y, pos->z);
    self->base.character->part = -1;
    Blood_reset(self->blood);
}

void Bot_destroy(Bot *self) {
    Blood_destroy(self->blood);
    self->bloodwall = NULL;
    self->blood = NULL;
}

void Bot_renderBlood(Bot *self, DirectX7 *g3d, int sz) {
    if (Blood_isBleeding(self->blood)) {
        Blood_render(self->blood, g3d, 3250, &self->bloodPos);
        self->bloodPos.x += -self->bloodSpeed.x * QFPS_frameTime / 50;
        self->bloodPos.y += -self->bloodSpeed.y * QFPS_frameTime / 50;
        self->bloodPos.z += -self->bloodSpeed.z * QFPS_frameTime / 50;
    }
}

bool Bot_damage(Bot *self, GameObject *obj, int dmg) {
    if (dmg > 0) {
        Matrix *mat = Character_getTransform(self->base.character);

        if (self->hasBlood) {
            self->bloodPos.x = mat->m03;
            self->bloodPos.y = mat->m13 + Character_getHeight(self->base.character);
            self->bloodPos.z = mat->m23;

            if (obj != NULL) {
                Matrix *mat2 = Character_getTransform(obj->character);
                s_tmpVec.x = mat->m03 - mat2->m03;
                s_tmpVec.y = mat->m13 - mat2->m13;
                s_tmpVec.z = mat->m23 - mat2->m23;

                Vector3D_setLength2(&s_tmpVec, dmg * Character_getRadius(self->base.character) / 400);
                Vector3D *spd = Character_getSpeed(self->base.character);
                spd->x = s_tmpVec.x; spd->y = s_tmpVec.y; spd->z = s_tmpVec.z;

                Vector3D_setLength2(&s_tmpVec, 60);
                self->bloodSpeed.x = s_tmpVec.x;
                self->bloodSpeed.y = s_tmpVec.y;
                self->bloodSpeed.z = s_tmpVec.z;
            }

            Blood_bleed(self->blood);
        }
    }
    GameObject_damage(&self->base, dmg);
    return true;
}

void Bot_update(Bot *self, Scene *scene, Player *player) {
    /* action or drop depending on alive state - called by subclass */
    (void)scene; (void)player;
}

void Bot_spawnBlood(Bot *self, Scene *scene) {
    Ray_reset(&s_ray);
    Matrix *mat = Character_getTransform(self->base.character);
    s_ray.start.x = mat->m03;
    s_ray.start.y = mat->m13 + 1;
    s_ray.start.z = mat->m23;
    s_ray.dir.x = 0;
    s_ray.dir.y = -10;
    s_ray.dir.z = 0;
    Ray_reset(&s_ray);
    House *house = Scene_getHouse(scene);
    House_rayCast(house, GameObject_getPart(&self->base), &s_ray, false);

    if (Ray_isCollision(&s_ray)) {
        self->bloodwall = Bot_createTrace(self, Ray_getCollisionPoint(&s_ray), Ray_getTriangle(&s_ray));
        self->bloodwall->base.character->part = Ray_getNumRoom(&s_ray);
        House_addObject(house, self->bloodwall);
    }
}

void Bot_drop(Bot *self, Scene *scene) {
    (void)scene;
    Matrix *t = Character_getTransform(self->base.character);
    if (t->m11 > 0) {
        Character_drop(self->base.character, -8);
    }
}

void Bot_dropSide(Bot *self, Scene *scene) {
    (void)scene;
    Matrix *t = Character_getTransform(self->base.character);
    if (t->m11 > 0) {
        Character_dropSide(self->base.character, -8);
    }
}

bool Bot_isTargetVisibleRaycast(Bot *self, House *house, GameObject *target) {
    if (!Bot_raycastTargetVisibility) return true;

    Matrix *objMat = Character_getTransform(target->character);
    Matrix *mat = Character_getTransform(self->base.character);

    Ray_reset(&s_ray);
    s_ray.start.x = mat->m03;
    s_ray.start.y = mat->m13 + Character_getHeight(self->base.character);
    s_ray.start.z = mat->m23;
    s_ray.dir.x = objMat->m03 - mat->m03;
    s_ray.dir.y = objMat->m13 + Character_getHeight(target->character) - mat->m13;
    s_ray.dir.z = objMat->m23 - mat->m23;

    House_rayCast(house, GameObject_getPart(&self->base), &s_ray, false);
    return !Ray_isCollision(&s_ray);
}

void Bot_lookAt(Bot *self, int x, int z) {
    Matrix *pos = Character_getTransform(self->base.character);

    s_dir.x = pos->m03 - x;
    s_dir.y = 0;
    s_dir.z = pos->m23 - z;
    Vector3D_setLength(&s_dir, -16384); /* -Matrix.FP */

    if (s_dir.x == 0 && s_dir.y == 0) return;

    /* setDir */
    Vector3D dir2;
    dir2.x = s_dir.x; dir2.y = 0; dir2.z = s_dir.z;
    Vector3D_setLength(&dir2, 16384);

    if (abs(dir2.x - pos->m02) < 20 && abs(dir2.y - pos->m12) < 20 && abs(dir2.z - pos->m22) < 20) return;

    s_up.x = 0; s_up.y = 16384; s_up.z = 0;
    Vector3D_crossFP(&s_side, &s_up, &dir2, 14);
    Vector3D_setLength(&s_side, 16384);

    if (Vector3D_lengthSquared(&dir2) != 0 && Vector3D_lengthSquared(&s_side) != 0) {
        pos->m02 = dir2.x; pos->m12 = dir2.y; pos->m22 = dir2.z;
        pos->m00 = s_side.x; pos->m10 = s_side.y; pos->m20 = s_side.z;
        pos->m01 = s_up.x; pos->m11 = s_up.y; pos->m21 = s_up.z;
    }
}

bool Bot_isTargetInFOV(GameObject *observer, GameObject *observable) {
    Vector3D lookDir;
    lookDir.x = observer->character->transform.m02;
    lookDir.y = 0;
    lookDir.z = observer->character->transform.m22;
    Vector3D_setLength(&lookDir, 4096);

    Vector3D obsDir;
    obsDir.x = observable->character->transform.m03 - observer->character->transform.m03;
    obsDir.y = 0;
    obsDir.z = observable->character->transform.m23 - observer->character->transform.m23;
    Vector3D_setLength(&obsDir, 4096);

    bool result = Vector3D_dot(&lookDir, &obsDir) > 0;
    result |= Character_distance(observer->character, observable->character) < (int64_t)2500 * 2500;
    return result;
}

Portal *Bot_commonPortal(House *house, int part1, int part2) {
    void **rooms = House_getRooms(house);
    int portal_n = 0;
    Portal **portals = (Portal **)Room_getPortals(rooms[part1], &portal_n);
    if (portals == NULL) return NULL;

    for (int i = 0; i < portal_n; i++) {
        Portal *portal = portals[i];
        if (Portal_getRoomId(portal) == part2) return portal;
    }
    return NULL;
}

void Bot_computeCentre(Portal *portal, Vector3D *center) {
    int vers_n = 0;
    Vertex **vers = Portal_getVertices(portal, &vers_n);
    int sx = 0, sy = 0, sz = 0;
    for (int i = 0; i < vers_n; i++) {
        sx += vers[i]->x;
        sy += vers[i]->y;
        sz += vers[i]->z;
    }
    if (vers_n > 0) {
        sx /= vers_n;
        sy /= vers_n;
        sz /= vers_n;
    }
    center->x = sx;
    center->y = sy;
    center->z = sz;
}

bool Bot_contains(int *list, int list_n, int need) {
    if (list == NULL) return true;
    for (int i = 0; i < list_n; i++) {
        if (list[i] == need) return true;
    }
    return false;
}

int64_t Bot_sqr(int x) {
    return (int64_t)x * (int64_t)x;
}

GameObject *Bot_findBot(Bot *self, void **objs, int objs_n, Bot *ignore, int *fractions, int fractions_n) {
    GameObject *nearest = NULL;
    int64_t minDist = 0x7FFFFFFFFFFFFFFFLL;

    for (int i = 0; i < objs_n; i++) {
        GameObject *obj = (GameObject *)objs[i];
        if (obj == (GameObject *)ignore) continue;
        if (obj == NULL) continue;
        if (GameObject_isDead(obj)) continue;

        Bot *bot = (Bot *)obj;
        if (!Bot_contains(fractions, fractions_n, bot->fraction)) continue;
        if (!Bot_isTargetInFOV(&self->base, obj)) continue;

        int64_t dist = Character_distance(self->base.character, obj->character);
        if (dist < minDist) {
            minDist = dist;
            nearest = obj;
        }
    }
    return nearest;
}

void Bot_increaseMeshSz(Mesh *mesh, int z) {
    extern RenderObject **Mesh_getPolygons(Mesh *m);
    extern int Mesh_getPolygonsCount(Mesh *m);
    RenderObject **objs = Mesh_getPolygons(mesh);
    int n = Mesh_getPolygonsCount(mesh);
    for (int i = 0; i < n; i++) {
        objs[i]->sz += z;
    }
}

Trace *Bot_createTrace(Bot *self, Vector3D *vector3f, RenderObject *meshr) {
    int minx = 0, miny = 0, minz = 0;
    int maxx = 0, maxy = 0, maxz = 0;
    int posx = 0, posy = 0, posz = 0;
    Vector3D v1 = {0, 0, 0}, v2 = {0, 0, 0}, v3 = {0, 0, 0}, v4 = {0, 0, 0};

    if (RenderObject_isPolygon4V(meshr)) {
        Polygon4V *p4v = Polygon4V_cast(meshr);
        Vertex *a = p4v->a;
        Vertex *b = p4v->b;
        Vertex *c = p4v->c;
        Vertex *d = p4v->d;

        posx = (a->x + b->x + c->x + d->x) / 4;
        posy = (a->y + b->y + c->y + d->y) / 4;
        posz = (a->z + b->z + c->z + d->z) / 4;

        int min2(int a, int b) { return a < b ? a : b; }
        int max2(int a, int b) { return a > b ? a : b; }

        minx = min2(min2(min2(a->x, b->x), c->x), d->x) - posx;
        miny = min2(min2(min2(a->y, b->y), c->y), d->y) - posy;
        minz = min2(min2(min2(a->z, b->z), c->z), d->z) - posz;
        maxx = max2(max2(max2(a->x, b->x), c->x), d->x) - posx;
        maxy = max2(max2(max2(a->y, b->y), c->y), d->y) - posy;
        maxz = max2(max2(max2(a->z, b->z), c->z), d->z) - posz;

        v1.x = a->x - posx; v1.y = a->y - posy; v1.z = a->z - posz;
        v2.x = b->x - posx; v2.y = b->y - posy; v2.z = b->z - posz;
        v3.x = c->x - posx; v3.y = c->y - posy; v3.z = c->z - posz;
        v4.x = d->x - posx; v4.y = d->y - posy; v4.z = d->z - posz;
    }

    Vector3D size;
    size.x = abs(minx) + abs(maxx);
    size.y = abs(miny) + abs(maxy);
    size.z = abs(minz) + abs(maxz);
    return Trace_new(vector3f->x, vector3f->y, vector3f->z, &v1, &v2, &v3, &v4, &size);
}
