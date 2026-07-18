/*
 * stringtools.h — porte fiel de code/utils/StringTools.java (Quantum Engine J2ME)
 */
#ifndef QE_UTILS_STRINGTOOLS_H
#define QE_UTILS_STRINGTOOLS_H

#include <stdint.h>
#include <stddef.h>

/*
 * As funcoes Java retornam String / String[] / int[] / float[]. Em C
 * usamos malloc/free — o chamador libera. Os "cutOn*" retornam um array
 * heap alocado com sua contagem via *out_count.
 */

/* Ler recurso texto (arquivo do romdisk /rd/...). Converte 192..255 -> +848
 * (windows-1251 -> UTF-8), pula '\r'. Retorna string heap-alocada '\0'-terminada,
 * ou NULL em erro. */
char *StringTools_getStringFromResource(const char *file);

/* Corta em substrings pelo char 'd'. Ignora tokens vazios (identico ao Java).
 * Retorna vetor de char* heap-alocado. Cada string tambem eh heap. Libera
 * com StringTools_freeStrings. */
char **StringTools_cutOnStrings(const char *str, char d, int *out_count);
void   StringTools_freeStrings(char **arr, int count);

/* cutOnInts / cutOnFloats */
int   *StringTools_cutOnInts(const char *str, char d, int *out_count);
float *StringTools_cutOnFloats(const char *str, char d, int *out_count);

/* Parses fieis ao Integer.parseInt/etc, trimando bordas */
float   StringTools_parseFloat(const char *val);
int     StringTools_parseInt(const char *val);
int     StringTools_cleverParseInt(const char *val); /* corta em '.' antes de parsear */
int8_t  StringTools_parseByte(const char *val);
int64_t StringTools_parseLong(const char *val);

int     StringTools_isNumeric(const char *s);

/* Retorna nova string heap com nao-numericos removidos das bordas. */
char   *StringTools_deleteNonNumeric(const char *s);

/* rgb como "r<d>g<d>b" -> inteiro 0xRRGGBB */
int     StringTools_getRGB(const char *rgbs, char div);

/* "#RRGGBB" ou "#AARRGGBB" -> inteiro */
int     StringTools_parseXMLColor(const char *argb);

#endif
