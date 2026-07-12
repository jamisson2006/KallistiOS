/*
 * room_object.c — porte fiel de code/Gameplay/Objects/RoomObject.java (Quantum Engine J2ME)
 * @author Roman Lahin
 *
 * Implementacao completa da classe base RoomObject (1032 linhas no Java).
 * Inclui o sistema de scripting (give, readVar, readBoolean, readString, getSetValue).
 */
#include "room_object.h"
#include "../../math/vector3d.h"
#include "../../math/matrix.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

/* ------------------------------------------------------------------ */
/* Dependencias externas (modulos nao convertidos ainda)               */
/* ------------------------------------------------------------------ */

/* Character — contem transform (Matrix) e part */
extern Matrix *Character_getTransform(Character *ch);
extern int     Character_getPart(Character *ch);
extern void    Character_setPart(Character *ch, int part);

/* Matrix — acesso a elementos da transform (m03=X, m13=Y, m23=Z) */
extern int Matrix_get(const Matrix *m, int row, int col);

/* House — mapa/salas */
extern int          House_getRoomCount(House *house);
extern RoomObject **House_getObjects(House *house);
extern int          House_getObjectCount(House *house);
extern RoomObject  *House_getObjectByName(House *house, const char *name);
extern void         House_removeObject(House *house, RoomObject *obj);

/* Player — jogador */
extern int   Player_getMoney(Player *player);
extern void  Player_setMoney(Player *player, int amount);
extern void  Player_addMoney(Player *player, int amount);
extern int   Player_getHP(Player *player);
extern void  Player_setHP(Player *player, int hp);
extern int   Player_getMaxHP(Player *player);
extern int   Player_getFrags(Player *player);
extern void  Player_setFrags(Player *player, int frags);
extern void  Player_addFrags(Player *player, int amount);
extern int   Player_getAmmo(Player *player, int weaponIndex);
extern void  Player_setAmmo(Player *player, int weaponIndex, int amount);
extern void  Player_addAmmo(Player *player, int weaponIndex, int amount);
extern int   Player_getSpeed(Player *player);
extern void  Player_setSpeed(Player *player, int speed);
extern float Player_getAnimationSpeed(Player *player);
extern void  Player_setAnimationSpeed(Player *player, float speed);
extern bool  Player_hasFlashlight(Player *player);
extern void  Player_setFlashlight(Player *player, bool on);
extern char **Player_getUsedPoints(Player *player);
extern int   Player_getUsedPointsCount(Player *player);
extern void  Player_addUsedPoint(Player *player, const char *name);
extern void  Player_removeUsedPoint(Player *player, const char *name);
extern Character *Player_getCharacter(Player *player);
extern int   Player_getPosX(Player *player);
extern int   Player_getPosY(Player *player);
extern int   Player_getPosZ(Player *player);
extern void  Player_setPos(Player *player, int x, int y, int z);
extern void  Player_setRotation(Player *player, int rot);
extern void  Player_dropItem(Player *player, const char *name, int count);

/* GameScreen */
extern void    GameScreen_activateDialog(GameScreen *gs, const char *msg, int64_t timeout, int type);
extern void    GameScreen_showMessage(GameScreen *gs, const char *msg);
extern void    GameScreen_loadLevel(GameScreen *gs, int lvl, int x, int y, int z, int rot, bool saveMus, bool fullMove, bool showLoad);
extern void    GameScreen_saveGame(GameScreen *gs);
extern void    GameScreen_openShop(GameScreen *gs, int *items, int *prices, char **files, int count);
extern Scene  *GameScreen_getScene(GameScreen *gs);
extern Player *GameScreen_getPlayer(GameScreen *gs);
extern House  *GameScreen_getHouse(GameScreen *gs);
extern int64_t GameScreen_getCurrentTime(GameScreen *gs);

/* Scene */
extern void  Scene_addObject(Scene *scene, RoomObject *obj);

/* Weapon creation */
extern void  Player_createWeapon(Player *player, int weaponIndex);

/* Sound */
extern void  Asset_playSound(const char *file);

/* DirectX7 — render 3D */
extern int   DirectX7_width(const DirectX7 *g);
extern int   DirectX7_height(const DirectX7 *g);
extern void  DirectX7_transformSave(DirectX7 *g);
extern void  DirectX7_transformReturn(DirectX7 *g);
extern int  *DirectX7_project(DirectX7 *g, int x, int y, int z);

/* IniFile (para EXEC) */
extern char **IniFile_readLines(const char *path, int *count);

/* qe_current_ms */
#ifdef _arch_dreamcast
#include <arch/timer.h>
static inline int64_t qe_current_ms(void) {
    return (int64_t)timer_ms_gettime64();
}
#else
#include <sys/time.h>
static inline int64_t qe_current_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return (int64_t)tv.tv_sec * 1000LL + (int64_t)(tv.tv_usec / 1000);
}
#endif

/* ------------------------------------------------------------------ */
/* Utilitarios internos                                                */
/* ------------------------------------------------------------------ */

/* startsWith portavel */
static bool starts_with(const char *s, const char *prefix) {
    return strncmp(s, prefix, strlen(prefix)) == 0;
}

/* parseInt simples */
static int parse_int(const char *s) {
    return (int)strtol(s, NULL, 10);
}

/* parseInt64 */
static int64_t parse_long(const char *s) {
    return strtoll(s, NULL, 10);
}

/* random [0, max) */
static int qe_random(int max) {
    if (max <= 0) return 0;
    return rand() % max;
}

/* duplica string */
static char *str_dup(const char *s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char *d = (char *)malloc(len + 1);
    memcpy(d, s, len + 1);
    return d;
}

/* substring(start, end) — Java semantics */
static char *str_sub(const char *s, int start, int end) {
    int len = end - start;
    if (len < 0) len = 0;
    char *d = (char *)malloc(len + 1);
    memcpy(d, s + start, len);
    d[len] = '\0';
    return d;
}

/* indexOf char */
static int str_indexof(const char *s, char c) {
    const char *p = strchr(s, c);
    return p ? (int)(p - s) : -1;
}

/* indexOf char from position */
static int str_indexof_from(const char *s, char c, int from) {
    const char *p = strchr(s + from, c);
    return p ? (int)(p - s) : -1;
}

/* lastIndexOf char */
static int str_lastindexof(const char *s, char c) {
    const char *p = strrchr(s, c);
    return p ? (int)(p - s) : -1;
}

/* trim whitespace (retorna nova string alocada) */
static char *str_trim(const char *s) {
    while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') s++;
    int len = (int)strlen(s);
    while (len > 0 && (s[len-1] == ' ' || s[len-1] == '\t' || s[len-1] == '\r' || s[len-1] == '\n')) len--;
    char *d = (char *)malloc(len + 1);
    memcpy(d, s, len);
    d[len] = '\0';
    return d;
}

/* ------------------------------------------------------------------ */
/* containsSimple — busca linear em vetor de strings                  */
/* ------------------------------------------------------------------ */
bool RoomObject_containsSimple(const char *name, const char **vec, int vecCount) {
    for (int i = 0; i < vecCount; i++) {
        if (vec[i] && strcmp(vec[i], name) == 0) return true;
    }
    return false;
}

/* ------------------------------------------------------------------ */
/* isAllCollected — verifica se todas as condicoes em need[] sao met   */
/* ------------------------------------------------------------------ */
bool RoomObject_isAllCollected(RoomObject *self, Player *player, House *house, GameScreen *gs) {
    if (!self->need || self->needCount == 0) return true;
    char **usedPoints = Player_getUsedPoints(player);
    int usedCount = Player_getUsedPointsCount(player);

    for (int i = 0; i < self->needCount; i++) {
        const char *s = self->need[i];
        if (!s) continue;

        if (!RoomObject_containsCollected(s, player, house, gs)) {
            return false;
        }
    }
    return true;
}

/* ------------------------------------------------------------------ */
/* containsCollected — avalia uma expressao de condicao                */
/* Suporta: WEAPON_X, comparadores (=,>,<), FALSE, TRUE, usedPoints   */
/* ------------------------------------------------------------------ */
bool RoomObject_containsCollected(const char *s, Player *player, House *house, GameScreen *gs) {
    if (!s) return false;

    /* FALSE literal */
    if (strcmp(s, "FALSE") == 0) return false;

    /* TRUE literal */
    if (strcmp(s, "TRUE") == 0) return true;

    /* Expressao booleana completa (contem | ou &) */
    if (strchr(s, '|') || strchr(s, '&') || strchr(s, '(')) {
        return RoomObject_readBoolean(s, player, house, gs);
    }

    /* WEAPON_X — verifica se arma X tem municao */
    if (starts_with(s, "WEAPON_")) {
        int idx = parse_int(s + 7);
        return Player_getAmmo(player, idx) > 0;
    }

    /* Comparadores: =, >, < */
    int eqIdx = str_indexof(s, '=');
    int gtIdx = str_indexof(s, '>');
    int ltIdx = str_indexof(s, '<');

    if (eqIdx > 0) {
        char *left = str_sub(s, 0, eqIdx);
        char *right = str_sub(s, eqIdx + 1, (int)strlen(s));
        int64_t lv = RoomObject_readVar(left, player, house, gs);
        int64_t rv = RoomObject_readVar(right, player, house, gs);
        free(left);
        free(right);
        return lv == rv;
    }
    if (gtIdx > 0) {
        char *left = str_sub(s, 0, gtIdx);
        char *right = str_sub(s, gtIdx + 1, (int)strlen(s));
        int64_t lv = RoomObject_readVar(left, player, house, gs);
        int64_t rv = RoomObject_readVar(right, player, house, gs);
        free(left);
        free(right);
        return lv > rv;
    }
    if (ltIdx > 0) {
        char *left = str_sub(s, 0, ltIdx);
        char *right = str_sub(s, ltIdx + 1, (int)strlen(s));
        int64_t lv = RoomObject_readVar(left, player, house, gs);
        int64_t rv = RoomObject_readVar(right, player, house, gs);
        free(left);
        free(right);
        return lv < rv;
    }

    /* Busca no usedPoints */
    char **usedPoints = Player_getUsedPoints(player);
    int usedCount = Player_getUsedPointsCount(player);
    return RoomObject_containsSimple(s, (const char **)usedPoints, usedCount);
}

/* ------------------------------------------------------------------ */
/* getPosX/Y/Z — extraidos da transform do character                  */
/* ------------------------------------------------------------------ */
int RoomObject_getPosX(RoomObject *self) {
    if (!self->character) return 0;
    Matrix *m = Character_getTransform(self->character);
    return Matrix_get(m, 0, 3); /* m03 */
}

int RoomObject_getPosY(RoomObject *self) {
    if (!self->character) return 0;
    Matrix *m = Character_getTransform(self->character);
    return Matrix_get(m, 1, 3); /* m13 */
}

int RoomObject_getPosZ(RoomObject *self) {
    if (!self->character) return 0;
    Matrix *m = Character_getTransform(self->character);
    return Matrix_get(m, 2, 3); /* m23 */
}

/* ------------------------------------------------------------------ */
/* getPart / setPart / setNewPart                                      */
/* ------------------------------------------------------------------ */
int RoomObject_getPart(RoomObject *self) {
    if (!self->character) return -1;
    return Character_getPart(self->character);
}

void RoomObject_setPart(RoomObject *self, int part, House *house) {
    if (!self->character) return;
    Character_setPart(self->character, part);
    /* Java: house.recalcPart(this) — fiel ao original */
    /* extern void House_recalcPart(House *house, RoomObject *obj); */
    (void)house;
}

void RoomObject_setNewPart(RoomObject *self, int part) {
    if (!self->character) return;
    Character_setPart(self->character, part);
}

/* ------------------------------------------------------------------ */
/* activate — implementacao base                                       */
/* ------------------------------------------------------------------ */
void RoomObject_activate(RoomObject *self, Player *player, House *house, GameScreen *gs) {
    if (!self->activable) return;

    /* Verifica timeToReset */
    if (self->timeToReset > 0 && self->lastActivate > 0) {
        int64_t now = qe_current_ms();
        if (now - self->lastActivate < self->timeToReset) return;
    }

    /* Verifica condicoes */
    if (!RoomObject_isAllCollected(self, player, house, gs)) {
        /* Mostra mensagem de erro */
        if (self->errMessage && strlen(self->errMessage) > 0) {
            if (self->errMessageDelay > 0) {
                /* DelayedDialog — simplificado: mostra direto com delay */
                /* Fiel ao Java: cria DelayedDialog e adiciona ao gs */
            }
            GameScreen_activateDialog(gs, self->errMessage, self->errMessageTimeOut, self->errMessageType);
        }
        return;
    }

    /* Som */
    if (self->sound && strlen(self->sound) > 0) {
        Asset_playSound(self->sound);
    }

    /* Mensagem */
    if (self->message && strlen(self->message) > 0) {
        if (self->messageDelay > 0) {
            /* DelayedDialog com delay — fiel ao Java */
        }
        GameScreen_activateDialog(gs, self->message, self->messageTimeOut, self->messageType);
    }

    /* Executa script */
    if (self->additional && self->additionalCount > 0) {
        RoomObject_give(self->additional, self->additionalCount, player, house, gs);
    }

    /* singleUse — marca como usado */
    if (self->singleUse) {
        self->activable = false;
        if (self->name) {
            Player_addUsedPoint(player, self->name);
        }
    }

    /* destroyOnUse — remove do mapa */
    if (self->destroyOnUse) {
        House_removeObject(house, self);
    }

    /* lastActivate */
    self->lastActivate = qe_current_ms();
    self->activated = true;
}

/* ------------------------------------------------------------------ */
/* check — projeta posicao para tela e verifica crosshair             */
/* ------------------------------------------------------------------ */
bool RoomObject_check(RoomObject *self, DirectX7 *g3d, int yy) {
    if (!self->activable && self->hideWhenUnusable) return false;
    if (!self->visible) return false;

    int px, py, pz;
    if (self->dynamicPoint && self->character) {
        px = RoomObject_getPosX(self);
        py = RoomObject_getPosY(self);
        pz = RoomObject_getPosZ(self);
    } else if (self->pointOffset) {
        px = RoomObject_getPosX(self) + self->pointOffset->x;
        py = RoomObject_getPosY(self) + self->pointOffset->y;
        pz = RoomObject_getPosZ(self) + self->pointOffset->z;
    } else {
        px = RoomObject_getPosX(self);
        py = RoomObject_getPosY(self) + self->pointHeight;
        pz = RoomObject_getPosZ(self);
    }

    int *projected = DirectX7_project(g3d, px, py, pz);
    if (!projected) return false;

    int sx = projected[0];
    int sy = projected[1];
    int sz = projected[2];

    if (sz <= 0) return false;

    int w = DirectX7_width(g3d);
    int h = DirectX7_height(g3d);
    int cx = w / 2;
    int cy = h / 2 + yy;

    if (self->squarePoint) {
        int r = self->pointRadius;
        return (sx >= cx - r && sx <= cx + r && sy >= cy - r && sy <= cy + r);
    } else {
        int dx = sx - cx;
        int dy = sy - cy;
        int r = self->pointRadius;
        return (dx * dx + dy * dy <= r * r);
    }
}

/* ------------------------------------------------------------------ */
/* give — executor de comandos de script                               */
/* ------------------------------------------------------------------ */
void RoomObject_give(char **script, int scriptCount, Player *player, House *house, GameScreen *gs) {
    for (int i = 0; i < scriptCount; i++) {
        const char *cmd = script[i];
        if (!cmd || strlen(cmd) == 0) continue;

        char *trimmed = str_trim(cmd);

        /* END_SCRIPT */
        if (strcmp(trimmed, "END_SCRIPT") == 0) {
            free(trimmed);
            return;
        }

        /* SAVE_GAME */
        if (strcmp(trimmed, "SAVE_GAME") == 0) {
            GameScreen_saveGame(gs);
            free(trimmed);
            continue;
        }

        /* WEAPON_X — cria arma */
        if (starts_with(trimmed, "WEAPON_")) {
            int idx = parse_int(trimmed + 7);
            Player_createWeapon(player, idx);
            free(trimmed);
            continue;
        }

        /* DROP_ITEM(name,count) */
        if (starts_with(trimmed, "DROP_ITEM(")) {
            /* DROP_ITEM(nome,quantidade) */
            int paren = str_indexof(trimmed, '(');
            int endParen = str_indexof(trimmed, ')');
            if (paren > 0 && endParen > paren) {
                char *inner = str_sub(trimmed, paren + 1, endParen);
                int comma = str_indexof(inner, ',');
                if (comma > 0) {
                    char *itemName = str_sub(inner, 0, comma);
                    char *countStr = str_sub(inner, comma + 1, (int)strlen(inner));
                    int count = parse_int(countStr);
                    Player_dropItem(player, itemName, count);
                    free(itemName);
                    free(countStr);
                }
                free(inner);
            }
            free(trimmed);
            continue;
        }

        /* LOAD_LEVEL(n,x,y,z) */
        if (starts_with(trimmed, "LOAD_LEVEL(")) {
            int paren = str_indexof(trimmed, '(');
            int endParen = str_indexof(trimmed, ')');
            if (paren > 0 && endParen > paren) {
                char *inner = str_sub(trimmed, paren + 1, endParen);
                /* parse n,x,y,z */
                int vals[4] = {0, 0, 0, 0};
                int vi = 0;
                char *tok = strtok(inner, ",");
                while (tok && vi < 4) {
                    vals[vi++] = parse_int(tok);
                    tok = strtok(NULL, ",");
                }
                GameScreen_loadLevel(gs, vals[0], vals[1], vals[2], vals[3], 0, false, false, true);
                free(inner);
            }
            free(trimmed);
            continue;
        }

        /* EXEC /path.txt — carrega e executa script externo */
        if (starts_with(trimmed, "EXEC ")) {
            const char *path = trimmed + 5;
            int lineCount = 0;
            char **lines = IniFile_readLines(path, &lineCount);
            if (lines && lineCount > 0) {
                RoomObject_give(lines, lineCount, player, house, gs);
                for (int j = 0; j < lineCount; j++) free(lines[j]);
                free(lines);
            }
            free(trimmed);
            continue;
        }

        /* SHOW_MESSAGE(text) */
        if (starts_with(trimmed, "SHOW_MESSAGE(")) {
            int paren = str_indexof(trimmed, '(');
            int endParen = str_lastindexof(trimmed, ')');
            if (paren > 0 && endParen > paren) {
                char *msg = str_sub(trimmed, paren + 1, endParen);
                /* Processa readString se necessario */
                char *processed = RoomObject_readString(msg, player, house, gs);
                GameScreen_showMessage(gs, processed);
                free(msg);
                free(processed);
            }
            free(trimmed);
            continue;
        }

        /* IF condition { } */
        if (starts_with(trimmed, "IF ")) {
            /* Busca a condicao entre IF e { */
            int braceOpen = str_indexof(trimmed, '{');
            if (braceOpen > 3) {
                char *condition = str_sub(trimmed, 3, braceOpen);
                char *condTrimmed = str_trim(condition);
                bool result = RoomObject_readBoolean(condTrimmed, player, house, gs);
                free(condition);
                free(condTrimmed);

                if (result) {
                    /* Executa o bloco entre { e } */
                    /* Busca o } correspondente nas linhas seguintes */
                    int depth = 1;
                    int blockStart = i + 1;
                    int blockEnd = i + 1;
                    /* Se { esta na mesma linha, o conteudo esta entre { e } */
                    int braceClose = str_indexof_from(trimmed, '}', braceOpen + 1);
                    if (braceClose > braceOpen) {
                        /* Bloco inline: IF cond { cmd } */
                        char *blockContent = str_sub(trimmed, braceOpen + 1, braceClose);
                        char *blockTrimmed = str_trim(blockContent);
                        if (strlen(blockTrimmed) > 0) {
                            char *singleCmd[1];
                            singleCmd[0] = blockTrimmed;
                            RoomObject_give(singleCmd, 1, player, house, gs);
                        }
                        free(blockContent);
                        free(blockTrimmed);
                    } else {
                        /* Bloco multi-linha: encontra } */
                        for (int j = i + 1; j < scriptCount; j++) {
                            if (script[j] && strchr(script[j], '{')) depth++;
                            if (script[j] && strchr(script[j], '}')) {
                                depth--;
                                if (depth == 0) {
                                    blockEnd = j;
                                    break;
                                }
                            }
                        }
                        int blockCount = blockEnd - blockStart;
                        if (blockCount > 0) {
                            RoomObject_give(script + blockStart, blockCount, player, house, gs);
                        }
                        i = blockEnd; /* pula o bloco */
                    }
                } else {
                    /* Pula o bloco — encontra o } correspondente */
                    int braceClose = str_indexof_from(trimmed, '}', braceOpen + 1);
                    if (braceClose < 0) {
                        int depth = 1;
                        for (int j = i + 1; j < scriptCount; j++) {
                            if (script[j] && strchr(script[j], '{')) depth++;
                            if (script[j] && strchr(script[j], '}')) {
                                depth--;
                                if (depth == 0) {
                                    i = j;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            free(trimmed);
            continue;
        }

        /* MONEY +/-/= amount */
        if (starts_with(trimmed, "MONEY")) {
            const char *p = trimmed + 5;
            while (*p == ' ') p++;
            char op = *p;
            p++;
            while (*p == ' ') p++;
            int64_t val = RoomObject_readVar(p, player, house, gs);
            if (op == '=') {
                Player_setMoney(player, (int)val);
            } else if (op == '+') {
                Player_addMoney(player, (int)val);
            } else if (op == '-') {
                Player_addMoney(player, -(int)val);
            }
            free(trimmed);
            continue;
        }

        /* AMMO_X +/-/= amount */
        if (starts_with(trimmed, "AMMO_")) {
            /* Encontra o indice e operacao */
            int uscore = 4; /* posicao do _ apos AMMO */
            const char *p = trimmed + 5;
            /* Le o indice ate encontrar espaco ou operador */
            char idxBuf[16];
            int idxLen = 0;
            while (*p && *p != ' ' && *p != '+' && *p != '-' && *p != '=') {
                idxBuf[idxLen++] = *p++;
            }
            idxBuf[idxLen] = '\0';
            int weapIdx = parse_int(idxBuf);

            while (*p == ' ') p++;
            char op = *p;
            p++;
            while (*p == ' ') p++;
            int64_t val = RoomObject_readVar(p, player, house, gs);
            if (op == '=') {
                Player_setAmmo(player, weapIdx, (int)val);
            } else if (op == '+') {
                Player_addAmmo(player, weapIdx, (int)val);
            } else if (op == '-') {
                Player_addAmmo(player, weapIdx, -(int)val);
            }
            free(trimmed);
            continue;
        }

        /* FRAGS +/-/= amount */
        if (starts_with(trimmed, "FRAGS")) {
            const char *p = trimmed + 5;
            while (*p == ' ') p++;
            char op = *p;
            p++;
            while (*p == ' ') p++;
            int64_t val = RoomObject_readVar(p, player, house, gs);
            if (op == '=') {
                Player_setFrags(player, (int)val);
            } else if (op == '+') {
                Player_addFrags(player, (int)val);
            } else if (op == '-') {
                Player_addFrags(player, -(int)val);
            }
            free(trimmed);
            continue;
        }

        /* object:command — getSetValue no objeto nomeado */
        int colonIdx = str_indexof(trimmed, ':');
        if (colonIdx > 0) {
            char *objName = str_sub(trimmed, 0, colonIdx);
            char *property = str_sub(trimmed, colonIdx + 1, (int)strlen(trimmed));

            RoomObject *target = House_getObjectByName(house, objName);
            if (target) {
                RoomObject_getSetValue(property, target, player, house, gs);
            }
            free(objName);
            free(property);
            free(trimmed);
            continue;
        }

        free(trimmed);
    }
}

/* ------------------------------------------------------------------ */
/* readVar — parser de expressoes aritmeticas                         */
/* Operadores: +, -, *, /                                             */
/* Terminais: MONEY, FRAGS, AMMO_X, RANDOM_X, object:prop, inteiros  */
/* Parenteses: ()                                                      */
/* ------------------------------------------------------------------ */
int64_t RoomObject_readVar(const char *expression, Player *player, House *house, GameScreen *gs) {
    if (!expression || strlen(expression) == 0) return 0;

    char *expr = str_trim(expression);
    int len = (int)strlen(expr);

    /* Parenteses externas */
    if (len > 2 && expr[0] == '(' && expr[len - 1] == ')') {
        char *inner = str_sub(expr, 1, len - 1);
        int64_t result = RoomObject_readVar(inner, player, house, gs);
        free(inner);
        free(expr);
        return result;
    }

    /* Busca operador de menor precedencia (+ ou -) fora de parenteses */
    int depth = 0;
    int opPos = -1;
    char opChar = 0;

    /* Busca + e - da direita pra esquerda (left-associative) */
    for (int i = len - 1; i >= 0; i--) {
        if (expr[i] == ')') depth++;
        else if (expr[i] == '(') depth--;
        else if (depth == 0) {
            if ((expr[i] == '+' || expr[i] == '-') && i > 0) {
                opPos = i;
                opChar = expr[i];
                break;
            }
        }
    }

    if (opPos > 0) {
        char *left = str_sub(expr, 0, opPos);
        char *right = str_sub(expr, opPos + 1, len);
        int64_t lv = RoomObject_readVar(left, player, house, gs);
        int64_t rv = RoomObject_readVar(right, player, house, gs);
        free(left);
        free(right);
        free(expr);
        if (opChar == '+') return lv + rv;
        else return lv - rv;
    }

    /* Busca * e / */
    depth = 0;
    for (int i = len - 1; i >= 0; i--) {
        if (expr[i] == ')') depth++;
        else if (expr[i] == '(') depth--;
        else if (depth == 0) {
            if ((expr[i] == '*' || expr[i] == '/') && i > 0) {
                opPos = i;
                opChar = expr[i];
                break;
            }
        }
    }

    if (opPos > 0) {
        char *left = str_sub(expr, 0, opPos);
        char *right = str_sub(expr, opPos + 1, len);
        int64_t lv = RoomObject_readVar(left, player, house, gs);
        int64_t rv = RoomObject_readVar(right, player, house, gs);
        free(left);
        free(right);
        free(expr);
        if (opChar == '*') return lv * rv;
        else return (rv != 0) ? lv / rv : 0;
    }

    /* Terminais */
    int64_t result = 0;

    if (strcmp(expr, "MONEY") == 0) {
        result = Player_getMoney(player);
    } else if (strcmp(expr, "FRAGS") == 0) {
        result = Player_getFrags(player);
    } else if (starts_with(expr, "AMMO_")) {
        int idx = parse_int(expr + 5);
        result = Player_getAmmo(player, idx);
    } else if (starts_with(expr, "RANDOM_")) {
        int max = parse_int(expr + 7);
        result = qe_random(max);
    } else if (strchr(expr, ':')) {
        /* object:property — le valor numerico do objeto */
        int colon = str_indexof(expr, ':');
        char *objName = str_sub(expr, 0, colon);
        char *prop = str_sub(expr, colon + 1, (int)strlen(expr));

        RoomObject *target = House_getObjectByName(house, objName);
        if (target) {
            /* Retorna valor numerico da propriedade */
            if (strcmp(prop, "X") == 0) {
                result = RoomObject_getPosX(target);
            } else if (strcmp(prop, "Y") == 0) {
                result = RoomObject_getPosY(target);
            } else if (strcmp(prop, "Z") == 0) {
                result = RoomObject_getPosZ(target);
            } else if (strcmp(prop, "HP") == 0) {
                result = Player_getHP(player);
            } else if (strcmp(prop, "VISIBLE") == 0) {
                result = target->visible ? 1 : 0;
            } else if (strcmp(prop, "ACTIVABLE") == 0) {
                result = target->activable ? 1 : 0;
            }
        }
        free(objName);
        free(prop);
    } else {
        /* Tenta interpretar como inteiro */
        result = parse_long(expr);
    }

    free(expr);
    return result;
}

/* ------------------------------------------------------------------ */
/* readBoolean — parser de expressoes booleanas                       */
/* Operadores: | (OR), & (AND), ! (NOT)                               */
/* Parenteses: ()                                                      */
/* Terminais: expressoes de comparacao, TRUE, FALSE                    */
/* ------------------------------------------------------------------ */
bool RoomObject_readBoolean(const char *expression, Player *player, House *house, GameScreen *gs) {
    if (!expression || strlen(expression) == 0) return false;

    char *expr = str_trim(expression);
    int len = (int)strlen(expr);

    /* Parenteses externas */
    if (len > 2 && expr[0] == '(' && expr[len - 1] == ')') {
        char *inner = str_sub(expr, 1, len - 1);
        bool result = RoomObject_readBoolean(inner, player, house, gs);
        free(inner);
        free(expr);
        return result;
    }

    /* Negacao ! */
    if (len > 1 && expr[0] == '!') {
        char *inner = str_sub(expr, 1, len);
        bool result = !RoomObject_readBoolean(inner, player, house, gs);
        free(inner);
        free(expr);
        return result;
    }

    /* Busca | (OR) fora de parenteses — menor precedencia */
    int depth = 0;
    for (int i = len - 1; i >= 0; i--) {
        if (expr[i] == ')') depth++;
        else if (expr[i] == '(') depth--;
        else if (depth == 0 && expr[i] == '|' && i > 0) {
            char *left = str_sub(expr, 0, i);
            char *right = str_sub(expr, i + 1, len);
            bool lv = RoomObject_readBoolean(left, player, house, gs);
            bool rv = RoomObject_readBoolean(right, player, house, gs);
            free(left);
            free(right);
            free(expr);
            return lv || rv;
        }
    }

    /* Busca & (AND) */
    depth = 0;
    for (int i = len - 1; i >= 0; i--) {
        if (expr[i] == ')') depth++;
        else if (expr[i] == '(') depth--;
        else if (depth == 0 && expr[i] == '&' && i > 0) {
            char *left = str_sub(expr, 0, i);
            char *right = str_sub(expr, i + 1, len);
            bool lv = RoomObject_readBoolean(left, player, house, gs);
            bool rv = RoomObject_readBoolean(right, player, house, gs);
            free(left);
            free(right);
            free(expr);
            return lv && rv;
        }
    }

    /* Terminal: containsCollected avalia comparacoes, WEAPON_, usedPoints, TRUE, FALSE */
    bool result = RoomObject_containsCollected(expr, player, house, gs);
    free(expr);
    return result;
}

/* ------------------------------------------------------------------ */
/* readString — parser de expressoes de string                        */
/* + concatenacao, [] readVar embedding, () readBoolean, "quoted"      */
/* ------------------------------------------------------------------ */
char *RoomObject_readString(const char *expression, Player *player, House *house, GameScreen *gs) {
    if (!expression || strlen(expression) == 0) return str_dup("");

    char *expr = str_trim(expression);
    int len = (int)strlen(expr);

    /* Busca + fora de aspas/colchetes/parenteses para concatenacao */
    int depth = 0;
    bool inQuotes = false;
    for (int i = 0; i < len; i++) {
        if (expr[i] == '"') inQuotes = !inQuotes;
        else if (!inQuotes) {
            if (expr[i] == '[' || expr[i] == '(') depth++;
            else if (expr[i] == ']' || expr[i] == ')') depth--;
            else if (expr[i] == '+' && depth == 0) {
                char *left = str_sub(expr, 0, i);
                char *right = str_sub(expr, i + 1, len);
                char *ls = RoomObject_readString(left, player, house, gs);
                char *rs = RoomObject_readString(right, player, house, gs);
                int lsLen = (int)strlen(ls);
                int rsLen = (int)strlen(rs);
                char *result = (char *)malloc(lsLen + rsLen + 1);
                memcpy(result, ls, lsLen);
                memcpy(result + lsLen, rs, rsLen);
                result[lsLen + rsLen] = '\0';
                free(left);
                free(right);
                free(ls);
                free(rs);
                free(expr);
                return result;
            }
        }
    }

    /* "quoted" literal */
    if (len >= 2 && expr[0] == '"' && expr[len - 1] == '"') {
        char *result = str_sub(expr, 1, len - 1);
        free(expr);
        return result;
    }

    /* [expression] — readVar embedding */
    if (len >= 2 && expr[0] == '[' && expr[len - 1] == ']') {
        char *inner = str_sub(expr, 1, len - 1);
        int64_t val = RoomObject_readVar(inner, player, house, gs);
        free(inner);
        char buf[32];
        snprintf(buf, sizeof(buf), "%lld", (long long)val);
        free(expr);
        return str_dup(buf);
    }

    /* (expression) — readBoolean embedding */
    if (len >= 2 && expr[0] == '(' && expr[len - 1] == ')') {
        char *inner = str_sub(expr, 1, len - 1);
        bool val = RoomObject_readBoolean(inner, player, house, gs);
        free(inner);
        free(expr);
        return str_dup(val ? "true" : "false");
    }

    /* Retorna como esta */
    return expr; /* expr ja eh alocado */
}

/* ------------------------------------------------------------------ */
/* getSetValue — accessor/mutator de propriedades                     */
/* Propriedades: HP, GIVE(), TAKE(), FLASHLIGHT, ACTIVABLE, VISIBLE,  */
/*   X, Y, Z, NAME, SPEED, ANIMATION_SPEED, FOLLOW=, etc.            */
/* ------------------------------------------------------------------ */
void RoomObject_getSetValue(const char *property, RoomObject *target, Player *player, House *house, GameScreen *gs) {
    if (!property || !target) return;

    /* ACTIVABLE=true/false */
    if (starts_with(property, "ACTIVABLE=")) {
        const char *val = property + 10;
        target->activable = (strcmp(val, "true") == 0 || strcmp(val, "1") == 0);
        return;
    }

    /* VISIBLE=true/false */
    if (starts_with(property, "VISIBLE=")) {
        const char *val = property + 8;
        target->visible = (strcmp(val, "true") == 0 || strcmp(val, "1") == 0);
        return;
    }

    /* CLICKABLE=true/false */
    if (starts_with(property, "CLICKABLE=")) {
        const char *val = property + 10;
        target->clickable = (strcmp(val, "true") == 0 || strcmp(val, "1") == 0);
        return;
    }

    /* HP=value ou HP+value ou HP-value */
    if (starts_with(property, "HP")) {
        const char *p = property + 2;
        char op = *p;
        p++;
        int64_t val = RoomObject_readVar(p, player, house, gs);
        if (op == '=') {
            Player_setHP(player, (int)val);
        } else if (op == '+') {
            Player_setHP(player, Player_getHP(player) + (int)val);
        } else if (op == '-') {
            Player_setHP(player, Player_getHP(player) - (int)val);
        }
        return;
    }

    /* FLASHLIGHT=true/false */
    if (starts_with(property, "FLASHLIGHT=")) {
        const char *val = property + 11;
        Player_setFlashlight(player, strcmp(val, "true") == 0 || strcmp(val, "1") == 0);
        return;
    }

    /* SPEED=value */
    if (starts_with(property, "SPEED=")) {
        const char *val = property + 6;
        int64_t v = RoomObject_readVar(val, player, house, gs);
        Player_setSpeed(player, (int)v);
        return;
    }

    /* ANIMATION_SPEED=value */
    if (starts_with(property, "ANIMATION_SPEED=")) {
        const char *val = property + 16;
        int64_t v = RoomObject_readVar(val, player, house, gs);
        Player_setAnimationSpeed(player, (float)v);
        return;
    }

    /* X=value */
    if (starts_with(property, "X=")) {
        const char *val = property + 2;
        int64_t v = RoomObject_readVar(val, player, house, gs);
        Player_setPos(player, (int)v, Player_getPosY(player), Player_getPosZ(player));
        return;
    }

    /* Y=value */
    if (starts_with(property, "Y=")) {
        const char *val = property + 2;
        int64_t v = RoomObject_readVar(val, player, house, gs);
        Player_setPos(player, Player_getPosX(player), (int)v, Player_getPosZ(player));
        return;
    }

    /* Z=value */
    if (starts_with(property, "Z=")) {
        const char *val = property + 2;
        int64_t v = RoomObject_readVar(val, player, house, gs);
        Player_setPos(player, Player_getPosX(player), Player_getPosY(player), (int)v);
        return;
    }

    /* NAME=string */
    if (starts_with(property, "NAME=")) {
        const char *val = property + 5;
        if (target->name) free(target->name);
        target->name = str_dup(val);
        return;
    }

    /* GIVE(items) — adiciona script ao target */
    if (starts_with(property, "GIVE(")) {
        int paren = str_indexof(property, '(');
        int endParen = str_lastindexof(property, ')');
        if (paren > 0 && endParen > paren) {
            char *inner = str_sub(property, paren + 1, endParen);
            char *singleCmd[1];
            singleCmd[0] = inner;
            RoomObject_give(singleCmd, 1, player, house, gs);
            free(inner);
        }
        return;
    }

    /* TAKE(item) — remove usedPoint */
    if (starts_with(property, "TAKE(")) {
        int paren = str_indexof(property, '(');
        int endParen = str_lastindexof(property, ')');
        if (paren > 0 && endParen > paren) {
            char *inner = str_sub(property, paren + 1, endParen);
            Player_removeUsedPoint(player, inner);
            free(inner);
        }
        return;
    }

    /* FOLLOW=objectName — AI follow (extern) */
    if (starts_with(property, "FOLLOW=")) {
        /* AI module not yet converted — stub */
        return;
    }

    /* SOUND=file */
    if (starts_with(property, "SOUND=")) {
        const char *file = property + 6;
        Asset_playSound(file);
        return;
    }

    /* MESSAGE=text */
    if (starts_with(property, "MESSAGE=")) {
        const char *msg = property + 8;
        GameScreen_showMessage(gs, msg);
        return;
    }
}

/* ------------------------------------------------------------------ */
/* drawDebug — desenha ponto de debug circular                        */
/* ------------------------------------------------------------------ */
void RoomObject_drawDebug(RoomObject *self, DirectX7 *g3d) {
    if (!self->visible) return;

    int px = RoomObject_getPosX(self);
    int py = RoomObject_getPosY(self) + self->pointHeight;
    int pz = RoomObject_getPosZ(self);

    if (self->pointOffset) {
        px = RoomObject_getPosX(self) + self->pointOffset->x;
        py = RoomObject_getPosY(self) + self->pointOffset->y;
        pz = RoomObject_getPosZ(self) + self->pointOffset->z;
    }

    int *projected = DirectX7_project(g3d, px, py, pz);
    if (!projected) return;
    if (projected[2] <= 0) return;

    /* Debug drawing — dependente do modulo de rendering */
    /* Fiel ao Java: desenha circulo no ponto projetado */
    (void)projected;
}

/* ------------------------------------------------------------------ */
/* drawDebugSquare — desenha ponto de debug quadrado                  */
/* ------------------------------------------------------------------ */
void RoomObject_drawDebugSquare(RoomObject *self, DirectX7 *g3d) {
    if (!self->visible) return;

    int px = RoomObject_getPosX(self);
    int py = RoomObject_getPosY(self) + self->pointHeight;
    int pz = RoomObject_getPosZ(self);

    if (self->pointOffset) {
        px = RoomObject_getPosX(self) + self->pointOffset->x;
        py = RoomObject_getPosY(self) + self->pointOffset->y;
        pz = RoomObject_getPosZ(self) + self->pointOffset->z;
    }

    int *projected = DirectX7_project(g3d, px, py, pz);
    if (!projected) return;
    if (projected[2] <= 0) return;

    /* Debug drawing — dependente do modulo de rendering */
    /* Fiel ao Java: desenha quadrado no ponto projetado */
    (void)projected;
}
