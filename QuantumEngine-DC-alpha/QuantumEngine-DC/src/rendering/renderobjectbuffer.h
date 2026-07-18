/*
 * renderobjectbuffer.h — porte fiel de code/Rendering/RenderObjectBuffer.java
 * @author DDDENISSS
 */
#ifndef QE_RENDERING_RENDER_OBJECT_BUFFER_H
#define QE_RENDERING_RENDER_OBJECT_BUFFER_H

typedef struct TMPElement    TMPElement;
typedef struct RenderObject  RenderObject;
typedef struct Texture       Texture;
typedef struct MultyTexture  MultyTexture;

typedef struct RenderObjectBuffer {
    TMPElement **buffer; /* array de ponteiros — Java tem TMPElement[] com objs alocados */
    int          buffer_cap;
    int          size;
} RenderObjectBuffer;

RenderObjectBuffer *RenderObjectBuffer_new(void);
void                RenderObjectBuffer_free(RenderObjectBuffer *self);

void RenderObjectBuffer_addRenderObjects        (RenderObjectBuffer *self, RenderObject **objs, int n, Texture       *tex, int x1, int y1, int x2, int y2);
void RenderObjectBuffer_addRenderObjectsNoTex   (RenderObjectBuffer *self, RenderObject **objs, int n,                     int x1, int y1, int x2, int y2);
void RenderObjectBuffer_addRenderObjectsMT      (RenderObjectBuffer *self, RenderObject **objs, int n, MultyTexture  *tex, int x1, int y1, int x2, int y2);

void RenderObjectBuffer_addRenderObjectTMP      (RenderObjectBuffer *self, TMPElement *obj, int x1, int y1, int x2, int y2);
void RenderObjectBuffer_addRenderObject         (RenderObjectBuffer *self, RenderObject *obj, int x1, int y1, int x2, int y2);
void RenderObjectBuffer_addRenderObjectTex      (RenderObjectBuffer *self, RenderObject *obj, Texture      *tex, int x1, int y1, int x2, int y2);
void RenderObjectBuffer_addRenderObjectDT       (RenderObjectBuffer *self, RenderObject *obj, Texture      *tex, int x1, int y1, int x2, int y2);
void RenderObjectBuffer_addRenderObjectMT       (RenderObjectBuffer *self, RenderObject *obj, MultyTexture *tex, int x1, int y1, int x2, int y2);

void RenderObjectBuffer_sort   (RenderObjectBuffer *self, int start, int end);
void RenderObjectBuffer_reset  (RenderObjectBuffer *self);
void RenderObjectBuffer_resetTex(RenderObjectBuffer *self);

TMPElement **RenderObjectBuffer_getBuffer(RenderObjectBuffer *self);
int          RenderObjectBuffer_getSize  (RenderObjectBuffer *self);

#endif
