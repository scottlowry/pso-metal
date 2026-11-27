# pso-metal P2-AG FIRST LINK — isolated build (objects NEVER touch build/bringup).
#   make -f host/link/link.mk all            # compile + gate + link out/pso_first_link
#   make -f host/link/link.mk run            # scripted boot slice (exit 0 = pass)
#   make -f host/link/link.mk clean
#
# Selection rule for the era-TU set (task step 1): take the P2-Y boot-spine
# closure (bringup objects, recipes mirrored from bringup.mk) and add every
# P2-U pass-set TU WHOLE-FILE whose `nm -u` closes inside the set once the 9
# scaffold backings + MSL table shim are admitted. Measured 2026-11-26:
#   IN : system/memory                  (heap trio -> osalloc_era.o; move_data -> slice)
#        system/MiddlewareInterruptLock (interrupt pair -> host_backings.c)
#        unk/text_803DEE48              (DVDChangeDir -> host_backings.c)
#        unk/text_803D9FF0              (___upper_map -> gen/msl_maps.c)
#   OUT: boss/derolle/ProjectedColorDraw (+4 ScreenPrimitiveDraw internals),
#        system/TLanguageFontPalette (+init_and_load_tlut, load_gvpl_palette_file),
#        system/Lzss (+calc_delta/debug_print/get_coarse_time/bswap_32) —
#        all game-internal-unresolved per data/link-deps.tsv; no honest
#        backing exists inside THIS set, and slicing their def TUs is the
#        LP64 pipeline's job, not the first link's.
SHELL := /bin/bash
REPO  := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/../..)
D     ?= $(HOME)/Hermes/pso-decomp/pso-decomp
BR    := $(REPO)/host/bringup
L     := $(REPO)/host/link
OBJ   := $(L)/obj
GEN   := $(L)/gen
OUT   := $(L)/out
COMPAT := $(REPO)/compat/pso_metal_compat.h
WAIV   := $(REPO)/build/waivers/pso_assert_waivers.h
MSL    := $(D)/libs/PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/ctype.c
TARGET := arm64-apple-macos12.0
# bringup flag set (D4/D5 per bringup.mk — hosted, facade -I first)
CB  := -w -fms-extensions -Wno-c++11-narrowing -DPSO_PORT_LAYOUT_WAIVER -D_DARWIN_C_SOURCE \
       -include $(COMPAT) -include $(WAIV) -I$(BR)/include
CXX := $(CB) -I$(BR) -I$(D)/include -I$(D)/src
# U's exact era-object flag set (full era includes, -ffreestanding, macos11)
UC  := -w -fms-extensions -ffreestanding -Wno-c++11-narrowing -include $(COMPAT) -include $(WAIV) \
       -I$(D)/include -I$(D)/include/clib -I$(D)/libs/dolphin/include \
       -I$(D)/libs/dolphin/include/dolphin -I$(D)/src
.PHONY: all run clean gate

all: gate $(OUT)/pso_first_link

$(OBJ) $(GEN) $(OUT):
	@mkdir -p $@

$(OBJ)/waiver.stamp: $(WAIV) | $(OBJ)
# D8-style sha-pinned verify (apply-6: replaces the pre-mass-waiver literal
# grep 'STATE: 99 waived' — stale since the 2179/2943 header eras; literal
# STATE guards rot silently, sha pins do not. Content authority =
# host/bringup/waiver-header.sha256, re-pinned by the integrator per wave.)
	@sha256sum $(WAIV) | cut -d' ' -f1 | grep -qxF "$$(cat $(BR)/waiver-header.sha256)" \
	  || { echo "GUARD: waiver header sha != host/bringup/waiver-header.sha256 — stop (regen is integrator-only, at no-make boundaries)"; exit 1; }
	@touch $@

# --- generated era copies (same guards as bringup.mk D3; waiver header consumed AS-IS, never gen_waivers) ---
$(GEN)/prs_glue.cpp: $(D)/src/pso/system/prs.cpp | $(GEN)
	@n=$$(grep -c '0x80001800' $<); [ "$$n" = 1 ] || { echo "GUARD: $$n window literals"; exit 1; }
	@sed 's/0x80001800/pso_prs_window_base/' $< > $@
	@sed 's/pso_prs_window_base/0x80001800/' $@ > $@.rt; diff -q $@.rt $< >/dev/null || { echo GUARD: drift; exit 1; }; rm -f $@.rt
$(GEN)/prs_fade_glue.cpp: $(D)/src/pso/system/TScreenFadeController.cpp | $(GEN)
	@n1=$$(grep -n '^u32 prs_decompress_buffer(const void \*src, void \*dst) {' $< | cut -d: -f1); \
	 n2=$$(grep -n '^s32 prs_decompress_alloc(const void \*src, void \*dst, s32 size_limit) {' $< | cut -d: -f1); \
	 [ -n "$$n1" ] && [ -n "$$n2" ] && [ "$$n2" -gt "$$n1" ] || { echo GUARD: anchors; exit 1; }; \
	 end=$$(awk -v n=$$n2 'NR>n && /^}$$/ {print NR; exit}' $<); \
	 { printf '#include "types.h"\n#include "pso/system/prs.h"\n/* verbatim slice lines %s..%s */\n' $$n1 $$end; sed -n "$${n1},$${end}p" $<; } > $@
$(GEN)/move_data_glue.cpp: $(D)/src/pso/unk/TLobby_803DE778.cpp | $(GEN)
	@m=$$(grep -n '^void move_data(void \*dst, const void \*src, s32 n) {' $< | cut -d: -f1); \
	 [ -n "$$m" ] || { echo GUARD: move_data anchor; exit 1; }; \
	 sed -n "$${m},$$((m+2))p" $< | sed -n '2p' | grep -qx '    memmove(dst, src, n);' || { echo GUARD: move_data body; exit 1; }; \
	 { printf '#include "types.h"\n#include <string.h>\n/* verbatim slice lines %s..%s */\n' $$m $$((m+2)); sed -n "$${m},$$((m+2))p" $<; } > $@
# ___upper_map/___lower_map: ONLY verbatim source = vendored MSL ctype.c
# (outside the 1680-unit probe set — U's truly-missing row). Extract just
# the two 256-byte tables (guards: exactly one table each, 256 entries each).
$(GEN)/msl_maps.c: $(MSL) | $(GEN)
	@{ printf '/* P2-AG: verbatim __upper_map/__lower_map extraction from\
 * %s (era MSL). Whole-file adoption is impossible on a hosted\
 * link unit (isalpha etc. would collide with host libc); these two 256-byte\
 * tables are the only symbols the era TU text_803D9FF0.cpp references. */\n' $(MSL); \
	   for t in upper lower; do \
	     awk -v t=$$t '$$0=="unsigned char __"t"_map[256] = {"{f=1} f{print} f&&/^};/{exit}' $<; \
	   done; \
	   printf '_Static_assert(sizeof __upper_map == 256, "upper_map width");\n_Static_assert(sizeof __lower_map == 256, "lower_map width");\n_Static_assert(sizeof __upper_map + sizeof __lower_map == 512, "2x256B shim");\n'; } > $@
	@c=$$(grep -c 'map\[256\] = {' $@); [ "$$c" = 2 ] || { echo "GUARD: table count $$c"; rm -f $@; exit 1; }
	@clang -std=c11 -c -o $@.syntax $@ 2>/dev/null && rm -f $@.syntax || { echo "GUARD: msl_maps.c failed compile/size-assert check"; rm -f $@; exit 1; }

# --- bringup closure (recipes mirrored from host/bringup/bringup.mk) ---
$(OBJ)/crt0_nomain.o: $(BR)/crt0.c | $(OBJ) $(OBJ)/waiver.stamp
	@clang -std=c11 $(CB) -DPSOB_TEST_BUILD -target $(TARGET) -c -o $@ $<
$(OBJ)/osinit.o $(OBJ)/dvd_shim.o $(OBJ)/osalloc_era.o: $(OBJ)/%.o: $(BR)/%.c | $(OBJ) $(OBJ)/waiver.stamp
	@clang -std=c11 $(CB) -target $(TARGET) -c -o $@ $<
$(OBJ)/prs_support.o: $(BR)/prs_support.cpp | $(OBJ) $(OBJ)/waiver.stamp
	@clang++ -std=c++17 $(CXX) -target $(TARGET) -c -o $@ $<
$(OBJ)/memory.o: $(D)/src/pso/system/memory.cpp | $(OBJ) $(OBJ)/waiver.stamp
	@clang++ -std=c++17 $(CXX) -target $(TARGET) -c -o $@ $<
$(OBJ)/prs_glue.o: $(GEN)/prs_glue.cpp | $(OBJ)
	@clang++ -std=c++17 $(CXX) -include $(BR)/include/prs_window_glue.h -target $(TARGET) -c -o $@ $<
$(OBJ)/prs_fade_glue.o $(OBJ)/move_data_glue.o: $(OBJ)/%.o: $(GEN)/%.cpp | $(OBJ)
	@clang++ -std=c++17 $(CXX) -target $(TARGET) -c -o $@ $<

# --- chosen era TUs, whole-file, U's exact flags ---
define ERA_RULE
$(OBJ)/era_$(subst /,_,$(1)).o: $(D)/src/pso/$(1).cpp | $(OBJ) $(OBJ)/waiver.stamp
	@clang++ -std=c++17 $(UC) -target arm64-apple-macos11 -c -o $$@ $$<
endef
$(foreach t,system/MiddlewareInterruptLock unk/text_803DEE48 unk/text_803D9FF0,$(eval $(call ERA_RULE,$(t))))

# --- host/link backings + boot slice ---
$(OBJ)/host_backings.o: $(L)/host_backings.c | $(OBJ)
	@clang -std=c11 -w -target $(TARGET) -c -o $@ $<
$(OBJ)/msl_maps.o: $(GEN)/msl_maps.c | $(OBJ)
	@clang -std=c11 -w -target $(TARGET) -c -o $@ $<
$(OBJ)/boot_slice.o: $(L)/boot_slice.cpp | $(OBJ) $(OBJ)/waiver.stamp
	@clang++ -std=c++17 $(CXX) -target $(TARGET) -c -o $@ $<

OBJS = $(OBJ)/boot_slice.o $(OBJ)/crt0_nomain.o $(OBJ)/osinit.o $(OBJ)/osalloc_era.o \
       $(OBJ)/dvd_shim.o $(OBJ)/prs_glue.o $(OBJ)/prs_support.o $(OBJ)/prs_fade_glue.o \
       $(OBJ)/move_data_glue.o $(OBJ)/memory.o \
       $(OBJ)/era_system_MiddlewareInterruptLock.o $(OBJ)/era_unk_text_803DEE48.o \
       $(OBJ)/era_unk_text_803D9FF0.o \
       $(OBJ)/host_backings.o $(OBJ)/msl_maps.o

# --- gate: ZERO non-system undefined. System set = explicit whitelist of
# macOS libSystem/libc++/libc++abi imports actually observed (26; equals
# P2-U's excluded 23 up to __Unwind/chk variants). Open symbols outside
# the whitelist fail closed. Additionally asserts all 9 scaffold symbols
# (plus ___upper_map) are DEFINED in the object set. Real verdict remains
# ld's default -undefined error at link time. ---
$(OBJ)/syswhitelist.txt: $(OBJS)
	@printf '%s\n' _memmove _memset _memcmp _strcmp _strlen _strstr _open _close \
	 _pread _stat _fstat _mmap _getenv _printf _fprintf _vprintf _abort _exit \
	 ___snprintf_chk ___memset_chk ___vsnprintf_chk ___stack_chk_fail ___stack_chk_guard \
	 ___stderrp ___cxa_allocate_exception ___cxa_begin_catch ___cxa_end_catch \
	 ___cxa_throw ___cxa_rethrow ___cxa_allocate_exception _clock_gettime \
	 __Unwind_Resume ___gxx_personality_v0 __Znwm __ZdlPv __Znam __ZdaPv \
	 __ZTVN10__cxxabiv117__class_type_infoE __ZTVN10__cxxabiv120__si_class_type_infoE \
	 __ZTISt9bad_alloc __ZTVSt9bad_alloc | sort -u > $@

gate: $(OBJS) $(OBJ)/syswhitelist.txt
	@nm -gU $(OBJS) | awk '{print $$NF}' | sort -u > $(OBJ)/defs.txt
	@nm -u $(OBJS)  | awk '{print $$NF}' | sort -u > $(OBJ)/undefs.txt
	@comm -23 $(OBJ)/undefs.txt $(OBJ)/defs.txt > $(OBJ)/open.txt
	@comm -23 $(OBJ)/open.txt $(OBJ)/syswhitelist.txt > $(OBJ)/open-nonsys.txt
	@if [ -s $(OBJ)/open-nonsys.txt ]; then echo "GATE FAIL — non-system undefined remains:"; cat $(OBJ)/open-nonsys.txt; exit 1; fi
	@miss=0; for s in _OSDisableInterrupts _OSRestoreInterrupts _GXBegin _GXDrawDone \
	  _DCStoreRange _OSAllocFromHeap _OSFreeToHeap _OSCreateHeap _DVDChangeDir ___upper_map; do \
	  grep -qx "$$s" $(OBJ)/defs.txt || { echo "GATE FAIL: backing missing: $$s"; miss=1; }; done; exit $$miss
	@echo "GATE PASS: $(words $(OBJS)) objects; open=system-only; 9 scaffold backings + ___upper_map all defined"

$(OUT)/pso_first_link: $(OBJS) | $(OUT)
	@clang++ -target $(TARGET) -o $@ $(OBJS)
	@echo "linked: $@"

# exe-level proof: remaining undefineds must ALL be system dylib imports
run: $(OUT)/pso_first_link
	@nm -u $< | awk '{print $$NF}' | sort -u > $(OBJ)/exe-undefs.txt
	@comm -23 $(OBJ)/exe-undefs.txt $(OBJ)/syswhitelist.txt > $(OBJ)/exe-open-nonsys.txt
	@echo "exe undefined imports: $$(grep -c . $(OBJ)/exe-undefs.txt), non-system: $$(grep -c . $(OBJ)/exe-open-nonsys.txt || true) (must be 0 — ld -undefined error already proves it)"
	@test ! -s $(OBJ)/exe-open-nonsys.txt
	@PSO_FAKE_DISC=$(BR)/fake_disc $<

clean:
	@rm -rf $(OBJ) $(GEN) $(OUT)
	@echo "cleaned host/link scratch"
