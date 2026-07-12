/*
 * renderobject.h — porte fiel de code/Rendering/RenderObject.java
 *
 * Classe abstrata Java. Aqui usamos vtable (render/renderFast/isVisible).
 */
#ifndef QE_RENDERING_RENDEROBJECT_H
#define QE_RENDERING_RENDEROBJECT_H

typedef struct RenderObject RenderObject;
typedef struct Vertex       Vertex;
typedef struct Texture      Texture;
typedef struct DirectX7     DirectX7;

typedef struct RenderObjectVTable {
    void (*render)    (RenderObject *self, DirectX7 *g3d, Texture *texture);
    void (*renderFast)(RenderObject *self, DirectX7 *g3d, Texture *texture);
    int  (*isVisible) (RenderObject *self, int x1, int y1, int x2, int y2);
    void (*destroy)   (RenderObject *self); /* free do subtipo */
} RenderObjectVTable;

struct RenderObject {
    const RenderObjectVTable *vt;
    int sz;
    int nx, ny, nz;
};

#define QE_RENDER_NORMAL_FP    12
#define QE_RENDER_NORMAL_FP_ONE (1 << QE_RENDER_NORMAL_FP)

/* ctors auxiliares — Java: this() e new RenderObject(Vertex,Vertex,Vertex). */
void RenderObject_init          (RenderObject *self, const RenderObjectVTable *vt);
void RenderObject_initWithNormals(RenderObject *self, const RenderObjectVTable *vt,
                                  Vertex *v1, Vertex *v2, Vertex *v3);

void RenderObject_calculateNormals         (RenderObject *self, Vertex *v1, Vertex *v2, Vertex *v3);
void RenderObject_calculateNormalsProjected(RenderObject *self, Vertex *v1, Vertex *v2, Vertex *v3);

/* Dispatchers */
void RenderObject_render    (RenderObject *self, DirectX7 *g3d, Texture *tex);
void RenderObject_renderFast(RenderObject *self, DirectX7 *g3d, Texture *tex);
int  RenderObject_isVisible (RenderObject *self, int x1, int y1, int x2, int y2);

int  RenderObject_getLight   (RenderObject *self, Vertex *a, int la, DirectX7 *g3d);
/* Java retorna int[3]; aqui *out sao 3 ints ja alocados pelo chamador. */
void RenderObject_getLight3  (RenderObject *self, Vertex *a, int la, int la2, int la3, DirectX7 *g3d, int *out);

#endif
