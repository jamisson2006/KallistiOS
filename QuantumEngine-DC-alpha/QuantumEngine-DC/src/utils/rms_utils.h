/*
 * rms_utils.h — porte fiel de code/utils/RmsUtils.java (Quantum Engine J2ME)
 * @author Roman Lahin
 *
 * O J2ME RecordStore era persistente. No Dreamcast usamos o VMU via KOS
 * (vmufs/vmu_pkg) para armazenar os saves; o schema conserva a mesma API
 * booleana get/put/has/remove.
 */
#ifndef QE_UTILS_RMS_UTILS_H
#define QE_UTILS_RMS_UTILS_H

#include <stddef.h>

int    RmsUtils_removeStore(const char *name);
/* openStore retorna dados heap-alocados (chamador free()). Retorna NULL
 * em erro (equivalente Java: null). *out_len recebe tamanho. */
void  *RmsUtils_openStore(const char *name, size_t *out_len);
int    RmsUtils_saveStore(const char *name, const void *data, size_t len);
int    RmsUtils_hasStore(const char *name);

#endif
