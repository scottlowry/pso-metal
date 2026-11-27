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
# control-flow proof (pixel proof = run_window.sh). No waiver header, no
# make against build/waivers (regen hazard rule).
SHELL := /bin/bash
REPO      := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/../..)
BUILD_DIR := $(REPO)/build
GXDIR     := $(REPO)/host/gx
BRINGUP   := $(REPO)/host/bringup
DECOMP_ROOT ?= $(HOME)/Hermes/pso-decomp/pso-decomp
DECOMP_SHA  := 6b38b5383

LINK_TARGET ?= arm64-apple-macos12.0
OUT := $(BUILD_DIR)/vi_branch
BIN := $(OUT)/bin
SLICE := $(OUT)/vi_era_slice.cpp

HOSTCFLAGS := -std=c11 -g -Wall -Wextra -DPSO_PORT_LAYOUT_WAIVER -D_DARWIN_C_SOURCE \
              -I$(BRINGUP) -I$(BRINGUP)/include -I$(GXDIR) -target $(LINK_TARGET)
# slices + driver are C++ TU set (era classes are C++); driver compiles the
# .c entry as C++ (-x c++) to keep the lane's enumerated filename.
CXXFLAGS := -std=c++17 -g -Wall -Wextra -DPSO_PORT_LAYOUT_WAIVER -D_DARWIN_C_SOURCE \
            -I$(BRINGUP)/include -I$(GXDIR) -target $(LINK_TARGET)

.PHONY: all clean
all: $(BIN)/vi_branch $(BIN)/vi_branch_flagdtv

$(OUT) $(BIN):
	@mkdir -p $@

# ---- verbatim era slice (manifest-hashed; fails on decomp drift) ----
$(SLICE): $(GXDIR)/vi_slice_gen.py $(GXDIR)/vi_slice_manifest.txt $(GXDIR)/vi_era_harness.h | $(OUT)
	@DECOMP_ROOT=$(DECOMP_ROOT) DECOMP_SHA=$(DECOMP_SHA) python3 $(GXDIR)/vi_slice_gen.py $@

$(OUT)/vi_era_slice.o: $(SLICE) $(GXDIR)/gx_era.h | $(BIN)
	clang++ $(CXXFLAGS) -c -o $@ $<

$(OUT)/vi_branch_driver.o: $(GXDIR)/vi_branch_driver.c $(GXDIR)/vi_era_harness.h \
                           $(GXDIR)/gx_vi.h $(GXDIR)/gx_era.h | $(BIN)
	clang++ $(CXXFLAGS) -x c++ -c -o $@ $<

$(OUT)/gx_vi.o: $(GXDIR)/gx_vi.c $(GXDIR)/gx_vi.h $(GXDIR)/gx_era.h | $(BIN)
	clang $(HOSTCFLAGS) -c -o $@ $<

# default binary: runtime/argv-selectable outcomes (compile flags default 0)
$(BIN)/vi_branch: $(OUT)/vi_branch_driver.o $(OUT)/vi_era_slice.o $(OUT)/gx_vi.o | $(BIN)
	clang++ -target $(LINK_TARGET) -o $@ $^

# compile-flag binary: -DGX_VI_DTV_OUTCOME=1 -DGX_VI_PROG_OUTCOME=1 exercises
# the other selection axis (task: "compile flag or argv").
$(OUT)/gx_vi_flagdtv.o: $(GXDIR)/gx_vi.c $(GXDIR)/gx_vi.h $(GXDIR)/gx_era.h | $(BIN)
	clang $(HOSTCFLAGS) -DGX_VI_DTV_OUTCOME=1 -DGX_VI_PROG_OUTCOME=1 -c -o $@ $<

$(BIN)/vi_branch_flagdtv: $(OUT)/vi_branch_driver.o $(OUT)/vi_era_slice.o $(OUT)/gx_vi_flagdtv.o | $(BIN)
	clang++ -target $(LINK_TARGET) -o $@ $^

clean:
	@rm -rf $(OUT)
	@echo "cleaned $(OUT)"
