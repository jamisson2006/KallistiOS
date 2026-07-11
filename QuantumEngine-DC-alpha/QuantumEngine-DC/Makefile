# ============================================================
#  Quantum Engine - Dreamcast Port
#  Makefile KallistiOS
#  source /opt/toolchains/dc/kos/environ.sh
# ============================================================
TARGET = quantum.elf

OBJS = \
    src/main.o \
    src/game.o \
    src/core/logger.o \
    src/core/qfps.o \
    src/core/res.o \
    src/core/input.o \
    src/config/inifile.o \
    src/render/framebuffer.o \
    src/render/font.o \
    src/render/raw_image.o \
    src/render/dc_font.o \
    src/render/qe_font.o \
    src/math3d/matrix.o \
    src/math3d/vector3d.o \
    src/math3d/mathutils2.o \
    src/collision/ray.o \
    src/collision/height.o \
    src/collision/raycast.o \
    src/collision/spherecast.o \
    src/collision/heightcomputer.o \
    src/render3d/directx7.o \
    src/render3d/texturing_affine.o \
    src/render3d/polygon.o \
    src/render3d/renderbuffer.o \
    src/meshes/mesh.o \
    src/meshes/multytexture.o \
    src/utils_c/sound.o \
    src/utils_c/asset.o \
    src/screens/screenmgr.o \
    src/screens/splash.o \
    src/screens/menu.o \
    src/screens/notimpl.o \
    src/screens/gamescreen.o \
    src/screens/levelselection.o \
    src/screens/help.o \
    src/screens/setting.o \
    src/screens/pausescreen.o \
    src/screens/loadingscreen.o \
    src/gameplay/level_loader.o \
    romdisk.o

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
