/*
 * multytexture.h — porte fiel de code/Rendering/MultyTexture.java
 */
#ifndef QE_RENDERING_MULTYTEXTURE_H
#define QE_RENDERING_MULTYTEXTURE_H

typedef struct Texture Texture;

typedef struct MultyTexture {
    Texture **textures;
    int       textures_n;
} MultyTexture;

MultyTexture *MultyTexture_new_empty(void);
MultyTexture *MultyTexture_new_files(const char *files, int perspectiveCorrect);
MultyTexture *MultyTexture_new_list(char **texList, int list_n);
MultyTexture *MultyTexture_new_single(Texture *tex);
MultyTexture *MultyTexture_new_size(int n);

void MultyTexture_free(MultyTexture *self);
void MultyTexture_updateAnimation(MultyTexture *self);

/* Retorno seguro (clamp para idx dentro do array) — usado por RenderObjectBuffer. */
Texture *MultyTexture_texture(MultyTexture *self, int idx);

#endif
