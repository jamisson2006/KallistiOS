/*
 * rms_utils.c — porte fiel de code/utils/RmsUtils.java (Quantum Engine J2ME)
 *
 * No Dreamcast usamos arquivos comuns no VMU montado em /vmu/aX (X = porta,
 * a = slot A) — semantica identica ao RecordStore para as necessidades do
 * engine (1 record por store). No host (testes) caimos em /tmp/qe_rms.
 */
#include "rms_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _arch_dreamcast
#include <sys/stat.h>
/* Slot padrao: VMU no controller A1 */
#define QE_RMS_DIR "/vmu/a1"
#else
#define QE_RMS_DIR "/tmp"
#endif

static void qe_rms_path(const char *name, char *out, size_t out_sz) {
    /* nomes de VMU precisam ser curtos (<=12 chars), mas passamos direto:
     * o filesystem KOS truncara/hashara conforme necessario. */
    snprintf(out, out_sz, "%s/%s", QE_RMS_DIR, name);
}

int RmsUtils_removeStore(const char *name) {
    char p[128]; qe_rms_path(name, p, sizeof p);
    return remove(p) == 0 ? 1 : 0;
}

void *RmsUtils_openStore(const char *name, size_t *out_len) {
    char p[128]; qe_rms_path(name, p, sizeof p);
    FILE *fp = fopen(p, "rb");
    if (!fp) { if (out_len) *out_len = 0; return NULL; }

    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (sz <= 0) { fclose(fp); if (out_len) *out_len = 0; return NULL; }

    void *buf = malloc((size_t) sz);
    if (!buf) { fclose(fp); if (out_len) *out_len = 0; return NULL; }

    size_t n = fread(buf, 1, (size_t) sz, fp);
    fclose(fp);
    if (n != (size_t) sz) { free(buf); if (out_len) *out_len = 0; return NULL; }

    if (out_len) *out_len = n;
    return buf;
}

int RmsUtils_saveStore(const char *name, const void *data, size_t len) {
    char p[128]; qe_rms_path(name, p, sizeof p);
    FILE *fp = fopen(p, "wb");
    if (!fp) return 0;
    size_t n = fwrite(data, 1, len, fp);
    fclose(fp);
    return n == len ? 1 : 0;
}

int RmsUtils_hasStore(const char *name) {
    char p[128]; qe_rms_path(name, p, sizeof p);
    FILE *fp = fopen(p, "rb");
    if (!fp) return 0;
    /* Java: rs.getNumRecords() > 0 — tratamos "tem >0 bytes" como
     * equivalente para as necessidades do engine (1 record por store). */
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fclose(fp);
    return sz > 0 ? 1 : 0;
}
