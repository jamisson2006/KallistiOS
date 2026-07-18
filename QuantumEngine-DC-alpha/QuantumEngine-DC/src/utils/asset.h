/*
 * asset.h — porte fiel de code/utils/Asset.java (Quantum Engine J2ME)
 *
 * Cache de recursos por chave string. Preserva a mesma politica de
 * versionamento das chaves ("MESH_", "MESHCLONE_", "ANIMATION_",
 * "MORPHING_", "MESHIMAGE_", "RIMG_", "MIPMAPS_", "SND_") do original.
 *
 * Tipos externos (Mesh, Texture, RawImage, Morphing, MeshImage) sao ponteiros
 * opacos definidos em outros modulos ainda por converter.
 */
#ifndef QE_UTILS_ASSET_H
#define QE_UTILS_ASSET_H

#include "sound.h"
#include <stddef.h>
#include <stdint.h>

/* Forward-decls (definidos em modulos ainda por converter): */
typedef struct Mesh      Mesh;
typedef struct Texture   Texture;
typedef struct RawImage  RawImage;
typedef struct Morphing  Morphing;
typedef struct MeshImage MeshImage;

/* Limpa o cache e chama destroy() em cada entrada apropriada. */
void  Asset_clear(void);

/* Reduz uma RawImage em cache pela metade (fiel a desizeSomething). */
int   Asset_desizeSomething(void);

/* --- getters cacheados --- */

/* Retorna array de meshes (heap array). scale* sao floats. Java retornava
 * Mesh[]. */
Mesh **Asset_getMeshes(const char *file, float sX, float sY, float sZ, int *out_count);

/* Nao cacheado — sempre carrega novo. */
Mesh  *Asset_getMeshCloneDynamic(const char *file, float sX, float sY, float sZ);

/* Cacheia com prefixo MESHCLONE_ (retorna [0]). */
Mesh  *Asset_getMeshClone(const char *file, float sX, float sY, float sZ);

/* No Java retornava short[][]. Aqui retornamos um int16_t** heap + out_rows/cols. */
int16_t **Asset_getAnimation(const char *file, float sX, float sY, float sZ,
                             int *out_rows, int *out_cols);

Morphing  *Asset_getMorphing (const char *file, float sX, float sY, float sZ);
MeshImage *Asset_getMeshImage(const char *file, float sX, float sY, float sZ);
MeshImage *Asset_getMeshImageDynamic(const char *file, float sX, float sY, float sZ);

/* Texture nao vai a cache principal (Java tambem nao) mas puxa mipmaps
 * cacheados de MIPMAPS_. */
Texture   *Asset_getTexture(const char *file);
RawImage  *Asset_getRawImage(const char *file);
Texture   *Asset_getTextureNM(const char *file);

/* Sons: verifica Main.isSounds/sounds ou Main.isFootsteps/footsteps.
 * Retorna NULL quando desabilitado. Caso contrario, cacheia por SND_<file>. */
Sound     *Asset_getSound(const char *file);
Sound     *Asset_getFootsteps(const char *file);

/* applyMeshEffects: quebra "effects" por ',' ou ';' e faz
 * texs[i].setDrawMode((byte)Integer.parseInt(mods[i])). */
void Asset_applyMeshEffects(Mesh *mesh, const char *effects);
void Asset_applyMeshEffectsArr(Mesh **meshes, int count, const char *effects);

#endif
