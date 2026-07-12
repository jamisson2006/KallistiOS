/*
 * light_mapper.c — porte fiel de code/Gameplay/Map/LightMapper.java
 * Dreamcast port: only pre-baked lightmap loading; real-time generation stubbed.
 */
#include "light_mapper.h"
#include "../../rendering/meshes/mesh.h"
#include "../../rendering/renderobject.h"
#include "light.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Static configuration variables                                      */
/* ------------------------------------------------------------------ */
int   LightMapper_aoDistance = 300;
int   LightMapper_aoIntensity = 50;
int   LightMapper_ambientLight[3] = {30, 30, 30};
int   LightMapper_skyLight[3] = {0, 0, 0};
int   LightMapper_sunLight[3] = {0, 0, 0};
int   LightMapper_giRays = 4;
int   LightMapper_giIntensity[3] = {0, 0, 0};
int   LightMapper_giFallOff[3] = {0, 0, 0};
int   LightMapper_lumFromTextures[3] = {0, 0, 0};
int   LightMapper_smoothMax = 3;
bool  LightMapper_cameraVectorLight = false;
bool  LightMapper_bwGI = false;
bool  LightMapper_bwTexGI = false;
bool  LightMapper_allRooms = false;
bool  LightMapper_fastCalc = false;
bool  LightMapper_slCheap = false;
int   LightMapper_raysC = 8;

Light **LightMapper_lights = NULL;
int     LightMapper_lightCount = 0;

/* ------------------------------------------------------------------ */
/* Big-endian read helpers                                             */
/* ------------------------------------------------------------------ */
static int16_t read_be_short(FILE *f) {
    uint8_t buf[2];
    fread(buf, 1, 2, f);
    return (int16_t)((buf[0] << 8) | buf[1]);
}

static int32_t read_be_int(FILE *f) {
    uint8_t buf[4];
    fread(buf, 1, 4, f);
    return (int32_t)((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3]);
}

static uint8_t read_byte(FILE *f) {
    uint8_t b;
    fread(&b, 1, 1, f);
    return b;
}

/* ------------------------------------------------------------------ */
/* Reset                                                               */
/* ------------------------------------------------------------------ */
void LightMapper_reset(void) {
    LightMapper_aoDistance = 300;
    LightMapper_aoIntensity = 50;
    LightMapper_ambientLight[0] = 30;
    LightMapper_ambientLight[1] = 30;
    LightMapper_ambientLight[2] = 30;
    LightMapper_skyLight[0] = 0;
    LightMapper_skyLight[1] = 0;
    LightMapper_skyLight[2] = 0;
    LightMapper_sunLight[0] = 0;
    LightMapper_sunLight[1] = 0;
    LightMapper_sunLight[2] = 0;
    LightMapper_giRays = 4;
    LightMapper_giIntensity[0] = 0;
    LightMapper_giIntensity[1] = 0;
    LightMapper_giIntensity[2] = 0;
    LightMapper_giFallOff[0] = 0;
    LightMapper_giFallOff[1] = 0;
    LightMapper_giFallOff[2] = 0;
    LightMapper_lumFromTextures[0] = 0;
    LightMapper_lumFromTextures[1] = 0;
    LightMapper_lumFromTextures[2] = 0;
    LightMapper_smoothMax = 3;
    LightMapper_cameraVectorLight = false;
    LightMapper_bwGI = false;
    LightMapper_bwTexGI = false;
    LightMapper_allRooms = false;
    LightMapper_fastCalc = false;
    LightMapper_slCheap = false;
    LightMapper_raysC = 8;

    LightMapper_lights = NULL;
    LightMapper_lightCount = 0;
}

/* ------------------------------------------------------------------ */
/* Parameter setters                                                   */
/* ------------------------------------------------------------------ */
void LightMapper_setFastCalc(bool fast) {
    LightMapper_fastCalc = fast;
}

void LightMapper_setRays(int rays) {
    LightMapper_raysC = rays;
}

void LightMapper_setslCheap(bool cheap) {
    LightMapper_slCheap = cheap;
}

void LightMapper_ambientLightSet(int r, int g, int b) {
    LightMapper_ambientLight[0] = r;
    LightMapper_ambientLight[1] = g;
    LightMapper_ambientLight[2] = b;
}

void LightMapper_skyLightIntensitySet(int r, int g, int b) {
    LightMapper_skyLight[0] = r;
    LightMapper_skyLight[1] = g;
    LightMapper_skyLight[2] = b;
}

void LightMapper_sunLightIntensitySet(int r, int g, int b) {
    LightMapper_sunLight[0] = r;
    LightMapper_sunLight[1] = g;
    LightMapper_sunLight[2] = b;
}

void LightMapper_giIntensitySet(int r, int g, int b) {
    LightMapper_giIntensity[0] = r;
    LightMapper_giIntensity[1] = g;
    LightMapper_giIntensity[2] = b;
}

void LightMapper_giFallOffSet(int r, int g, int b) {
    LightMapper_giFallOff[0] = r;
    LightMapper_giFallOff[1] = g;
    LightMapper_giFallOff[2] = b;
}

void LightMapper_lumFromTexturesSet(int r, int g, int b) {
    LightMapper_lumFromTextures[0] = r;
    LightMapper_lumFromTextures[1] = g;
    LightMapper_lumFromTextures[2] = b;
}

/* ------------------------------------------------------------------ */
/* loadLightMap — reads pre-baked polygon light values from file       */
/* Format: for each mesh, for each polygon, read 1 byte (light value) */
/* Java original reads bytes via DataInputStream                       */
/* ------------------------------------------------------------------ */
void LightMapper_loadLightMap(Mesh **meshes, int count, const char *file) {
    if (!file || !meshes) return;

    char path[256];
    snprintf(path, sizeof(path), "/rd/%s", file);

    FILE *f = fopen(path, "rb");
    if (!f) return;

    for (int i = 0; i < count; i++) {
        if (!meshes[i]) continue;
        int polyCount = 0;
        RenderObject **polys = Mesh_polygons(meshes[i], &polyCount);

        for (int p = 0; p < polyCount; p++) {
            uint8_t lightVal = read_byte(f);
            if (polys[p]) {
                /* Store light value in the sz field of RenderObject
                 * (Java original stores per-polygon light this way) */
                polys[p]->sz = (int)lightVal;
            }
        }
    }

    fclose(f);
}

/* ------------------------------------------------------------------ */
/* saveLightMap — stub on Dreamcast                                    */
/* ------------------------------------------------------------------ */
void LightMapper_saveLightMap(Mesh **meshes, int count, const char *file) {
    (void)meshes;
    (void)count;
    (void)file;
    /* Not needed on Dreamcast — lightmaps are pre-baked on PC */
}

/* ------------------------------------------------------------------ */
/* generateLightMap — stub: loads from file if available               */
/* ------------------------------------------------------------------ */
void LightMapper_generateLightMap(Mesh **meshes, int count, const char *file) {
    /* On Dreamcast, real-time lightmap generation is too slow.
     * Just load the pre-baked data if the file exists. */
    LightMapper_loadLightMap(meshes, count, file);
}
