/*
 * camera.h — porte fiel de code/Rendering/Camera.java
 * @author DDDENISSS
 */
#ifndef QE_RENDERING_CAMERA_H
#define QE_RENDERING_CAMERA_H

typedef struct Matrix Matrix;
typedef struct House  House;

typedef struct Camera {
    Matrix *tmpPos, *tmp, *tmp2;
    Matrix *camera;

    int x, y, z;
    int rotX, rotY;
    float currentRotX, currentRotY;
    int   smoothSteps;

    int oldPart, part;
    int fset;
} Camera;

Camera *Camera_new(void);
void    Camera_free(Camera *self);

void    Camera_set(Camera *self, const Matrix *pos, float newRotX, float newRotY,
                   int absolutePos, int absoluteRot);

void    Camera_addAngle(Camera *self, int i);
void    Camera_addY    (Camera *self, int i);
void    Camera_addZ    (Camera *self, int i);
void    Camera_addX    (Camera *self, int i);

Matrix *Camera_getCamera(Camera *self);
void    Camera_calcPart (Camera *self, House *home);
int     Camera_getPart  (const Camera *self);
void    Camera_setPart  (Camera *self, int part);

#endif
