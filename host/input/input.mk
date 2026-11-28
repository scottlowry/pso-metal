# P2-RDPREP step-1 — host/input seat build fragment (standalone; mirrors
# bringup.mk's flag regime: D4 include split, D5 no -ffreestanding, D8
# verify-only waiver stamp — NEVER regenerates waivers).
#   make -f host/input/input.mk            # build bin/input_test
#   make -f host/input/input.mk clean
SHELL := /bin/bash
REPO        := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/../..)
BUILD_DIR   := $(REPO)/build
INPUT       := $(REPO)/host/input
BRINGUP     := $(REPO)/host/bringup
DECOMP_ROOT ?= $(HOME)/Hermes/pso-decomp/pso-decomp-port-pin
COMPAT      := $(REPO)/compat/pso_metal_compat.h
WAIVER_HDR  := $(BUILD_DIR)/waivers/pso_assert_waivers.h
LINK_TARGET ?= arm64-apple-macos12.0
OUT         := $(BUILD_DIR)/input
GEN         := $(OUT)/gen
BIN         := $(OUT)/bin

# seat include set: host/input/include FIRST (the PADStatus-only dolphin shim),
# then the era include/src pair — same D4 posture as bringup (clib/ and
# libs/dolphin stay OFF executing TUs).
COMMON := -w -fms-extensions -Wno-c++11-narrowing \
          -DPSO_PORT_LAYOUT_WAIVER -D_DARWIN_C_SOURCE \
          -include $(COMPAT) -include $(WAIVER_HDR)
CXXFLAGS := -std=c++17 -g $(COMMON) \
            -I$(INPUT)/include -I$(BRINGUP)/include \
            -I$(DECOMP_ROOT)/include -I$(DECOMP_ROOT)/src \
            -target $(LINK_TARGET)

# D8 verify-only pattern: fail-closed if the committed waiver header changed
# from the integrator-approved sha (pinned in host/bringup/waiver-header.sha256).
$(OUT)/waiver.stamp: $(BUILD_DIR)/waivers/pso_assert_waivers.h | $(OUT)
	@test "$$(shasum -a 256 $(WAIVER_HDR) | cut -d' ' -f1)" = \
	  "$$(cat $(BRINGUP)/waiver-header.sha256)" || \
	 { echo "GUARD: waiver header sha != approved — stop"; exit 1; }
	@touch $@

$(OUT) $(GEN) $(BIN):
	@mkdir -p $@

CMS_SRC := $(DECOMP_ROOT)/src/pso/render/ChunkMaterialState.cpp
CIS_SRC := $(DECOMP_ROOT)/src/pso/system/ControllerInput.cpp
KBS_SRC := $(DECOMP_ROOT)/src/pso/ui/keyboard/KeyboardKeyState.cpp
MTH_SRC := $(DECOMP_ROOT)/src/pso/object/base/TObjChara.cpp

$(GEN)/input_pump_glue.cpp: $(INPUT)/gen_input_pump.py $(CMS_SRC) $(CIS_SRC) $(KBS_SRC) $(MTH_SRC) | $(GEN) $(OUT)/waiver.stamp
	@python3 $(INPUT)/gen_input_pump.py $(DECOMP_ROOT) $@ $(GEN)/input_slices.sha256
	@echo "gen: input_pump_glue.cpp (RDPREP legs 1-5 verbatim, pso_era_* rename-at-emit; manifest input_slices.sha256 @pin)"

$(GEN)/input_pump_glue.o: $(GEN)/input_pump_glue.cpp $(INPUT)/include/pso_input.h | $(OUT)
	clang++ $(CXXFLAGS) -c -o $@ $<

$(OUT)/input_state.o: $(INPUT)/input_state.cpp $(INPUT)/include/pso_input.h | $(OUT) $(OUT)/waiver.stamp
	clang++ $(CXXFLAGS) -c -o $@ $<

$(OUT)/input_test.o: $(INPUT)/input_test.cpp $(INPUT)/include/pso_input.h | $(OUT) $(OUT)/waiver.stamp
	clang++ $(CXXFLAGS) -c -o $@ $<

# INPUT_OBJS: what step-2/3 consumers add to their link lines.
INPUT_OBJS = $(OUT)/input_state.o $(GEN)/input_pump_glue.o

$(BIN)/input_test: $(OUT)/crt0_nomain.o $(OUT)/input_state.o $(GEN)/input_pump_glue.o $(OUT)/input_test.o | $(BIN)
	clang++ -target $(LINK_TARGET) -o $@ $^ -lm

# the seat needs the arena installer for LAYOUT-free data? NO — input globals
# are ordinary host BSS; crt0 is linked only for the compat arena the era
# window anchors expect (none here) — kept for regime parity with bringup.
$(OUT)/crt0_nomain.o: $(BRINGUP)/crt0.c | $(OUT) $(OUT)/waiver.stamp
	clang $(CFLAGS_PURE) -DPSOB_TEST_BUILD -c -o $@ $<
CFLAGS_PURE := -std=c11 -g $(COMMON) -I$(BRINGUP)/include -target $(LINK_TARGET)

clean:
	@rm -rf $(OUT)
	@echo "cleaned $(OUT)"

.PHONY: clean

all: $(BIN)/input_test

.PHONY: all clean
