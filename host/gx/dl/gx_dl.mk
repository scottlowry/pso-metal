# pso-metal P2-Z display-list slice build — mirrors host/gx/gx_window.mk
# (same substrate objects, same flag conventions), OUT=build/gxdl.
#   make -f host/gx/dl/gx_dl.mk all | clean
SHELL := /bin/bash
REPO      := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/../../..)
BUILD_DIR := $(REPO)/build
GXDIR     := $(REPO)/host/gx
DLDIR     := $(GXDIR)/dl
BRINGUP   := $(REPO)/host/bringup
DECOMP_ROOT ?= $(HOME)/Hermes/pso-decomp/pso-decomp
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

.PHONY: all clean
all: $(BIN)/gx_dl

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
