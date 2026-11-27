# pso-metal P2-Z display-list slice build — mirrors host/gx/gx_window.mk
# (same substrate objects, same flag conventions), OUT=build/gxdl.
#   make -f host/gx/dl/gx_dl.mk all | clean
SHELL := /bin/bash
REPO      := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/../../..)
BUILD_DIR := $(REPO)/build
GXDIR     := $(REPO)/host/gx
DLDIR     := $(GXDIR)/dl
BRINGUP   := $(REPO)/host/bringup
DECOMP_ROOT ?= $(HOME)/Hermes/pso-decomp/pso-decomp-port-pin
COMPAT    := $(REPO)/compat/pso_metal_compat.h
WAIVER_HDR  := $(BUILD_DIR)/waivers/pso_assert_waivers.h

LINK_TARGET ?= arm64-apple-macos12.0
OUT := $(BUILD_DIR)/gxdl
BIN := $(OUT)/bin

SDK    := $(shell xcrun --sdk macosx --show-sdk-path)
COMMON := -w -fms-extensions -Wno-c++11-narrowing \
          -DPSO_PORT_LAYOUT_WAIVER -D_DARWIN_C_SOURCE \
          -include $(COMPAT) -include $(WAIVER_HDR)
CFLAGS := -std=c11 -g $(COMMON) -I$(BRINGUP)/include -I$(GXDIR) -target $(LINK_TARGET)
HOSTCFLAGS := -std=c11 -g -Wall -Wextra -DPSO_PORT_LAYOUT_WAIVER -D_DARWIN_C_SOURCE \
              -I$(BRINGUP) -I$(BRINGUP)/include -I$(GXDIR) -I$(DLDIR) -target $(LINK_TARGET)
OBJCFLAGS := -fobjc-arc -g -Wall -D_DARWIN_C_SOURCE -I$(BRINGUP)/include \
             -I$(GXDIR) -I$(DLDIR) -isysroot $(SDK) -target $(LINK_TARGET)
FRAMEWORKS := -framework Foundation -framework AppKit -framework Metal -framework QuartzCore

.PHONY: all clean logo logo-av1
all: $(BIN)/gx_dl
logo: $(BIN)/gx_logo
logo-av1: $(BIN)/gx_logo_av1

# ---- P2-BA V4: LOGO VISIBLE binary (docs/p2-v-wave.md) ----
# Era-chain objects come from the bringup fragment (sub-make; the WAIVER_HDR
# regen hazard is untouched there — bringup.mk is D8 verify-only). Driver +
# logo glue + consumer compile under the SAME per-TU NJS shadow flags as
# ladder6 (docs/p2-njs-repin-1.md §6: .cpp TUs ONLY).
BRINGUB ?= $(BUILD_DIR)/bringup
NJS6 := -I$(REPO)/compat/njs -I$(DECOMP_ROOT)/libs/dolphin/include
L6CXXFLAGS := -std=c++17 -g $(COMMON) -I$(BRINGUP) -I$(BRINGUP)/include -I$(GXDIR) -I$(DLDIR) \
              $(NJS6) -I$(DECOMP_ROOT)/include -I$(DECOMP_ROOT)/src -target $(LINK_TARGET)
L6DEPS := $(DECOMP_ROOT)/src/pso/ui/keyboard/SystemBootInit.cpp \
          $(DECOMP_ROOT)/src/pso/ui/TLogo.cpp $(DECOMP_ROOT)/src/pso/render/gjSystem.c

$(BUILD_DIR)/bringup/.ba-stamp: $(BRINGUP)/bringup.mk $(BRINGUP)/gen_ladder6_slices.py \
        $(BRINGUP)/ladder6_test.cpp $(BRINGUP)/include/ladder6_tex_prelude.h | $(OUT)
	@$(MAKE) --no-print-directory -C $(BRINGUP) -f bringup.mk all
	@touch $@

$(OUT)/gen: | $(OUT)
	@mkdir -p $@

$(OUT)/gen/ladder6_logo_glue.cpp $(OUT)/gen/ladder6_gx_glue.c: $(DLDIR)/gen_ladder6_logo.py \
        $(L6DEPS) $(DLDIR)/ladder6_logo_prelude.h $(DLDIR)/ladder6_gx_prelude.h | $(OUT)/gen
	@python3 $(DLDIR)/gen_ladder6_logo.py $(DECOMP_ROOT) \
	  $(OUT)/gen/ladder6_logo_glue.cpp $(OUT)/gen/ladder6_gx_glue.c

$(OUT)/gen/ladder6_logo_glue.o: $(OUT)/gen/ladder6_logo_glue.cpp | $(OUT)
	clang++ $(L6CXXFLAGS) -c -o $@ $<

$(OUT)/gen/ladder6_gx_glue.o: $(OUT)/gen/ladder6_gx_glue.c | $(OUT)
	clang $(CFLAGS) -I$(DLDIR) -c -o $@ $<

$(OUT)/gx_logo_driver.o: $(DLDIR)/gx_logo_driver.cpp $(DLDIR)/gx_logo_clut.h \
        $(BRINGUP)/include/ladder6_tex_prelude.h | $(OUT)
	clang++ $(L6CXXFLAGS) -c -o $@ $<

# P2-BB V5: AV-1 stand-in palette regime (same sources, one define) — the
# synthetic-fixture regime stays buildable+green alongside the era CLUT path.
$(OUT)/gx_logo_driver_av1.o: $(DLDIR)/gx_logo_driver.cpp $(DLDIR)/gx_logo_clut.h \
        $(BRINGUP)/include/ladder6_tex_prelude.h | $(OUT)
	clang++ $(L6CXXFLAGS) -DGX_LOGO_AV1 -c -o $@ $<

BRINGOBJS := $(BRINGUB)/crt0_nomain.o $(BRINGUB)/osinit.o $(BRINGUB)/osalloc_era.o $(BRINGUB)/dvd_shim.o \
             $(BRINGUB)/gen/move_data_glue.o $(BRINGUB)/gen/ladder4_gvr_glue.o $(BRINGUB)/ladder4_supply.o \
             $(BRINGUB)/gen/ladder5_gxtexture_glue.o $(BRINGUB)/gen/ladder5_tex_glue.o $(BRINGUB)/ladder5_seam.o \
             $(BRINGUB)/gen/ladder6_tex_consumer.o
LOGOBJS  := $(OUT)/gx_logo_driver.o $(OUT)/gen/ladder6_logo_glue.o $(OUT)/gen/ladder6_gx_glue.o \
            $(OUT)/gx_dl.o $(OUT)/gx_dl_metal.o

$(BIN)/gx_logo: $(LOGOBJS) $(BUILD_DIR)/bringup/.ba-stamp | $(BIN)
	clang -target $(LINK_TARGET) -o $@ $(LOGOBJS) $(BRINGOBJS) $(FRAMEWORKS)

$(BIN)/gx_logo_av1: $(OUT)/gx_logo_driver_av1.o $(OUT)/gen/ladder6_logo_glue.o \
        $(OUT)/gen/ladder6_gx_glue.o $(OUT)/gx_dl.o $(OUT)/gx_dl_metal.o \
        $(BUILD_DIR)/bringup/.ba-stamp | $(BIN)
	clang -target $(LINK_TARGET) -o $@ $(OUT)/gx_logo_driver_av1.o $(OUT)/gen/ladder6_logo_glue.o \
	  $(OUT)/gen/ladder6_gx_glue.o $(OUT)/gx_dl.o $(OUT)/gx_dl_metal.o $(BRINGOBJS) $(FRAMEWORKS)
# NOTE: gx_dl.o/gx_dl_metal.o/gx_dl_driver.o rules unchanged above; the
# shipped 4x4 gate path is byte-identical (GX_DL_TEXN unset there).

$(OUT) $(BIN):
	@mkdir -p $@

$(WAIVER_HDR): $(BUILD_DIR)/gen_waivers.py $(REPO)/data/assert-verdicts.tsv | $(OUT)
	@python3 $(BUILD_DIR)/gen_waivers.py --decomp $(DECOMP_ROOT) >/dev/null

# ---- Q's substrate (identical rules to gx_window.mk) ----
$(OUT)/crt0_nomain.o: $(BRINGUP)/crt0.c $(BRINGUP)/pso_bringup.h | $(OUT) $(WAIVER_HDR)
	clang $(CFLAGS) -DPSOB_TEST_BUILD -c -o $@ $<

$(OUT)/osinit.o: $(BRINGUP)/osinit.c $(BRINGUP)/pso_bringup.h | $(OUT) $(WAIVER_HDR)
	clang $(CFLAGS) -c -o $@ $<

# Q's OSAlloc-era entry points (HEAD osinit.c calls OSInitAlloc/OSCreateHeap;
# W's mk predates that split — we carry the extra object, bringup.mk:150 flags)
$(OUT)/osalloc_era.o: $(BRINGUP)/osalloc_era.c $(BRINGUP)/pso_bringup.h | $(OUT) $(WAIVER_HDR)
	clang $(CFLAGS) -c -o $@ $<

# ---- host/gx/dl port tree ----
$(OUT)/gx_dl_metal.o: $(DLDIR)/gx_dl_metal.m $(DLDIR)/gx_dl.h | $(OUT)
	clang $(OBJCFLAGS) -c -o $@ $<

$(OUT)/gx_dl.o $(OUT)/gx_dl_driver.o: $(OUT)/%.o: $(DLDIR)/%.c \
    $(DLDIR)/gx_dl.h $(GXDIR)/gx_era.h $(BRINGUP)/pso_bringup.h | $(OUT)
	clang $(HOSTCFLAGS) -c -o $@ $<

$(BIN)/gx_dl: $(OUT)/gx_dl_driver.o $(OUT)/gx_dl.o $(OUT)/gx_dl_metal.o \
              $(OUT)/crt0_nomain.o $(OUT)/osinit.o $(OUT)/osalloc_era.o | $(BIN)
	clang -target $(LINK_TARGET) -o $@ $^ $(FRAMEWORKS)

clean:
	@rm -rf $(OUT)
	@echo "cleaned $(OUT)"
