# ============================================================
#  Quantum Engine - Dreamcast Port
#  Makefile KallistiOS
#  source /opt/toolchains/dc/kos/environ.sh
# ============================================================
TARGET = quantum.elf

# Descoberta automatica de todos os .c convertidos (utils, math, rendering,
# hud, gameplay, collision, ai) + o entry point KOS em src/main.c.
# Evita que o Makefile fique dessincronizado da arvore real de fontes.
SRCS = $(shell find src -name '*.c')
OBJS = $(SRCS:.c=.o) romdisk.o

KOS_ROMDISK_DIR = romdisk

# stb_image: single-header, sem dependencias externas
# -Ivendor para achar stb_image.h  |  -Isrc para os headers do engine
KOS_CFLAGS += -Isrc -Ivendor -Wall

# stb_image usa muitas features do C std que o KOS desabilita por
# default; garantimos que o C99/11 esteja ativo
KOS_CFLAGS += -std=gnu11

all: rm-elf sync-res $(TARGET)

include $(KOS_BASE)/Makefile.rules

sync-res:
	@echo "  SYNC   res/ -> $(KOS_ROMDISK_DIR)/"
	@mkdir -p $(KOS_ROMDISK_DIR)
	@cp -rf res/. $(KOS_ROMDISK_DIR)/

clean: rm-elf
	-rm -f $(OBJS)
	-rm -f romdisk.img quantum.bin 1ST_READ.BIN quantum.cdi

rm-elf:
	-rm -f $(TARGET) romdisk.o romdisk.img

$(TARGET): $(OBJS)
	kos-cc -o $(TARGET) $(OBJS) -lm

cdi: $(TARGET)
	@./tools/make_cdi.sh $(TARGET) quantum.cdi

run: $(TARGET)
	$(KOS_LOADER) $(TARGET)

.PHONY: all clean rm-elf sync-res cdi run
