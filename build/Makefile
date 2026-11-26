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

# The waiver header is ALWAYS force-included after compat. With zero active
# waivers it is a no-op (#define PSO_ASSERT_WAIVER_ACTIVE 0 only), so the
# baseline stays byte-equivalent to the Phase-0 probe; with waivers active it
# installs the per-name LAYOUT_ASSERT dispatch (see gen_waivers.py).
COMMON := -w -fms-extensions -ffreestanding -Wno-c++11-narrowing \
          -include $(COMPAT) -include $(WAIVER_HDR) $(SDK_INCS)
CXXFLAGS_PARSE := -fsyntax-only -std=c++17 $(COMMON) -target $(PARSE_TARGET)
CFLAGS_PARSE   := -fsyntax-only -std=c11   $(COMMON) -target $(PARSE_TARGET)

export DECOMP_ROOT COMPAT WAIVER_HDR PARSE_DIR CXXFLAGS_PARSE CFLAGS_PARSE

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
	  case "$$f" in \
	    *.cpp) clang++ $$CXXFLAGS_PARSE "$$abs" > "$(PARSE_DIR)/err/$$safe.err" 2>&1 || echo "$$f" >> "$(PARSE_DIR)/fail.txt" ;; \
	    *.c)   clang   $$CFLAGS_PARSE   "$$abs" > "$(PARSE_DIR)/err/$$safe.err" 2>&1 || echo "$$f" >> "$(PARSE_DIR)/fail.txt" ;; \
	  esac' < $(UNITS)
	@total=$$(wc -l < $(UNITS) | tr -d ' '); fail=$$(wc -l < $(PARSE_DIR)/fail.txt | tr -d ' '); \
	 echo "== $(TAG): TOTAL=$$total FAIL=$$fail PASS=$$((total-fail)) (target=$(TARGET_USED))" | tee $(PARSE_DIR)/summary.txt
endef

parse: export TARGET_USED := $(PARSE_TARGET)
parse: export TAG := parse
parse: units waivers
	$(PARSE_RECIPE)

# LP64 reality check: what passes under the triple the boot slice links with.
# Expect a DIFFERENT (lower) count than 669 — the baseline gate is arm64_32.
parse-lp64: export PARSE_TARGET := $(LINK_TARGET)
parse-lp64: export TARGET_USED := $(LINK_TARGET)
parse-lp64: export TAG := parse-lp64
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
