# pso-metal Phase-1 build skeleton
#
# Two ABI modes, deliberately separate (see docs/p1-buildsystem.md):
#   PARSE_TARGET — arm64_32 (ILP32) — reproduces the Phase-0 669-unit parse
#     baseline measured by probes/tree_compile_probe.sh. This triple CANNOT
#     link on macOS; it exists to measure "does the source parse under a
#     console-like 32-bit data model".
#   LINK_TARGET  — the LP64 Apple Silicon triple the boot slice will actually
#     execute. Real compilation/linking (Phase 1 runtime) happens here.
#
# Everything is driven by DECOMP_ROOT so nothing hardcodes a machine:
#   make -C build parse                     # default DECOMP_ROOT=$(HOME)/Hermes/pso-decomp/pso-decomp
#   make -C build parse DECOMP_ROOT=/path/to/pso-decomp
#   make -C build list-fail                 # -> build/parse_fail.txt
#   make -C build parse PARSE_TARGET=arm64-apple-macos12.0   # LP64 reality check

REPO         := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/..)
BUILD_DIR    := $(REPO)/build
DECOMP_ROOT  ?= $(HOME)/Hermes/pso-decomp/pso-decomp
COMPAT       := $(REPO)/compat/pso_metal_compat.h
WAIVER_HDR   := $(BUILD_DIR)/waivers/pso_assert_waivers.h
PARSE_DIR    := $(BUILD_DIR)/parse
UNITS        := $(BUILD_DIR)/units.txt
JOBS         ?= 8

# Exact flag set proven by probes/tree_compile_probe.sh (669/1680 @ arm64_32).
# WHY -ffreestanding: decomp declares 'void main(void)'; the hosted-mode
#   "'main' must return int" check is a host conformance rule, not a parse error.
# WHY -Wno-c++11-narrowing: 'case -1:' on a u32 selector is valid MWCC
#   wrap-around; clang's default-error narrowing warning is a host rule.
PARSE_TARGET ?= arm64_32-apple-watchos7.0
LINK_TARGET  ?= arm64-apple-macos12.0

SDK_INCS := -I$(DECOMP_ROOT)/include -I$(DECOMP_ROOT)/include/clib \
            -I$(DECOMP_ROOT)/libs/dolphin/include \
            -I$(DECOMP_ROOT)/libs/dolphin/include/dolphin \
            -I$(DECOMP_ROOT)/src

# P2 adoption wave (docs/p2-collision-debt.md §5 rebind-only block VERBATIM +
# docs/p2-gx-tex-consumer.md §4): the collision pin force-include occupies the
# era guard _DOLPHIN_TYPES_H_ so the quoted-sibling edge (mtx.h -> "types.h",
# 1,109 TUs) can never fire the rebind-vs-era typedef cluster. Laws honored:
#  - pin comes AFTER compat + waivers in the -include list, exactly as measured;
#  - pin fails closed without -DPSO_PORT_LAYOUT_WAIVER, silent no-op without
#    the rebind -I, so BOTH ride together (host/bringup/include earliest among
#    repo dirs wins <types.h>);
#  - compat/repin and compat/njs are NEVER -include'd (repin-as-include breaks
#    the ceil-libc-chain in 5 TUs, apply4 caveat); repin joins the corpus only
#    via a future full-stack re-record, njs per-TU below (AR §6: never on .c).
# REBIND IS LP64-ONLY. Every adoption-program number (H0 162, H1C 257, H3
# 1,258) was measured on the LP64 triple. Spraying the block tree-wide into
# arm64_32 fabricates layout noise the legacy anchor gate never saw (measured
# 2026-11-27: TItemWeaponDrillLauncher.cpp NJS_OBJECT_size negative-array under
# ILP32+rebind — not in the AZ collateral ledger; plus 5 unrequested flips).
COLLISION_PIN := $(REPO)/compat/collision/pin_dolphin_types.h
# apply-6 additions (docs/p2-stitch-apply5/6.md): -I compat/net (NP era-model
# shadow — hijacks all 50 quoted NetworkRuntime.h edges, NP receipt) and
# -I compat/repin (MA AT_ADDRESS dialect shadows, 38-row class) land per the
# verified NP paste block order. compat/njs stays PER-TU ONLY (allow-list).
# apply-7 adds compat/facade (D4 era OSAlloc/OSTime decls — MUST precede
# host/bringup/include: bringup's OSAlloc.h wins that angle otherwise,
# docs/p2-d4facade-1.md OF-A/OF-B proof) and compat/dialect/pshim (P2-T
# hw_regs supply, docs/p2-tail-1.md Play-1). Facade wins first AT/inclusive
# edges; pso_bringup's divergent DVD spellings vs era reconciled by the
# anchor-guarded skip blocks in compat/repin/dolphin/dvd.h (apply-7).
LP64_REBIND   := -DPSO_PORT_LAYOUT_WAIVER -include $(COLLISION_PIN) \
                 -I$(REPO)/compat/net -I$(REPO)/compat/repin \
                 -I$(REPO)/compat/facade -I$(REPO)/compat/dialect/pshim \
                 -I$(REPO)/host/bringup/include

# The waiver header is ALWAYS force-included after compat. With zero active
# waivers it is a no-op (#define PSO_ASSERT_WAIVER_ACTIVE 0 only), so the
# baseline stays byte-equivalent to the Phase-0 probe; with waivers active it
# installs the per-name LAYOUT_ASSERT dispatch (see gen_waivers.py).
CORE := -w -fms-extensions -ffreestanding -Wno-c++11-narrowing \
        -include $(COMPAT) -include $(WAIVER_HDR)
# RECURSIVE (=, not :=): -target and REBIND_FLAGS must re-expand per goal so
# the target-specific exports below actually reach clang. With := the
# watchos/ILP32 triple was baked in at file-parse time and `parse-lp64`
# silently measured arm64_32 (discovered 2026-11-27: its count equaled the
# 32-bit gate 1578 and LP64-failing TItemData.cpp was absent from fail.txt).
CXXFLAGS_PARSE = -fsyntax-only -std=c++17 $(CORE) $(REBIND_FLAGS) $(SDK_INCS) \
                 -target $(PARSE_TARGET) -ferror-limit=0
CFLAGS_PARSE   = -fsyntax-only -std=c11   $(CORE) $(REBIND_FLAGS) $(SDK_INCS) \
                 -target $(PARSE_TARGET) -ferror-limit=0

# P2-AV §4: per-TU C++ NJS shadow opt-in (docs/p2-njs-repin-1.md §6).
NJS_ALLOWLIST := $(BUILD_DIR)/njs_allowlist.txt

# P2-MX (docs/p2-musyx-1.md §5): the musyx runtime is compiled as C++ — its
# 28 .c TUs carry bare in-body `extern "C" {` blocks that no header supply can
# launder in C mode (P2-T precedent, flags measured by MX: 25/27; residue
# chorus_fx.c + reverb.c = asm-void bodies with absent orig_asm/*.inc, named
# host-facade queue). -x c++ keeps the .c corpus honest everywhere else;
# -Icompat/musyx BEFORE net/repin/SDK (MX + AA linknames audit attached).
MUSYX_CXXFLAGS = -fsyntax-only -std=c++17 $(CORE) -I$(REPO)/compat/musyx \
                 $(REBIND_FLAGS) $(SDK_INCS) \
                 -target $(PARSE_TARGET) -ferror-limit=0

export DECOMP_ROOT COMPAT WAIVER_HDR PARSE_DIR CXXFLAGS_PARSE CFLAGS_PARSE MUSYX_CXXFLAGS

.PHONY: all units waivers parse parse-lp64 list-fail smoke-link clean

all: parse

# Guard: fail early with a clear message if the decomp checkout is missing.
$(UNITS): FORCE
	@test -d $(DECOMP_ROOT)/src || { \
	  echo "ERROR: DECOMP_ROOT=$(DECOMP_ROOT) has no src/ — point make at the pso-decomp checkout (make DECOMP_ROOT=...)"; exit 1; }
	@mkdir -p $(BUILD_DIR)
	@cd $(DECOMP_ROOT) && find src \( -name '*.cpp' -o -name '*.c' \) | sort > $@
	@echo "units: $$(wc -l < $@ | tr -d ' ') source units -> $@"

units: $(UNITS)

# Regenerated on every run so the waiver flags can never drift against the tree.
waivers: FORCE
	@python3 $(BUILD_DIR)/gen_waivers.py --decomp $(DECOMP_ROOT)

# One clang -fsyntax-only per unit, xargs-parallel. NOTE: the unit path is
# passed as bash's $0 argv, NOT via xargs -I@ — BSD xargs caps each
# -I-substituted argv element at ~255 bytes and errors "too long" otherwise.
define PARSE_RECIPE
	@rm -rf $(PARSE_DIR); mkdir -p $(PARSE_DIR)/err; : > $(PARSE_DIR)/fail.txt
	@cd $(DECOMP_ROOT) && xargs -n1 -P $(JOBS) bash -c ' \
	  f="$$0"; abs="$$PWD/$$f"; safe=$$(echo "$$f" | tr / _); \
	  extra=""; if [ -n "$$NJS_LEG" ]; then case "$$f" in *.cpp) grep -qxF "$$f" $(NJS_ALLOWLIST) && extra="-I$(REPO)/compat/njs" ;; esac; fi; \
	  case "$$f" in \
	    *.cpp) clang++ $$extra $$CXXFLAGS_PARSE "$$abs" > "$(PARSE_DIR)/err/$$safe.err" 2>&1 || echo "$$f" >> "$(PARSE_DIR)/fail.txt" ;; \
	    *.c)   case "$$f" in src/musyx/runtime/*) clang++ $$MUSYX_CXXFLAGS -x c++ "$$abs" > "$(PARSE_DIR)/err/$$safe.err" 2>&1 || echo "$$f" >> "$(PARSE_DIR)/fail.txt" ;; \
	                   *) clang $$CFLAGS_PARSE "$$abs" > "$(PARSE_DIR)/err/$$safe.err" 2>&1 || echo "$$f" >> "$(PARSE_DIR)/fail.txt" ;; \
	             esac ;; \
	  esac' < $(UNITS)
	@total=$$(wc -l < $(UNITS) | tr -d ' '); fail=$$(wc -l < $(PARSE_DIR)/fail.txt | tr -d ' '); \
	 echo "== $(TAG): TOTAL=$$total FAIL=$$fail PASS=$$((total-fail)) (target=$(TARGET_USED))" | tee $(PARSE_DIR)/summary.txt
endef

parse: export TARGET_USED := $(PARSE_TARGET)
parse: export TAG := parse
parse: export REBIND_FLAGS :=
parse: export NJS_LEG :=
parse: units waivers
	$(PARSE_RECIPE)

# LP64 reality check: what passes under the triple the boot slice links with —
# plus the ADOPTED rebind+pin block (LP64_REBIND). Expect ~257-261 PASS, NOT
# the 32-bit 1,578 anchor; parse_gate certifies the arm64_32 leg.
parse-lp64: export PARSE_TARGET := $(LINK_TARGET)
parse-lp64: export TARGET_USED := $(LINK_TARGET)
parse-lp64: export TAG := parse-lp64
parse-lp64: export REBIND_FLAGS := $(LP64_REBIND)
parse-lp64: export NJS_LEG := 1
parse-lp64: units waivers
	$(PARSE_RECIPE)

list-fail: parse
	@sort $(PARSE_DIR)/fail.txt > $(BUILD_DIR)/parse_fail.txt
	@echo "failing units ($$(wc -l < $(BUILD_DIR)/parse_fail.txt | tr -d ' ')) -> $(BUILD_DIR)/parse_fail.txt"

# Proof that the real-LINK target toolchain works end to end (syntax+codegen+
# link), independent of the decomp sources which are not yet linkable.
smoke-link:
	@mkdir -p $(BUILD_DIR)/bin
	@clang -target $(LINK_TARGET) -o $(BUILD_DIR)/bin/host_smoke $(BUILD_DIR)/smoke/host_smoke.c
	@$(BUILD_DIR)/bin/host_smoke && echo "smoke-link: PASS ($(LINK_TARGET) compile+link+exec)"

clean:
	@rm -rf $(PARSE_DIR) $(BUILD_DIR)/bin $(BUILD_DIR)/parse_fail.txt
	@echo "cleaned parse artifacts (units.txt and waivers/ regenerate on next run)"

FORCE:
