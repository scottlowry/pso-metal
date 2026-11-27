# pso-metal P2-W GX→Metal window build fragment — extends bringup.mk's flag
# conventions (docs/p1-buildsystem.md). One-command entry: host/gx/run_window.sh
# Links Q's substrate (crt0 arena + mini OSInit, compiled identically to
# bringup.mk — same CFLAGS, waiver force-include) + host/gx's Metal backing.
#
#   make -f host/gx/gx_window.mk            # build the windowed gate binary
#   make -f host/gx/gx_window.mk clean
#
SHELL := /bin/bash
REPO      := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/../..)
BUILD_DIR := $(REPO)/build
GXDIR     := $(REPO)/host/gx
BRINGUP   := $(REPO)/host/bringup
DECOMP_ROOT ?= $(HOME)/Hermes/pso-decomp/pso-decomp
COMPAT    := $(REPO)/compat/pso_metal_compat.h
WAIVER_HDR  := $(BUILD_DIR)/waivers/pso_assert_waivers.h

LINK_TARGET ?= arm64-apple-macos12.0
OUT := $(BUILD_DIR)/gxwindow
BIN := $(OUT)/bin

SDK    := $(shell xcrun --sdk macosx --show-sdk-path)
# Same COMMON as bringup.mk (waiver + compat force-includes; D4/D5 flags) —
# substrate TUs must compile exactly as they do for boot_spine.
COMMON := -w -fms-extensions -Wno-c++11-narrowing \
          -DPSO_PORT_LAYOUT_WAIVER -D_DARWIN_C_SOURCE \
          -include $(COMPAT) -include $(WAIVER_HDR)
CFLAGS := -std=c11 -g $(COMMON) -I$(BRINGUP)/include -I$(GXDIR) -target $(LINK_TARGET)
# host/gx's own TUs are port code: no era waiver force-include needed, but
# bringup's types.h fail-closed asserts on -DPSO_PORT_LAYOUT_WAIVER (parsing
# era-style) and pso_bringup.h lives in host/bringup — both flags follow.
HOSTCFLAGS := -std=c11 -g -Wall -Wextra -DPSO_PORT_LAYOUT_WAIVER -D_DARWIN_C_SOURCE \
              -I$(BRINGUP) -I$(BRINGUP)/include -I$(GXDIR) -target $(LINK_TARGET)
OBJCFLAGS := -fobjc-arc -g -Wall -D_DARWIN_C_SOURCE -I$(GXDIR) \
             -isysroot $(SDK) -target $(LINK_TARGET)
FRAMEWORKS := -framework Foundation -framework AppKit -framework Metal -framework QuartzCore

.PHONY: all clean
all: $(BIN)/gx_window

$(OUT) $(BIN):
	@mkdir -p $@

# Waiver header: regenerate exactly like bringup.mk (shared artifact — the
# bringup stamp may already have built it; the rule is idempotent).
$(WAIVER_HDR): $(BUILD_DIR)/gen_waivers.py $(REPO)/data/assert-verdicts.tsv | $(OUT)
	@python3 $(BUILD_DIR)/gen_waivers.py --decomp $(DECOMP_ROOT) >/dev/null

# ---- Q's substrate (same sources, same flags as bringup.mk) ----
$(OUT)/crt0_nomain.o: $(BRINGUP)/crt0.c $(BRINGUP)/pso_bringup.h | $(OUT) $(WAIVER_HDR)
	clang $(CFLAGS) -DPSOB_TEST_BUILD -c -o $@ $<

$(OUT)/osinit.o: $(BRINGUP)/osinit.c $(BRINGUP)/pso_bringup.h | $(OUT) $(WAIVER_HDR)
	clang $(CFLAGS) -c -o $@ $<

# ---- host/gx port tree ----
$(OUT)/gx_metal.o: $(GXDIR)/gx_metal.m $(GXDIR)/gx_metal.h | $(OUT)
	clang $(OBJCFLAGS) -c -o $@ $<

$(OUT)/gx_shims.o $(OUT)/gx_window_driver.o: $(OUT)/%.o: $(GXDIR)/%.c \
    $(GXDIR)/gx_era.h $(GXDIR)/gx_metal.h $(BRINGUP)/pso_bringup.h | $(OUT)
	clang $(HOSTCFLAGS) -c -o $@ $<

$(BIN)/gx_window: $(OUT)/gx_window_driver.o $(OUT)/gx_shims.o $(OUT)/gx_metal.o \
                  $(OUT)/crt0_nomain.o $(OUT)/osinit.o | $(BIN)
	clang -target $(LINK_TARGET) -o $@ $^ $(FRAMEWORKS)

clean:
	@rm -rf $(OUT)
	@echo "cleaned $(OUT)"
