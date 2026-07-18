/*
 * gameini.h — porte fiel de code/utils/GameIni.java (Quantum Engine J2ME)
 * @author Roman Lahin
 *
 * No Java, GameIni extends IniFile — mesma tabela, mas com traducao i18n
 * (via Main.gameText) nos get()/getDef().
 * Em C: usamos IniFile* como base (composicao) e expomos funcoes GameIni_*
 * que delegam para IniFile_* aplicando a traducao quando ha Main_getGameText.
 */
#ifndef QE_UTILS_GAMEINI_H
#define QE_UTILS_GAMEINI_H

#include "inifile.h"

/* GameIni eh apenas IniFile "marcado" pelo alias tipo. */
typedef IniFile GameIni;

/* Construtores (equivalentes aos do Java) */
GameIni *GameIni_createFromResource(const char *file);              /* useGroups=false */
GameIni *GameIni_createFromResourceKeys(const char *file, int useGroups);

/* createGroups: le arquivo, quebra em [SECTION]s, retorna arrays paralelos. */
void GameIni_createGroups(const char *file,
                          char ***out_names, GameIni ***out_groups, int *out_count);

/* Wrapper de string cut com dois divisores possiveis (str procura por d2;
 * se achar, usa d2, senao usa d). */
char **GameIni_cutOnStrings(const char *str, char d, char d2, int *out_count);
int   *GameIni_cutOnInts(const char *str, char d, char d2, int *out_count);

/* createPos: cutOnStrings depois strip ';' das bordas e parseInt. */
int   *GameIni_createPos(const char *str, char d, int *out_count);

/* get raw sem tentativa de i18n */
const char *GameIni_getNoLang(GameIni *self, const char *key);
const char *GameIni_getNoLangDef(GameIni *self, const char *key, const char *def);
int         GameIni_getIntNoLangDef(GameIni *self, const char *key, int def);

/* Com i18n: se ha traducao em Main_getGameText, aplica; senao devolve original. */
const char *GameIni_get(GameIni *self, const char *key);
const char *GameIni_get_g(GameIni *self, const char *group, const char *key);
const char *GameIni_getDef(GameIni *self, const char *key, const char *def);
const char *GameIni_getDef_g(GameIni *self, const char *group, const char *key, const char *def);

/* Retorna -1 se nao houver prefixo; senao indice do prefixo em list */
int GameIni_startsWith(const char *str, char **list, int list_count);

#endif
