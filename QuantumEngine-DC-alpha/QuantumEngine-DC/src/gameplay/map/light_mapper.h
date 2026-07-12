/*
 * light_mapper.h — porte fiel de code/Gameplay/Map/LightMapper.java
 * Dreamcast port: only pre-baked lightmap loading; generation is stubbed.
 */
#ifndef QE_GAMEPLAY_MAP_LIGHT_MAPPER_H
#define QE_GAMEPLAY_MAP_LIGHT_MAPPER_H

#include <stdbool.h>
#include <stdint.h>

typedef struct Mesh  Mesh;
typedef struct Light Light;

/* Static configuration variables */
extern int   LightMapper_aoDistance;
extern int   LightMapper_aoIntensity;
extern int   LightMapper_ambientLight[3];
extern int   LightMapper_skyLight[3];
extern int   LightMapper_sunLight[3];
extern int   LightMapper_giRays;
extern int   LightMapper_giIntensity[3];
extern int   LightMapper_giFallOff[3];
extern int   LightMapper_lumFromTextures[3];
extern int   LightMapper_smoothMax;
extern bool  LightMapper_cameraVectorLight;
extern bool  LightMapper_bwGI;
extern bool  LightMapper_bwTexGI;
extern bool  LightMapper_allRooms;
extern bool  LightMapper_fastCalc;
extern bool  LightMapper_slCheap;
extern int   LightMapper_raysC;

/* Lights array */
extern Light **LightMapper_lights;
extern int     LightMapper_lightCount;

/* Reset all parameters to defaults */
void LightMapper_reset(void);

/* Parameter setters */
void LightMapper_setFastCalc(bool fast);
void LightMapper_setRays(int rays);
void LightMapper_setslCheap(bool cheap);
void LightMapper_ambientLightSet(int r, int g, int b);
void LightMapper_skyLightIntensitySet(int r, int g, int b);
void LightMapper_sunLightIntensitySet(int r, int g, int b);
void LightMapper_giIntensitySet(int r, int g, int b);
void LightMapper_giFallOffSet(int r, int g, int b);
void LightMapper_lumFromTexturesSet(int r, int g, int b);

/* Load pre-baked lightmap from file */
void LightMapper_loadLightMap(Mesh **meshes, int count, const char *file);

/* Save lightmap — stub on Dreamcast */
void LightMapper_saveLightMap(Mesh **meshes, int count, const char *file);

/* Generate lightmap — stub: calls loadLightMap if file exists */
void LightMapper_generateLightMap(Mesh **meshes, int count, const char *file);

#endif
