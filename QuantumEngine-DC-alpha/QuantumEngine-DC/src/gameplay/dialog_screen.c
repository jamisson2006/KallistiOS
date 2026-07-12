/*
 * dialog_screen.c — porte fiel de code/Gameplay/DialogScreen.java
 */
#include "dialog_screen.h"
#include <stdlib.h>
#include <string.h>

extern char  *StringTools_getStringFromResource(const char *path);
extern char **StringTools_cutOnStrings(const char *str, char sep, int *outCount);
extern int    StringTools_parseInt(const char *str);

extern void GameScreen_runScriptFromFile(GameScreen *gs, const char *file);
extern bool GameScreen_readBooleanFromScript(GameScreen *gs, const char *expr);
extern void GameScreen_runScript(GameScreen *gs, char **lines, int count);
extern void GameScreen_start(GameScreen *gs);

extern void QFPS_miniReset(void);

static bool nextText(DialogScreen *self);

static void clearBcks(DialogScreen *self) {
    for (int i = 0; i < 10; i++) {
        self->bckList[i] = NULL;
    }
}

static bool goToLabel(DialogScreen *self, const char *option) {
    int optLen = (int)strlen(option);
    for (int lineId = 0; lineId < self->dialogCount; lineId++) {
        const char *line = self->dialog[lineId];
        int lineLen = (int)strlen(line);

        if (optLen == lineLen - 2 &&
            line[0] == '$' && line[lineLen - 1] == ':' &&
            strncmp(line + 1, option, optLen) == 0) {
            self->index = lineId;
            return nextText(self);
        }
    }
    return false;
}

static bool nextText(DialogScreen *self) {
    if (self->questionItems != NULL) {
        if (self->answersGoIndex == NULL) {
            int cap = self->itemListHasCaption ? 1 : 0;
            self->index += self->questionCount + self->questionIndex - cap;
        } else {
            int cap = self->itemListHasCaption ? 1 : 0;
            self->index = self->answersGoIndex[self->questionIndex - cap] - 1;
        }
        free(self->questionItems);
        self->questionItems = NULL;
        self->questionCount = 0;
        free(self->answersGoIndex);
        self->answersGoIndex = NULL;
        self->answersGoCount = 0;
    }

    if (self->index + 1 < self->dialogCount) {
        self->index++;
        const char *text = self->dialog[self->index];

        if (text[0] != '$') {
            /* Normal text — set to textView */
            return true;
        }

        /* Script command */
        int spacePlace = -1;
        for (int i = 0; text[i]; i++) {
            if (text[i] == ' ') { spacePlace = i; break; }
        }

        char script[64];
        const char *option = text;
        if (spacePlace >= 0) {
            int len = spacePlace - 1;
            if (len > 63) len = 63;
            strncpy(script, text + 1, len);
            script[len] = '\0';
            option = text + spacePlace + 1;
        } else {
            strncpy(script, text + 1, 63);
            script[63] = '\0';
        }

        /* Convert script to lowercase */
        for (int i = 0; script[i]; i++) {
            if (script[i] >= 'A' && script[i] <= 'Z') script[i] += 32;
        }

        if (strcmp(script, "exec") == 0) {
            GameScreen_runScriptFromFile(self->gs, option);
            return nextText(self);
        } else if (strcmp(script, "if") == 0) {
            if (!GameScreen_readBooleanFromScript(self->gs, option)) self->index++;
            return nextText(self);
        } else if (strcmp(script, "cmd") == 0) {
            char *line = strdup(option);
            GameScreen_runScript(self->gs, &line, 1);
            free(line);
            return nextText(self);
        } else if (strncmp(script, "bck", 3) == 0) {
            /* Background loading — stub for DC port */
            return nextText(self);
        } else if (strncmp(script, "avatar", 6) == 0) {
            /* Avatar loading — stub for DC port */
            return nextText(self);
        } else if (strcmp(script, "go") == 0) {
            return goToLabel(self, option);
        } else if (strcmp(script, "question") == 0) {
            /* Question system — simplified stub */
            return true;
        } else if (strcmp(script, "end") == 0) {
            return false;
        }

        return nextText(self);
    }

    return false;
}

DialogScreen *DialogScreen_new(const char *text, Font *font, DirectX7 *g3d, GameScreen *gs) {
    DialogScreen *self = (DialogScreen *)calloc(1, sizeof(DialogScreen));
    self->g3d = g3d;
    self->gs = gs;
    self->index = -1;
    clearBcks(self);

    if (text != NULL) {
        DialogScreen_set(self, text, font, g3d, gs);
    }
    return self;
}

void DialogScreen_set(DialogScreen *self, const char *text, Font *font,
                      DirectX7 *g3d, GameScreen *gs) {
    self->g3d = g3d;
    self->gs = gs;
    self->index = -1;

    free(self->questionItems);
    self->questionItems = NULL;
    self->questionCount = 0;

    const char *newText = NULL;
    if (text[0] == '/' && strlen(text) > 4) {
        const char *ext = text + strlen(text) - 4;
        if (strcmp(ext, ".txt") == 0 || strcmp(ext, ".TXT") == 0) {
            newText = StringTools_getStringFromResource(text);
        }
    }

    int count = 0;
    if (newText == NULL) {
        self->dialog = StringTools_cutOnStrings(text, '@', &count);
    } else {
        self->dialog = StringTools_cutOnStrings(newText, '\n', &count);
    }
    self->dialogCount = count;

    /* Trim leading/trailing newlines from each line */
    for (int i = 0; i < self->dialogCount; i++) {
        char *s = self->dialog[i];
        if (s && s[0] == '\n') {
            memmove(s, s + 1, strlen(s));
        }
        int len = (int)strlen(s);
        if (len > 0 && s[len - 1] == '\n') {
            s[len - 1] = '\0';
        }
    }

    (void)font;
    nextText(self);
}

void DialogScreen_destroy(DialogScreen *self) {
    if (!self) return;
    if (self->dialog) {
        for (int i = 0; i < self->dialogCount; i++) free(self->dialog[i]);
        free(self->dialog);
    }
    clearBcks(self);
    self->avatar = NULL;
    free(self->questionItems);
    free(self->answersGoIndex);
    free(self);
}

void DialogScreen_paint(DialogScreen *self, Graphics *g) {
    /* Paint — connected at integration with DC framebuffer */
    (void)g;

    /* Step logic */
    if (self->scrollDown) {
        /* scroll text or question list down */
        self->scrollDown = false;
    }
    if (self->scrollUp) {
        self->scrollUp = false;
    }
}

void DialogScreen_keyPressed(DialogScreen *self, int keyCode) {
    (void)keyCode;
    /* Fire key advances text */
}

void DialogScreen_keyReleased(DialogScreen *self, int keyCode) {
    (void)keyCode;
}

void DialogScreen_pointerPressed(DialogScreen *self, int x, int y) {
    (void)x; (void)y;
}

void DialogScreen_pointerReleased(DialogScreen *self, int x, int y) {
    (void)x; (void)y;
}

char *DialogScreen_loadTextFromFile(const char *text) {
    if (text == NULL) return NULL;

    int len = (int)strlen(text);
    if (text[0] != '/' || len < 5) return strdup(text);

    const char *ext = text + len - 4;
    if (strcmp(ext, ".txt") != 0 && strcmp(ext, ".TXT") != 0) return strdup(text);

    char *loaded = StringTools_getStringFromResource(text);
    if (!loaded) return strdup(text);

    /* Replace \n with @ */
    for (int i = 0; loaded[i]; i++) {
        if (loaded[i] == '\n') loaded[i] = '@';
    }
    return loaded;
}
