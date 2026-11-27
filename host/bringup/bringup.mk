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
# (fail-closed if the committed header changed from the integrator-approved sha).
$(OUT)/waiver.stamp: $(REPO)/build/waivers/pso_assert_waivers.h | $(OUT)
	@test "$$(shasum -a 256 $(BUILD_DIR)/waivers/pso_assert_waivers.h | cut -d' ' -f1)" = \
	  "$$(cat $(BRINGUP)/waiver-header.sha256)" || \
	 { echo "GUARD: waiver header sha != approved (host/bringup/waiver-header.sha256) — stop"; exit 1; }
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

# --- P2-AE boot ladder rung 3 ---------------------------------------------
# Three generated VERBATIM slice TUs (same guard style as the P2-Y glues):
#   logo : TLogo.cpp predicates (:57-63) + ctor/dtor/run_task/render
#          (:126-199) + SystemBootInit.cpp request_main_phase/
#          reset_main_phase_state
#   quest: QuestOpcodeHandlers.cpp qst_op_call_image_data (:2278-2286)
#   gsl  : TGStream.cpp find_gsl_entry (:207-219)
# Prelude headers in include/ carry every external the slices reference,
# each with its era .s citation (docs/p2-bringup-3.md owns the deviation
# roster DA-1..DA-5).
TLOGO_SRC := $(DECOMP_ROOT)/src/pso/ui/TLogo.cpp
SBI_SRC   := $(DECOMP_ROOT)/src/pso/ui/keyboard/SystemBootInit.cpp
QUEST_SRC := $(DECOMP_ROOT)/src/pso/quest/QuestOpcodeHandlers.cpp
TGS_SRC   := $(DECOMP_ROOT)/src/pso/system/TGStream.cpp

# awk block: from the exact sig line through the first column-0 '}'.
# guard() = block non-empty AND the two/three era body lines we hand-traced
# from the .s are present AND nothing beyond the anchor leaked in.
$(GEN)/ladder3_logo_glue.cpp: $(TLOGO_SRC) $(SBI_SRC) $(BRINGUP)/include/ladder3_logo_prelude.h | $(GEN) $(OUT)/waiver.stamp
	@{ printf '#include "ladder3_logo_prelude.h"\n'; \
	   awk '/^int is_logo_clear_flag_set\(void\)/{f=1} /^void TLogo::init_logo_scene/{f=0} f' $(TLOGO_SRC); \
	   awk '/^TLogo::TLogo\(TObject \*parent\)/{f=1} /^s32 CursorAction::get/{f=0} f' $(TLOGO_SRC); \
	   awk '/^void request_main_phase\(s32 phase\) \{/{f=1} f{print} f&&/^\}$$/{exit}' $(SBI_SRC); \
	   awk '/^void reset_main_phase_state\(void\) \{/{f=1} f{print} f&&/^\}$$/{exit}' $(SBI_SRC); \
	 } > $@.body
	@for s in 'is_vi_config_answered' 'is_sega_logo_done' 'is_logo_exit_requested' 'is_adx_logo_done' \
	          'switch (state)' 'request_main_phase(0xA)' 'void TLogo::render() {}' \
	          'main_phase_state.next = phase'; do \
	   grep -qF "$$s" $@.body || { echo "GUARD: ladder3 logo slice missing '$$s' — stop"; rm -f $@.body; exit 1; }; \
	 done; \
	 grep -qF 'init_logo_scene' $@.body && { echo "GUARD: ladder3 logo slice leaked past anchor — stop"; rm -f $@.body; exit 1; } || true
	@mv $@.body $@; echo "gen: ladder3_logo_glue.cpp (TLogo predicates+machine, request/reset_main_phase — verbatim)"

$(GEN)/ladder3_quest_glue.cpp: $(QUEST_SRC) $(BRINGUP)/include/ladder3_quest_prelude.h | $(GEN) $(OUT)/waiver.stamp
	@{ printf '#include "ladder3_quest_prelude.h"\n'; \
	   awk '/^void qst_op_call_image_data\(\) \{/{f=1} f{print} f&&/^\}$$/{exit}' $(QUEST_SRC); \
	 } > $@.body
	@grep -qF 'prs_decompress_buffer(source_data, resource_decompress_buffer);' $@.body && \
	 grep -qF 'load_sin_board_image(resource_decompress_buffer, size);' $@.body || \
	 { echo "GUARD: ladder3 quest slice body drifted — stop"; rm -f $@.body; exit 1; }
	@mv $@.body $@; echo "gen: ladder3_quest_glue.cpp (qst_op_call_image_data — verbatim)"

$(GEN)/ladder3_gsl_glue.cpp: $(TGS_SRC) $(BRINGUP)/include/ladder3_gsl_prelude.h | $(GEN) $(OUT)/waiver.stamp
	@{ printf '#include "ladder3_gsl_prelude.h"\n'; \
	   awk '/^GslArchiveEntry \*find_gsl_entry\(const char \*name, s32 unused\) \{/{f=1} f{print} f&&/^\}$$/{exit}' $(TGS_SRC); \
	 } > $@.body
	@grep -qF 'strcmp(e->name, name)' $@.body && grep -qF 'gsl_adxf_handle == 0' $@.body || \
	 { echo "GUARD: ladder3 gsl slice body drifted — stop"; rm -f $@.body; exit 1; }
	@mv $@.body $@; echo "gen: ladder3_gsl_glue.cpp (find_gsl_entry — verbatim)"

$(GEN)/ladder3_logo_glue.o $(GEN)/ladder3_quest_glue.o $(GEN)/ladder3_gsl_glue.o: $(GEN)/%.o: $(GEN)/%.cpp $(BRINGUP)/include/ladder3_harness.h $(BRINGUP)/include/ladder3_logo_prelude.h $(BRINGUP)/include/ladder3_quest_prelude.h $(BRINGUP)/include/ladder3_gsl_prelude.h | $(OUT)
	clang++ $(CXXFLAGS) -c -o $@ $<

$(OUT)/ladder3_test.o: $(BRINGUP)/ladder3_test.cpp $(BRINGUP)/include/ladder3_harness.h | $(OUT) $(OUT)/waiver.stamp
	clang++ $(CXXFLAGS) -c -o $@ $<

$(BIN)/ladder3_test: $(OUT)/crt0_nomain.o $(OUT)/osinit.o $(OUT)/osalloc_era.o $(OUT)/dvd_shim.o \
                     $(GEN)/prs_glue.o $(OUT)/prs_support.o $(GEN)/prs_fade_glue.o $(GEN)/move_data_glue.o \
                     $(OUT)/memory.o \
                     $(GEN)/ladder3_logo_glue.o $(GEN)/ladder3_quest_glue.o $(GEN)/ladder3_gsl_glue.o \
                     $(OUT)/ladder3_test.o | $(BIN)
	clang++ -target $(LINK_TARGET) -o $@ $^

all: $(BIN)/ladder3_test

# --- P2-AN boot ladder rung 4: texture supply chain ------------------------
# One new generated VERBATIM slice (find_gvrt_texture_chunk from
# text_803D96A8.cpp:160-186, home 0x803D9C98) + the prelude-level identity
# seam for its swap call site (deviation AN-1, docs/p2-tex-supply.md; the
# era swap body must NOT enter this TU — guarded below). Reuses AE's
# ladder3_gsl_glue.o (find_gsl_entry + era GSL globals) and Y's
# move_data_glue.o, so the chain disc-table -> entry -> GVR walk -> era
# byte-mover is era code end to end.
GVR_SRC := $(DECOMP_ROOT)/src/pso/unk/text_803D96A8.cpp

$(GEN)/ladder4_gvr_glue.cpp: $(GVR_SRC) $(BRINGUP)/include/ladder4_gvr_prelude.h | $(GEN) $(OUT)/waiver.stamp
	@{ printf '#include "ladder4_gvr_prelude.h"\n'; \
	   awk '/^u32 \*find_gvrt_texture_chunk\(TLobby \*lobby, u32 \*out_id, u32 \*data\) \{/{f=1} f{print} f&&/^\}$$/{exit}' $(GVR_SRC); \
	 } > $@.body
	@grep -qF 'move_data(lobby, p + 2, 8);' $@.body && \
	 grep -qF 'case 0x58494247:' $@.body && grep -qF 'swap_gvrt_word(&len);' $@.body && \
	 grep -qF 'texture_error.code = 4;' $@.body || \
	 { echo "GUARD: ladder4 gvr slice body drifted — stop"; rm -f $@.body; exit 1; }
	@grep -qF 'void swap_gvrt_word(u32 *p) {' $@.body && \
	 { echo "GUARD: ladder4 slice leaked the era swap body (AN-1 keeps it at the prelude seam) — stop"; rm -f $@.body; exit 1; } || true
	@mv $@.body $@; echo "gen: ladder4_gvr_glue.cpp (find_gvrt_texture_chunk — verbatim)"

$(GEN)/ladder4_gvr_glue.o: $(GEN)/ladder4_gvr_glue.cpp $(BRINGUP)/include/ladder4_gvr_prelude.h | $(OUT)
	clang++ $(CXXFLAGS) -c -o $@ $<

# C11 TU: the ONLY place compat/pso_byteswap.h is bound (the header refuses
# C++); the ladder's entire endian vocabulary flows through its accessors.
$(OUT)/ladder4_supply.o: $(BRINGUP)/ladder4_supply.c | $(OUT) $(OUT)/waiver.stamp
	clang $(CFLAGS) -c -o $@ $<

$(OUT)/ladder4_test.o: $(BRINGUP)/ladder4_test.cpp | $(OUT) $(OUT)/waiver.stamp
	clang++ $(CXXFLAGS) -c -o $@ $<

$(BIN)/ladder4_test: $(OUT)/crt0_nomain.o $(OUT)/osinit.o $(OUT)/osalloc_era.o $(OUT)/dvd_shim.o \
                     $(GEN)/move_data_glue.o $(GEN)/ladder3_gsl_glue.o \
                     $(GEN)/ladder4_gvr_glue.o $(OUT)/ladder4_supply.o $(OUT)/ladder4_test.o | $(BIN)
	clang++ -target $(LINK_TARGET) -o $@ $^

all: $(BIN)/ladder4_test

# --- P2-AU boot ladder rung 5: texture UPLOAD composition ------------------
# Two generated VERBATIM slice TUs (docs/p2-tex-upload.md):
#   ladder5_tex_glue.cpp : GinjaTextureHeader.cpp:62-105 — init_ginja_texture_header
#     (0x803DB0AC) + get_texture_buffer_size (0x803DB15C) + alloc_buffer_from_heap
#     (0x803DB1B4), C++ (era def unit is .cpp), prelude carries the struct map +
#     extern "C" bindings (dolphin include trees stay OFF executing TUs, D4).
#   ladder5_gxtexture_glue.c : GXTexture.c:53-96/:98-139 — __GXGetTexTileShift +
#     GXGetTexBufferSize (0x8038BE90, the bpp switch; jumptable_804F50A8),
#     compiled as C11 (era unit is C; C++ would reject the u32→enum call).
# The wrapper seam (0x803DA12C composition, deviation AU-1) is ladder5_seam.cpp.
# WHOLE-FILE PROBES (both decomp units refused): GinjaTextureHeader.cpp pulls
# <dolphin/gx/GXTexture.h>/<dolphin/os/OSAlloc.h> + NinjaTypes.h (stitch-pending
# NJS pair); GXTexture.c pulls <dolphin/gx.h> (PPC reg types, CHECK_GXBEGIN HW).
GINTH_SRC := $(DECOMP_ROOT)/src/pso/render/GinjaTextureHeader.cpp
GXTXT_SRC := $(DECOMP_ROOT)/libs/dolphin/src/gx/GXTexture.c

$(GEN)/ladder5_tex_glue.cpp: $(GINTH_SRC) $(BRINGUP)/include/ladder5_tex_prelude.h | $(GEN) $(OUT)/waiver.stamp
	@{ printf '#include "ladder5_tex_prelude.h"\n'; \
	   awk '/^s32 init_ginja_texture_header\(GinjaTextureHeader/{f=1} f{print} f&&/^}$$/{if(++n==3) exit}' $(GINTH_SRC); \
	 } > $@.body
	@grep -qF 'header->format = (u8)format;' $@.body && \
	 grep -qF 'if ((format & 0x100) && width != height) {' $@.body && \
	 grep -qF 'header->size = get_texture_buffer_size(header);' $@.body && \
	 grep -qF 'return GXGetTexBufferSize(header->width, header->height, header->format, mipmap, 100);' $@.body && \
	 grep -qF 'void *pixels = OSAllocFromHeap(locked_cache_heap_handle, header->size);' $@.body && \
	 grep -qF 'header->pixels = (u32 *)pixels;' $@.body || \
	 { echo "GUARD: ladder5 tex slice body drifted — stop"; rm -f $@.body; exit 1; }
	@grep -qF 'OSFreeToHeap' $@.body && \
	 { echo "GUARD: ladder5 tex slice leaked free_texture_pixels — stop"; rm -f $@.body; exit 1; } || true
	@mv $@.body $@; echo "gen: ladder5_tex_glue.cpp (GinjaTextureHeader.cpp :62-105 three fns — verbatim)"

$(GEN)/ladder5_gxtexture_glue.c: $(GXTXT_SRC) $(BRINGUP)/include/ladder5_gx_defs.h | $(GEN) $(OUT)/waiver.stamp
	@{ printf '#include "ladder5_gx_defs.h"\n'; \
	   awk '/^static inline void __GXGetTexTileShift\(GXTexFmt/{f=1} f{print} f&&/^}$$/{exit}' $(GXTXT_SRC); \
	   awk '/^u32 GXGetTexBufferSize\(u16/{f=1} f{print} f&&/^}$$/{exit}' $(GXTXT_SRC); \
	 } > $@.body
	@grep -qF '*rowTileS = 3;' $@.body && grep -qF '*colTileS = 2;' $@.body && \
	 grep -qF 'case 0x9:' $@.body && \
	 grep -qF 'if (format == GX_TF_RGBA8 || format == GX_TF_Z24X8) {' $@.body && \
	 grep -qF 'bufferSize = nx * ny * tileBytes;' $@.body && \
	 grep -qF 'ASSERTMSGLINEV(0x1AB, width <= 1024' $@.body || \
	 { echo "GUARD: ladder5 gxtexture slice body drifted — stop"; rm -f $@.body; exit 1; }
	@grep -qF 'void GXInitTexObj' $@.body && \
	 { echo "GUARD: ladder5 gxtexture slice leaked past GXGetTexBufferSize — stop"; rm -f $@.body; exit 1; } || true
	@mv $@.body $@; echo "gen: ladder5_gxtexture_glue.c (GXTexture.c :53-96/:98-139 — verbatim)"

$(GEN)/ladder5_tex_glue.o: $(GEN)/ladder5_tex_glue.cpp | $(OUT)
	clang++ $(CXXFLAGS) -c -o $@ $<

$(GEN)/ladder5_gxtexture_glue.o: $(GEN)/ladder5_gxtexture_glue.c | $(OUT)
	clang $(CFLAGS) -I$(DECOMP_ROOT)/include -c -o $@ $<

$(OUT)/ladder5_seam.o: $(BRINGUP)/ladder5_seam.cpp $(BRINGUP)/include/ladder5_tex_prelude.h | $(OUT) $(OUT)/waiver.stamp
	clang++ $(CXXFLAGS) -c -o $@ $<

$(OUT)/ladder5_test.o: $(BRINGUP)/ladder5_test.cpp | $(OUT) $(OUT)/waiver.stamp
	clang++ $(CXXFLAGS) -c -o $@ $<

$(BIN)/ladder5_test: $(OUT)/crt0_nomain.o $(OUT)/osinit.o $(OUT)/osalloc_era.o $(OUT)/dvd_shim.o \
                     $(GEN)/move_data_glue.o $(GEN)/ladder3_gsl_glue.o $(GEN)/ladder4_gvr_glue.o \
                     $(OUT)/ladder4_supply.o \
                     $(GEN)/ladder5_gxtexture_glue.o $(GEN)/ladder5_tex_glue.o $(OUT)/ladder5_seam.o \
                     $(OUT)/ladder5_test.o | $(BIN)
	clang++ -target $(LINK_TARGET) -o $@ $^

all: $(BIN)/ladder5_test

# --- P2-BA boot ladder rung 6: TEXTURE CONSUMER (V1 adapters + V2 walk) -----
# Generated TU = the contract §1.3 consumer chain sliced VERBATIM off the
# read-only decomp tree + the enumerated NJS_HANDLE adapter table (generator
# fail-louds on anchor drift and prints the edit receipt). PER-TU NJS
# ALLOW-LIST (docs/p2-njs-repin-1.md §6): exactly the two TUs below compile
# with -Icompat/njs FIRST (shadow NJS_TEXNAME/NJS_TEXLIST u32 slots); no .c TU
# ever receives this flag set (AR §5). The build/Makefile parse-side request
# (docs/p2-gx-tex-consumer.md §4) is integrator-owned and NOT applied here.
NJS6_CXXFLAGS := -I$(REPO)/compat/njs -I$(DECOMP_ROOT)/libs/dolphin/include $(CXXFLAGS)
L6_DEPS := $(DECOMP_ROOT)/src/pso/unk/text_803DD5E0.cpp \
           $(DECOMP_ROOT)/src/pso/unk/text_803DA798.cpp \
           $(DECOMP_ROOT)/src/pso/unk/text_803D96A8.cpp \
           $(DECOMP_ROOT)/src/pso/render/GinjaTextureTable.cpp \
           $(DECOMP_ROOT)/src/pso/render/psoLoadTexture.cpp \
           $(DECOMP_ROOT)/src/pso/render/GinjaModelQueue.cpp \
           $(DECOMP_ROOT)/src/pso/ui/frontend/TAdSinglePlyChrSelectGC_803DA038.cpp \
           $(DECOMP_ROOT)/src/pso/player/TObjPlayer_8023E480.cpp \
           $(DECOMP_ROOT)/src/pso/object/TObjSinBoard.cpp \
           $(DECOMP_ROOT)/src/pso/render/GinjaTextureHeader.cpp

$(GEN)/ladder6_tex_consumer.cpp: $(BRINGUP)/gen_ladder6_slices.py \
        $(L6_DEPS) $(BRINGUP)/include/ladder6_tex_prelude.h | $(GEN) $(OUT)/waiver.stamp
	@python3 $(BRINGUP)/gen_ladder6_slices.py $(DECOMP_ROOT) $@
	@echo "gen: ladder6_tex_consumer.cpp (consumer chain — verbatim + BA-V1 adapter table; receipt above)"

$(GEN)/ladder6_tex_consumer.o: $(GEN)/ladder6_tex_consumer.cpp | $(OUT)
	clang++ $(NJS6_CXXFLAGS) -c -o $@ $<

# (V2 lands the ladder6_test binary rule below this marker + run.sh line.)
$(OUT)/ladder6_test.o: $(BRINGUP)/ladder6_test.cpp | $(OUT) $(OUT)/waiver.stamp
	clang++ $(NJS6_CXXFLAGS) -c -o $@ $<

$(BIN)/ladder6_test: $(OUT)/crt0_nomain.o $(OUT)/osinit.o $(OUT)/osalloc_era.o $(OUT)/dvd_shim.o \
                     $(GEN)/move_data_glue.o $(GEN)/ladder4_gvr_glue.o $(OUT)/ladder4_supply.o \
                     $(GEN)/ladder5_gxtexture_glue.o $(GEN)/ladder5_tex_glue.o $(OUT)/ladder5_seam.o \
                     $(GEN)/ladder6_tex_consumer.o $(OUT)/ladder6_test.o | $(BIN)
	clang++ -target $(LINK_TARGET) -o $@ $^

all: $(BIN)/ladder6_test

# --- P2-L7 boot ladder rung 5 PLAY-1: logo main-phase legs -------------------
# Five promoted legs slice VERBATIM off the read-only decomp (fail-loud
# anchors, gen_ladder7_slices receipt printed by the generator): TMainTask::
# begin_frame (TObject.cpp:640 era-empty), poll_reset_request (ControllerInput
# .cpp:258), process_async_file_reads (AsyncFileRead.cpp:198 — home TU
# parse-PASS @apply-7), update_sound_request_slots (SoundRequests.cpp:340
# guard leg), TQuestVmDownLoadServer::is_ready (TLobby_800F8AA8.cpp:1371 via
# the LB-2 pump seam) — plus the TLogo::run_frame + request_main_phase carrier
# anchors gen_ladder6_logo.py pins gx-side. NO NJS shadow flags on these TUs
# (no adapter edits; the era headers' assert names carry applied waivers).
L7RF_SRC := $(TLOGO_SRC) $(SBI_SRC) $(DECOMP_ROOT)/src/pso/system/task/TObject.cpp \
            $(DECOMP_ROOT)/src/pso/system/ControllerInput.cpp \
            $(DECOMP_ROOT)/src/pso/system/AsyncFileRead.cpp \
            $(DECOMP_ROOT)/src/pso/audio/SoundRequests.cpp \
            $(DECOMP_ROOT)/src/pso/unk/TLobby_800F8AA8.cpp $(TGS_SRC)

$(GEN)/ladder7_runframe_glue.cpp: $(BRINGUP)/gen_ladder7_runframe.py \
        $(L7RF_SRC) $(BRINGUP)/include/ladder7_run_prelude.h | $(GEN) $(OUT)/waiver.stamp
	@python3 $(BRINGUP)/gen_ladder7_runframe.py $(DECOMP_ROOT) $@
	@echo "gen: ladder7_runframe_glue.cpp (five promoted legs + run_frame/request_main_phase — verbatim, zero edits; receipt above)"

$(GEN)/ladder7_runframe_glue.o: $(GEN)/ladder7_runframe_glue.cpp | $(OUT)
	clang++ $(CXXFLAGS) -c -o $@ $<

$(OUT)/ladder7_seam.o: $(BRINGUP)/ladder7_seam.cpp $(BRINGUP)/include/ladder7_run_prelude.h | $(OUT) $(OUT)/waiver.stamp
	clang++ $(CXXFLAGS) -c -o $@ $<

$(OUT)/ladder7_test.o: $(BRINGUP)/ladder7_test.cpp $(BRINGUP)/include/ladder7_run_prelude.h | $(OUT) $(OUT)/waiver.stamp
	clang++ $(CXXFLAGS) -c -o $@ $<

$(BIN)/ladder7_test: $(OUT)/crt0_nomain.o $(OUT)/osinit.o $(OUT)/osalloc_era.o $(OUT)/dvd_shim.o \
                     $(GEN)/move_data_glue.o \
                     $(GEN)/ladder7_runframe_glue.o $(OUT)/ladder7_seam.o $(OUT)/ladder7_test.o | $(BIN)
	clang++ -target $(LINK_TARGET) -o $@ $^

all: $(BIN)/ladder7_test

# --- P2-HG1 handle-program GOLDEN: window registry + HG-1 runtime contract ----
# The contract test docs/p2-handle-program-1.md §5 demands BEFORE any of the
# 260 RUNTIME-GATED rows is adopted. host/bringup owns the registry
# (hg1_windows.cpp — runtime form of the INERT compat/njs/handle sketch, which
# stays unreferenced: no -I to it, ever). crt0.c owns the two base registers
# AND the era-window anchor table the binds derive from (HG-1.0). No NJS
# shadow flags, no era bodies: this leg crosses the seam directly. Links only
# crt0_nomain.o (install + anchors) — no disc/heap/PRS deps.
$(OUT)/hg1_windows.o: $(BRINGUP)/hg1_windows.cpp $(BRINGUP)/include/hg1_windows.h $(BRINGUP)/pso_bringup.h | $(OUT) $(OUT)/waiver.stamp
	clang++ $(CXXFLAGS) -c -o $@ $<

$(OUT)/hg1_test.o: $(BRINGUP)/hg1_test.cpp $(BRINGUP)/include/hg1_windows.h | $(OUT) $(OUT)/waiver.stamp
	clang++ $(CXXFLAGS) -c -o $@ $<

$(BIN)/hg1_test: $(OUT)/crt0_nomain.o $(OUT)/hg1_windows.o $(OUT)/hg1_test.o | $(BIN)
	clang++ -target $(LINK_TARGET) -o $@ $^

all: $(BIN)/hg1_test

clean:
	@rm -rf $(OUT)
	@echo "cleaned $(OUT)"
