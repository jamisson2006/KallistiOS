/*
 * inifile.h — porte fiel de code/utils/IniFile.java (Quantum Engine J2ME)
 */
#ifndef QE_UTILS_INIFILE_H
#define QE_UTILS_INIFILE_H

#include <stdint.h>
#include <stdio.h>

/* No Java, IniFile encapsula um Hashtable cujos valores sao ou String
 * (chave->valor simples) ou outro Hashtable (grupo [SECTION]).
 * Reproduzimos com um pequeno hashtable de arrays (linear scan), suficiente
 * para arquivos ini de setting.txt/weapons.txt (poucas centenas de entradas).
 */
typedef enum {
    QE_INI_STRING = 0,
    QE_INI_TABLE  = 1
} qe_ini_kind;

struct IniFile;

typedef struct qe_ini_entry {
    char *key;
    qe_ini_kind kind;
    union {
        char *s;                 /* QE_INI_STRING: string heap */
        struct IniFile *t;       /* QE_INI_TABLE:  sub-inifile heap */
    } v;
} qe_ini_entry;

typedef struct IniFile {
    qe_ini_entry *entries;
    int           count;
    int           capacity;
    int           owns_entries; /* 1 = destroy libera as entries; 0 = view */
} IniFile;

/* Construtores */
IniFile *IniFile_new(void);                                    /* vazio */
IniFile *IniFile_new_fromStr(const char *str, int useGroups);  /* Java: IniFile(String,boolean) */
IniFile *IniFile_new_fromLines(char **lines, int nlines, int useGroups);

IniFile *IniFile_createFromResource(const char *file);         /* useGroups=false */
IniFile *IniFile_createFromResourceKeys(const char *file, int useGroups);

/* Retorna {char** names, IniFile** groups, int count} (todos heap). */
void IniFile_createGroups(const char *file,
                          char ***out_names, IniFile ***out_groups, int *out_count);

void IniFile_free(IniFile *self);
void IniFile_freeGroupsResult(char **names, IniFile **groups, int count);

/* Parsing */
void IniFile_set(IniFile *self, char **lines, int nlines, int useGroups);

/* save (arquivo binario/texto). Java: recursivo por Hashtable. */
void IniFile_save(IniFile *self, FILE *stream);

/* Keys top-level */
char **IniFile_keys(IniFile *self, int *out_count);
/* subtabelas top-level */
IniFile **IniFile_hashtables(IniFile *self, int *out_count);
int  IniFile_groupExists(IniFile *self, const char *group);

/* put */
void IniFile_put(IniFile *self, const char *key, const char *value);
void IniFile_putGroup(IniFile *self, const char *group, const char *key, const char *value);

/* get: em grupo */
const char *IniFile_get_g(IniFile *self, const char *group, const char *key);
const char *IniFile_getDef_g(IniFile *self, const char *group, const char *key, const char *def);
int8_t      IniFile_getByte_g(IniFile *self, const char *group, const char *key);
float       IniFile_getFloat_g(IniFile *self, const char *group, const char *key);
float       IniFile_getFloatDef_g(IniFile *self, const char *group, const char *key, float def);
int         IniFile_getInt_g(IniFile *self, const char *group, const char *key);
int         IniFile_getIntDef_g(IniFile *self, const char *group, const char *key, int def);
int64_t     IniFile_getLong_g(IniFile *self, const char *group, const char *key);

/* get: chave direto (top-level) */
const char *IniFile_get(IniFile *self, const char *key);
const char *IniFile_getDef(IniFile *self, const char *key, const char *def);
int8_t      IniFile_getByte(IniFile *self, const char *key);
float       IniFile_getFloat(IniFile *self, const char *key);
float       IniFile_getFloatDef(IniFile *self, const char *key, float def);
int         IniFile_getInt(IniFile *self, const char *key);
int         IniFile_getIntDef(IniFile *self, const char *key, int def);
int64_t     IniFile_getLong(IniFile *self, const char *key);

#endif
