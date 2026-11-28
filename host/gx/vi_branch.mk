# pso-metal P2-AB VI-branch build fragment — extends gx_window.mk's flag
# conventions (docs/p1-buildsystem.md) WITHOUT touching it. One-command entry:
# host/gx/run_vi.sh
#
#   make -f host/gx/vi_branch.mk            # generate slice + build both gates
#   make -f host/gx/vi_branch.mk clean
#
# Deliberate links: W's gx_era.h (the compiling era GX/VI decl set) + Q's
# types.h shadow + W's flag conventions. Deliberate NON-link: gx_shims.o —
# gx_shims.c:156 defines VIGetDTVStatus->0 (the W1 contract itself) and would
# duplicate-symbol against the selectable gx_vi.o; the DTV=0 scenario here
# proves that same contract dynamically. No window/Metal needed: this is a
# control-flow proof (pixel proof = run_window.sh).
# BRINGUP7 step-2 (R-D retirement): the binaries now link $(INPUT_OBJS) — the
# host/input seat owns the ONLY ControllerInputState/bank definitions, and the
# slice TUs compile the REAL era headers through the same pin-include regime
# as input.mk (era types + compat + waiver header are -include'd, NEVER
# regenerated: the seat's D8 verify-only stamp guards the sha each build).
SHELL := /bin/bash
REPO      := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/../..)
BUILD_DIR := $(REPO)/build
GXDIR     := $(REPO)/host/gx
BRINGUP   := $(REPO)/host/bringup
DECOMP_ROOT ?= $(HOME)/Hermes/pso-decomp/pso-decomp-port-pin
DECOMP_SHA  := 6b38b5383

LINK_TARGET ?= arm64-apple-macos12.0
OUT := $(BUILD_DIR)/vi_branch
BIN := $(OUT)/bin
SLICE := $(OUT)/vi_era_slice.cpp

INPUT      := $(REPO)/host/input
INPUT_OUT  := $(BUILD_DIR)/input
INPUT_OBJS := $(INPUT_OUT)/input_state.o $(INPUT_OUT)/gen/input_pump_glue.o
COMPAT     := $(REPO)/compat/pso_metal_compat.h
WAIVER_HDR := $(BUILD_DIR)/waivers/pso_assert_waivers.h

HOSTCFLAGS := -std=c11 -g -Wall -Wextra -DPSO_PORT_LAYOUT_WAIVER -D_DARWIN_C_SOURCE \
              -I$(BRINGUP) -I$(BRINGUP)/include -I$(GXDIR) -target $(LINK_TARGET)
# slices + driver are C++ TU set (era classes are C++); driver compiles the
# .c entry as C++ (-x c++) to keep the lane's enumerated filename.
# Step-2: seat include regime added — host/input/include FIRST (PADStatus-only
# dolphin shim), era include/src pair, compat+waiver -include pair,
# -fms-extensions -Wno-c++11-narrowing: mirrors input.mk COMMON/CXXFLAGS.
CXXFLAGS := -std=c++17 -g -Wall -Wextra -fms-extensions -Wno-c++11-narrowing \
            -DPSO_PORT_LAYOUT_WAIVER -D_DARWIN_C_SOURCE \
            -include $(COMPAT) -include $(WAIVER_HDR) \
            -I$(INPUT)/include -I$(BRINGUP)/include -I$(GXDIR) \
            -I$(DECOMP_ROOT)/include -I$(DECOMP_ROOT)/src -target $(LINK_TARGET)

.PHONY: all clean
all: $(BIN)/vi_branch $(BIN)/vi_branch_flagdtv

$(OUT) $(BIN):
	@mkdir -p $@

# ---- seat objects (step-2): built by host/input/input.mk (its waiver.stamp
# is VERIFY-only — sha compared, zero regen; gen_waivers/make parse untouched)
$(INPUT_OBJS):
	@$(MAKE) --no-print-directory -C $(INPUT) -f input.mk $@

# ---- verbatim era slice (manifest-hashed; fails on decomp drift) ----
$(SLICE): $(GXDIR)/vi_slice_gen.py $(GXDIR)/vi_slice_manifest.txt $(GXDIR)/vi_era_harness.h | $(OUT)
	@DECOMP_ROOT=$(DECOMP_ROOT) DECOMP_SHA=$(DECOMP_SHA) python3 $(GXDIR)/vi_slice_gen.py $@

$(OUT)/vi_era_slice.o: $(SLICE) $(GXDIR)/gx_era.h $(INPUT)/include/pso_input.h | $(BIN)
	clang++ $(CXXFLAGS) -c -o $@ $<

$(OUT)/vi_branch_driver.o: $(GXDIR)/vi_branch_driver.c $(GXDIR)/vi_era_harness.h \
                           $(GXDIR)/gx_vi.h $(GXDIR)/gx_era.h $(INPUT)/include/pso_input.h | $(BIN)
	clang++ $(CXXFLAGS) -x c++ -c -o $@ $<

$(OUT)/gx_vi.o: $(GXDIR)/gx_vi.c $(GXDIR)/gx_vi.h $(GXDIR)/gx_era.h | $(BIN)
	clang $(HOSTCFLAGS) -c -o $@ $<

# default binary: runtime/argv-selectable outcomes (compile flags default 0)
$(BIN)/vi_branch: $(OUT)/vi_branch_driver.o $(OUT)/vi_era_slice.o $(OUT)/gx_vi.o \
                  $(INPUT_OBJS) | $(BIN)
	clang++ -target $(LINK_TARGET) -o $@ $^ -lm

# compile-flag binary: -DGX_VI_DTV_OUTCOME=1 -DGX_VI_PROG_OUTCOME=1 exercises
# the other selection axis (task: "compile flag or argv").
$(OUT)/gx_vi_flagdtv.o: $(GXDIR)/gx_vi.c $(GXDIR)/gx_vi.h $(GXDIR)/gx_era.h | $(BIN)
	clang $(HOSTCFLAGS) -DGX_VI_DTV_OUTCOME=1 -DGX_VI_PROG_OUTCOME=1 -c -o $@ $<

$(BIN)/vi_branch_flagdtv: $(OUT)/vi_branch_driver.o $(OUT)/vi_era_slice.o $(OUT)/gx_vi_flagdtv.o \
                          $(INPUT_OBJS) | $(BIN)
	clang++ -target $(LINK_TARGET) -o $@ $^ -lm

clean:
	@rm -rf $(OUT)
	@echo "cleaned $(OUT)"
