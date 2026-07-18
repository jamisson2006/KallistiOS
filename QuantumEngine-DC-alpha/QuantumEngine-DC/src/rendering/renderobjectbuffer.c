/*
 * renderobjectbuffer.c — porte fiel de code/Rendering/RenderObjectBuffer.java
 */
#include "renderobjectbuffer.h"
#include "tmpelement.h"
#include "renderobject.h"

#include <stdlib.h>
#include <string.h>

/* Polygon3V / Polygon4V — para tex-index em addRenderObject com MultyTexture. */
typedef struct Polygon3V { RenderObject base; int tex; } Polygon3V;
typedef struct Polygon4V { RenderObject base; int tex; } Polygon4V;
extern const void *Polygon3V_vt(void); /* devolve ponteiro para vtable Polygon3V */
extern const void *Polygon4V_vt(void);

/* MultyTexture */
typedef struct MultyTexture MultyTexture;
extern Texture *MultyTexture_texture(MultyTexture *self, int idx);

RenderObjectBuffer *RenderObjectBuffer_new(void) {
    RenderObjectBuffer *b = (RenderObjectBuffer*) calloc(1, sizeof(RenderObjectBuffer));
    return b;
}
void RenderObjectBuffer_free(RenderObjectBuffer *b) {
    if (!b) return;
    for (int i = 0; i < b->buffer_cap; i++) TMPElement_free(b->buffer[i]);
    free(b->buffer); free(b);
}

static void increaseBuffer(RenderObjectBuffer *b) {
    const int increase = 50;
    int newCap = b->buffer_cap + increase;
    b->buffer = (TMPElement**) realloc(b->buffer, sizeof(TMPElement*) * newCap);
    for (int i = b->buffer_cap; i < newCap; i++) b->buffer[i] = TMPElement_new();
    b->buffer_cap = newCap;
}

static Texture *tex_for_multi(RenderObject *obj, MultyTexture *tex) {
    /* Java verifica instanceof Polygon4V / Polygon3V. Aqui usamos vtable. */
    Texture *t = MultyTexture_texture(tex, 0);
    if (obj->vt == Polygon4V_vt()) t = MultyTexture_texture(tex, ((Polygon4V*)obj)->tex);
    if (obj->vt == Polygon3V_vt()) t = MultyTexture_texture(tex, ((Polygon3V*)obj)->tex);
    return t;
}

void RenderObjectBuffer_addRenderObjects(RenderObjectBuffer *self, RenderObject **objs, int n, Texture *tex,
                                         int x1, int y1, int x2, int y2) {
    for (int i = 0; i < n; i++) {
        RenderObject *obj = objs[i];
        if (RenderObject_isVisible(obj, x1, y1, x2, y2)) {
            if (self->size >= self->buffer_cap) increaseBuffer(self);
            TMPElement *el = self->buffer[self->size];
            el->obj = obj; el->tex = tex;
            self->size++;
        }
    }
}
void RenderObjectBuffer_addRenderObjectsNoTex(RenderObjectBuffer *self, RenderObject **objs, int n,
                                              int x1, int y1, int x2, int y2) {
    for (int i = 0; i < n; i++) {
        RenderObject *obj = objs[i];
        if (RenderObject_isVisible(obj, x1, y1, x2, y2)) {
            if (self->size >= self->buffer_cap) increaseBuffer(self);
            TMPElement *el = self->buffer[self->size];
            el->obj = obj; el->tex = NULL;
            self->size++;
        }
    }
}
void RenderObjectBuffer_addRenderObjectsMT(RenderObjectBuffer *self, RenderObject **objs, int n, MultyTexture *tex,
                                            int x1, int y1, int x2, int y2) {
    for (int i = 0; i < n; i++) {
        RenderObject *obj = objs[i];
        if (RenderObject_isVisible(obj, x1, y1, x2, y2)) {
            if (self->size >= self->buffer_cap) increaseBuffer(self);
            TMPElement *el = self->buffer[self->size];
            el->obj = obj;
            el->tex = tex_for_multi(obj, tex);
            self->size++;
        }
    }
}

void RenderObjectBuffer_addRenderObjectTMP(RenderObjectBuffer *self, TMPElement *obj, int x1, int y1, int x2, int y2) {
    if (RenderObject_isVisible(obj->obj, x1, y1, x2, y2)) {
        if (self->size >= self->buffer_cap) increaseBuffer(self);
        TMPElement *el = self->buffer[self->size];
        el->obj = obj->obj; el->tex = obj->tex;
        self->size++;
    }
}
void RenderObjectBuffer_addRenderObject(RenderObjectBuffer *self, RenderObject *obj, int x1, int y1, int x2, int y2) {
    if (RenderObject_isVisible(obj, x1, y1, x2, y2)) {
        if (self->size >= self->buffer_cap) increaseBuffer(self);
        TMPElement *el = self->buffer[self->size];
        el->obj = obj; el->tex = NULL;
        self->size++;
    }
}
void RenderObjectBuffer_addRenderObjectTex(RenderObjectBuffer *self, RenderObject *obj, Texture *tex,
                                            int x1, int y1, int x2, int y2) {
    if (RenderObject_isVisible(obj, x1, y1, x2, y2)) {
        if (self->size >= self->buffer_cap) increaseBuffer(self);
        TMPElement *el = self->buffer[self->size];
        el->obj = obj; el->tex = tex;
        self->size++;
    }
}
void RenderObjectBuffer_addRenderObjectDT(RenderObjectBuffer *self, RenderObject *obj, Texture *tex,
                                          int x1, int y1, int x2, int y2) {
    (void) x1; (void) y1; (void) x2; (void) y2;
    if (self->size >= self->buffer_cap) increaseBuffer(self);
    TMPElement *el = self->buffer[self->size];
    el->obj = obj; el->tex = tex;
    self->size++;
}
void RenderObjectBuffer_addRenderObjectMT(RenderObjectBuffer *self, RenderObject *obj, MultyTexture *tex,
                                          int x1, int y1, int x2, int y2) {
    if (RenderObject_isVisible(obj, x1, y1, x2, y2)) {
        if (self->size >= self->buffer_cap) increaseBuffer(self);
        TMPElement *el = self->buffer[self->size];
        el->obj = obj;
        el->tex = tex_for_multi(obj, tex);
        self->size++;
    }
}

/* Quicksort (fiel ao Java: pivo no meio, sort ordena por sz decrescente). */
void RenderObjectBuffer_sort(RenderObjectBuffer *self, int start, int end) {
    if (start >= end) return;
    int opora = self->buffer[(start + end) >> 1]->obj->sz;
    int first = start, second = end;
    while (first <= second) {
        while (self->buffer[first]->obj->sz > opora) first++;
        while (self->buffer[second]->obj->sz < opora) second--;
        if (first <= second) {
            TMPElement *tmp = self->buffer[first];
            self->buffer[first] = self->buffer[second];
            self->buffer[second] = tmp;
            first++; second--;
        }
    }
    if (start < second) RenderObjectBuffer_sort(self, start, second);
    if (end > first)    RenderObjectBuffer_sort(self, first, end);
}

void RenderObjectBuffer_reset(RenderObjectBuffer *self) { self->size = 0; }
void RenderObjectBuffer_resetTex(RenderObjectBuffer *self) {
    for (int i = 0; i < self->buffer_cap; i++) self->buffer[i]->tex = NULL;
}

TMPElement **RenderObjectBuffer_getBuffer(RenderObjectBuffer *self) { return self->buffer; }
int          RenderObjectBuffer_getSize  (RenderObjectBuffer *self) { return self->size; }
