#!/bin/bash
# ============================================================
#  make_cdi.sh <input.elf> <output.cdi>
#  Gera .cdi bootavel para Flycast.
#
#  Estrategia:
#    1) mkdcdisc (o mais simples — instale com meson do repo simulant)
#    2) fallback: sh-elf-objcopy -R .stack -O binary + scramble + mkisofs + cdi4dc
# ============================================================
set -e
ELF="${1:-quantum.elf}"
CDI="${2:-quantum.cdi}"

[ ! -f "$ELF" ] && { echo "ERRO: $ELF nao existe"; exit 1; }

# ---- Opcao 1: mkdcdisc ----
if command -v mkdcdisc >/dev/null 2>&1; then
    echo ">> Usando mkdcdisc"
    mkdcdisc -e "$ELF" -o "$CDI" -N
    echo ">> Gerado: $CDI"
    exit 0
fi

echo ">> mkdcdisc nao encontrado; tentando fluxo classico"

# ---- Opcao 2: fluxo classico ----
# resolve toolchain
OBJCOPY="${KOS_OBJCOPY:-sh-elf-objcopy}"
if ! command -v "$OBJCOPY" >/dev/null 2>&1; then
    OBJCOPY="/opt/toolchains/dc/sh-elf/bin/sh-elf-objcopy"
fi
[ ! -x "$OBJCOPY" ] && [ ! -f "$OBJCOPY" ] && \
    { echo "ERRO: sh-elf-objcopy nao encontrado"; exit 1; }

TMP=$(mktemp -d)
echo ">> ELF -> BIN via $OBJCOPY"
"$OBJCOPY" -R .stack -O binary "$ELF" "$TMP/quantum.bin"

# scramble
SCRAMBLE=""
for c in scramble "$KOS_BASE/utils/scramble/scramble" \
         "/opt/toolchains/dc/kos/utils/scramble/scramble"; do
    if command -v "$c" >/dev/null 2>&1 || [ -x "$c" ]; then SCRAMBLE="$c"; break; fi
done
[ -z "$SCRAMBLE" ] && { echo "ERRO: 'scramble' nao encontrado. Instale mkdcdisc (recomendado)."; rm -rf "$TMP"; exit 1; }

mkdir -p "$TMP/cd"
"$SCRAMBLE" "$TMP/quantum.bin" "$TMP/cd/1ST_READ.BIN"

# IP.BIN
IPBIN=""
for c in "$KOS_BASE/utils/bootstraps/IP.BIN" \
         "/opt/toolchains/dc/kos/utils/bootstraps/IP.BIN" \
         "IP.BIN"; do
    [ -f "$c" ] && IPBIN="$c" && break
done

# mkisofs -> iso
if [ -n "$IPBIN" ]; then
    mkisofs -C 0,11702 -V QUANTUM -G "$IPBIN" -joliet -rock -l \
            -o "$TMP/quantum.iso" "$TMP/cd" 2>/dev/null || \
    mkisofs -V QUANTUM -joliet -rock -l -o "$TMP/quantum.iso" "$TMP/cd"
else
    mkisofs -V QUANTUM -joliet -rock -l -o "$TMP/quantum.iso" "$TMP/cd"
fi

# cdi4dc -> cdi
if command -v cdi4dc >/dev/null 2>&1; then
    cdi4dc "$TMP/quantum.iso" "$CDI" >/dev/null
    rm -rf "$TMP"
    echo ">> Gerado: $CDI"
else
    echo "AVISO: cdi4dc nao encontrado — .iso gerado em $TMP/quantum.iso"
    echo "       O Flycast tambem carrega .elf direto: flycast $ELF"
    cp "$TMP/quantum.iso" ./quantum.iso 2>/dev/null || true
    rm -rf "$TMP"
    exit 1
fi
