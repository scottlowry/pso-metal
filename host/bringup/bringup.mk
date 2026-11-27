# pso-metal P2-Q bringup build fragment — do NOT edit build/Makefile; this
# includes/extends its conventions (docs/p1-buildsystem.md, p1-init-ledger §4
# step 1). One-command entry: host/bringup/run.sh
#
#   make -f host/bringup/bringup.mk            # build both binaries
#   make -f host/bringup/bringup.mk clean
#
SHELL := /bin/bash
REPO        := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/../..)
BUILD_DIR   := $(REPO)/build
BRINGUP     := $(REPO)/host/bringup
DECOMP_ROOT ?= $(HOME)/Hermes/pso-decomp/pso-decomp
COMPAT      := $(REPO)/compat/pso_metal_compat.h
WAIVER_HDR  := $(BUILD_DIR)/waivers/pso_assert_waivers.h
PRS_SRC     := $(DECOMP_ROOT)/src/pso/system/prs.cpp

# LINK_TARGET = the actual execution triple (build/Makefile's convention).
LINK_TARGET ?= arm64-apple-macos12.0
OUT         := $(BUILD_DIR)/bringup
GEN         := $(OUT)/gen
BIN         := $(OUT)/bin

# Step 1 flag set: same COMMON as build/Makefile, plus the shadow -I FIRST
# (assert-policy §4 mechanism), -DPSO_PORT_LAYOUT_WAIVER, and the generated
# waiver list force-included via pso_assert_waivers.h (its PSO_LA_FLAG_* names
# ARE the generated list — build/waivers is regenerated, never hand-edited).
# Deviation D4 (docs/p2-bringup-1.md): the era clib/ + libs/dolphin/include/
# trees shadow host libc headers (dolphin/string.h, libc/math.h, PPC
# uintptr_t …). EXECUTING TUs must bind to the real host libc, so the bringup
# include set drops -I clib and both -I libs/dolphin paths (dolphin.h types
# are era-PPC; no unit in this scope includes them). Parse builds keep the
# full set — this split is exactly the parse-vs-exec separation.
SDK_INCS_C   := -I$(BRINGUP)/include
SDK_INCS_CXX := -I$(BRINGUP)/include -I$(DECOMP_ROOT)/include \
                -I$(DECOMP_ROOT)/src
# Deviation D5 (docs/p2-bringup-1.md): -ffreestanding is DROPPED here vs
# build/Makefile's COMMON (it was parse-only, for era 'void main(void)').
# With -ffreestanding clang treats C++ main as an ordinary function and
# MANGLAMES it (__Z4mainv) — no linker entry point. Bringup TUs are hosted.
COMMON := -w -fms-extensions -Wno-c++11-narrowing \
          -DPSO_PORT_LAYOUT_WAIVER -D_DARWIN_C_SOURCE \
          -include $(COMPAT) -include $(WAIVER_HDR)
CFLAGS   := -std=c11   -g $(COMMON) $(SDK_INCS_C) -target $(LINK_TARGET)
CXXFLAGS := -std=c++17 -g $(COMMON) $(SDK_INCS_CXX) -target $(LINK_TARGET)

.PHONY: all clean check
all: $(BIN)/boot_spine $(BIN)/prs_golden_test

$(OUT) $(GEN) $(BIN):
	@mkdir -p $@

# Regenerate the waiver header exactly like build/Makefile's `waivers` target
# (fail-closed: deny-list enforced inside gen_waivers.py).
$(OUT)/waiver.stamp: $(BUILD_DIR)/gen_waivers.py $(REPO)/data/assert-verdicts.tsv | $(OUT)
	@python3 $(BUILD_DIR)/gen_waivers.py --decomp $(DECOMP_ROOT) >/dev/null
	@touch $@

# --- deviation D3 (docs/p2-bringup-1.md): macOS refuses fixed mappings at
# 0x80000000 (probe: mmap+mach_vm_allocate both fail there), so the era
# window = (u8*)0x80001800 literal is rebased to the arena-window global in a
# GENERATED copy. Guards prove the copy differs from the read-only decomp
# source in that single literal and nothing else.
$(GEN)/prs_glue.cpp: $(PRS_SRC) $(BRINGUP)/include/prs_window_glue.h | $(GEN) $(OUT)/waiver.stamp
	@n=$$(grep -c '0x80001800' $(PRS_SRC)); \
	 [ "$$n" = 1 ] || { echo "GUARD: $(PRS_SRC) has $$n window literals (expected exactly 1) — stop"; exit 1; }
	@sed 's/0x80001800/pso_prs_window_base/' $(PRS_SRC) > $@
	@sed 's/pso_prs_window_base/0x80001800/' $@ > $@.roundtrip
	@diff -q $@.roundtrip $(PRS_SRC) >/dev/null || \
	  { echo "GUARD: glue copy drifted beyond the single window literal"; rm -f $@; exit 1; }
	@rm -f $@.roundtrip
	@echo "gen: prs_glue.cpp (era prs.cpp + 1 window-literal rebase, guarded)"

$(GEN)/prs_glue.o: $(GEN)/prs_glue.cpp | $(OUT)
	clang++ $(CXXFLAGS) -include $(BRINGUP)/include/prs_window_glue.h -c -o $@ $<

$(OUT)/prs_support.o: $(BRINGUP)/prs_support.cpp $(BRINGUP)/pso_bringup.h | $(OUT) $(GEN)
	clang++ $(CXXFLAGS) -c -o $@ $<

$(OUT)/osinit.o $(OUT)/dvd_shim.o: $(OUT)/%.o: $(BRINGUP)/%.c $(BRINGUP)/pso_bringup.h | $(OUT) $(OUT)/waiver.stamp
	clang $(CFLAGS) -c -o $@ $<

$(OUT)/crt0_main.o: $(BRINGUP)/crt0.c | $(OUT) $(OUT)/waiver.stamp
	clang $(CFLAGS) -c -o $@ $<
$(OUT)/crt0_nomain.o: $(BRINGUP)/crt0.c | $(OUT) $(OUT)/waiver.stamp
	clang $(CFLAGS) -DPSOB_TEST_BUILD -c -o $@ $<

$(OUT)/prs_golden_test.o: $(BRINGUP)/prs_golden_test.cpp | $(OUT) $(OUT)/waiver.stamp
	clang++ $(CXXFLAGS) -c -o $@ $<

$(BIN)/boot_spine: $(OUT)/crt0_main.o $(OUT)/osinit.o $(OUT)/dvd_shim.o $(GEN)/prs_glue.o $(OUT)/prs_support.o | $(BIN)
	clang++ -target $(LINK_TARGET) -o $@ $^

$(BIN)/prs_golden_test: $(OUT)/crt0_nomain.o $(OUT)/osinit.o $(OUT)/dvd_shim.o $(GEN)/prs_glue.o $(OUT)/prs_support.o $(OUT)/prs_golden_test.o | $(BIN)
	clang++ -target $(LINK_TARGET) -o $@ $^

clean:
	@rm -rf $(OUT)
	@echo "cleaned $(OUT)"
