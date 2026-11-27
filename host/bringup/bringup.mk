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
# P2-Y targets (docs/p2-bringup-2.md §1 inventory):
FADE_SRC    := $(DECOMP_ROOT)/src/pso/system/TScreenFadeController.cpp
MEMORY_SRC  := $(DECOMP_ROOT)/src/pso/system/memory.cpp
LOBBY_SRC   := $(DECOMP_ROOT)/src/pso/unk/TLobby_803DE778.cpp

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
all: $(BIN)/boot_spine $(BIN)/prs_golden_test $(BIN)/ladder_test

$(OUT) $(GEN) $(BIN):
	@mkdir -p $@

# Waiver discipline (P2-Y adjustment, deviation D8): build/** is integrator-
# owned with a staged waiver-stitch pending — gen_waivers.py must NOT run
# from this lane. The committed build/waivers/pso_assert_waivers.h (99
# ILP32-targeted LAYOUT_ASSERT waivers + keep-flags) is force-included
# AS-IS via COMMON -include; this stamp now only VERIFIED-gates it
# (fail-closed if the committed header or its 99-waive state is missing).
$(OUT)/waiver.stamp: $(REPO)/build/waivers/pso_assert_waivers.h | $(OUT)
	@grep -q 'STATE: 99 waived' $(BUILD_DIR)/waivers/pso_assert_waivers.h || \
	 { echo "GUARD: committed waiver header missing/changed (expected 99 waived) — stop"; exit 1; }
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

$(BIN)/boot_spine: $(OUT)/crt0_main.o $(OUT)/osinit.o $(OUT)/osalloc_era.o $(OUT)/dvd_shim.o $(GEN)/prs_glue.o $(OUT)/prs_support.o | $(BIN)
	clang++ -target $(LINK_TARGET) -o $@ $^

$(BIN)/prs_golden_test: $(OUT)/crt0_nomain.o $(OUT)/osinit.o $(OUT)/osalloc_era.o $(OUT)/dvd_shim.o $(GEN)/prs_glue.o $(OUT)/prs_support.o $(OUT)/prs_golden_test.o | $(BIN)
	clang++ -target $(LINK_TARGET) -o $@ $^

# --- P2-Y: era PRS wrappers + era heap machinery -------------------------
# memory.cpp compiles VERBATIM from the read-only decomp tree: its only
# externals are the OSAlloc entry points (osalloc_era.c port), the
# AllocatorCallbacks/memory.h decomp headers, and move_data (sliced below).
# No literal rebase needed — it touches no era absolute addresses.
$(OUT)/memory.o: $(MEMORY_SRC) | $(OUT) $(OUT)/waiver.stamp
	clang++ $(CXXFLAGS) -c -o $@ $<

# move_data: TLobby_803DE778.cpp is a 4k-line lobby unit; only the 3-line
# memmove wrapper memory.cpp:73 needs. Generated line-range-slice, verbatim
# (guard: signature line, body line and '}' must match exactly at $N..$N+2).
$(GEN)/move_data_glue.cpp: $(LOBBY_SRC) | $(GEN) $(OUT)/waiver.stamp
	@n=$$(grep -n '^void move_data(void \*dst, const void \*src, s32 n) {' $(LOBBY_SRC) | cut -d: -f1); \
	 c=$$(grep -c '^void move_data(void \*dst, const void \*src, s32 n) {' $(LOBBY_SRC)); \
	 [ "$$c" = 1 ] || { echo "GUARD: move_data sig count $$c (expected 1) — stop"; exit 1; }; \
	 sed -n "$${n},$$((n+2))p" $(LOBBY_SRC) > $@.body; \
	 sed -n "2p" $@.body | grep -qx '    memmove(dst, src, n);' || \
	  { echo "GUARD: move_data body drifted from memmove — stop"; exit 1; }; \
	 sed -n "3p" $@.body | grep -qx '}' || \
	  { echo "GUARD: move_data slice end drifted — stop"; exit 1; }; \
	 { printf '#include "types.h"\n#include <string.h>\n/* P2-Y verbatim slice: $(LOBBY_SRC) lines %s..%s (move_data only)\n */\n' "$$n" "$$((n+2))"; cat $@.body; } > $@; \
	 rm -f $@.body; \
	 echo "gen: move_data_glue.cpp (verbatim lines $$n..$$((n+2)) of TLobby_803DE778.cpp)"

# prs_decompress_buffer + prs_decompress_alloc: the two C-callable wrappers
# live at TScreenFadeController.cpp:309/:321 (units home: 0x800F7DAC/
# 0x800F7D30 in era .s). Whole-file compile would drag the GX quad/camera
# deps, so generate a line-range VERBATIM slice, anchored by exact signature
# lines (guard: 1 match each; slice must start/end with the traced braces).
$(GEN)/prs_fade_glue.cpp: $(FADE_SRC) | $(GEN) $(OUT)/waiver.stamp
	@n1=$$(grep -n '^u32 prs_decompress_buffer(const void \*src, void \*dst) {' $(FADE_SRC) | cut -d: -f1); \
	 n2=$$(grep -n '^s32 prs_decompress_alloc(const void \*src, void \*dst, s32 size_limit) {' $(FADE_SRC) | cut -d: -f1); \
	 [ "$$(grep -c '^u32 prs_decompress_buffer(const void \*src, void \*dst) {' $(FADE_SRC))" = 1 ] && \
	 [ "$$(grep -c '^s32 prs_decompress_alloc(const void \*src, void \*dst, s32 size_limit) {' $(FADE_SRC))" = 1 ] && \
	 [ -n "$$n1" ] && [ -n "$$n2" ] && [ "$$n2" -gt "$$n1" ] || \
	  { echo "GUARD: TScreenFadeController.cpp wrapper anchors drifted — stop"; exit 1; }; \
	 end=$$(awk -v n="$$n2" 'NR>n && /^}$$/ {print NR; exit}' $(FADE_SRC)); \
	 [ -n "$$end" ] || { echo "GUARD: no closing brace after alloc — stop"; exit 1; }; \
	 sed -n "$${n1},$${end}p" $(FADE_SRC) > $@.body; \
	 [ "$$(grep -c '^}$$' $@.body)" = 2 ] || \
	  { echo "GUARD: fade slice spans more than the two wrappers — stop"; exit 1; }; \
	 { printf '#include "types.h"\n#include "pso/system/prs.h"\n/* P2-Y verbatim slice: $(FADE_SRC) lines %s..%s (prs_decompress_buffer + prs_decompress_alloc only)\n */\n' "$$n1" "$$end"; cat $@.body; } > $@; \
	 rm -f $@.body; \
	 echo "gen: prs_fade_glue.cpp (verbatim lines $$n1..$$end of TScreenFadeController.cpp)"

$(OUT)/osalloc_era.o: $(BRINGUP)/osalloc_era.c $(BRINGUP)/pso_bringup.h | $(OUT) $(OUT)/waiver.stamp
	clang $(CFLAGS) -c -o $@ $<

$(GEN)/move_data_glue.o: $(GEN)/move_data_glue.cpp | $(OUT)
	clang++ $(CXXFLAGS) -c -o $@ $<
$(GEN)/prs_fade_glue.o: $(GEN)/prs_fade_glue.cpp | $(OUT) $(GEN)
	clang++ $(CXXFLAGS) -c -o $@ $<

$(OUT)/ladder_test.o: $(BRINGUP)/ladder_test.cpp | $(OUT) $(OUT)/waiver.stamp
	clang++ $(CXXFLAGS) -c -o $@ $<

$(BIN)/ladder_test: $(OUT)/crt0_nomain.o $(OUT)/osinit.o $(OUT)/osalloc_era.o $(OUT)/dvd_shim.o \
                    $(GEN)/prs_glue.o $(OUT)/prs_support.o $(GEN)/prs_fade_glue.o $(GEN)/move_data_glue.o \
                    $(OUT)/memory.o $(OUT)/ladder_test.o | $(BIN)
	clang++ -target $(LINK_TARGET) -o $@ $^

clean:
	@rm -rf $(OUT)
	@echo "cleaned $(OUT)"
