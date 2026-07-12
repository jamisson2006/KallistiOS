/*
 * tmpelement.h — porte fiel de code/Rendering/TMPElement.java
 */
#ifndef QE_RENDERING_TMPELEMENT_H
#define QE_RENDERING_TMPELEMENT_H

typedef struct Texture      Texture;
typedef struct RenderObject RenderObject;

typedef struct TMPElement {
    Texture      *tex;
    RenderObject *obj;
} TMPElement;

TMPElement *TMPElement_new(void);
void        TMPElement_free(TMPElement *self);

#endif
