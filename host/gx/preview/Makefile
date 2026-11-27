# pso-metal P2-PREVIEW owner preview tooling — own mk, own binary, own OUT dir.
# Mirrors host/gx/gx_window.mk conventions but compiles NOTHING from the
# driver lanes: this lane never rebuilds gx_window/gx_dl/gx_logo, it only
# builds the window-capture helper. Sole build exception granted by the
# wave-5.5b brief: `make -C host/gx/preview` -> build/gxpreview/ (disjoint
# from build/gxwindow, build/gxdl, build/bringup).
#
#   make -C host/gx/preview          # -> build/gxpreview/bin/winlist
#   make -C host/gx/preview clean
#
# The binary is gitignored (build/.gitignore: `bin/`); the SOURCE of the
# capture tool is committed under tools/.
SHELL := /bin/bash
REPO := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/../../..)
OUT  := $(REPO)/build/gxpreview
BIN  := $(OUT)/bin
LINK_TARGET ?= arm64-apple-macos12.0

.PHONY: all clean
all: $(BIN)/winlist

$(OUT) $(BIN):
	@mkdir -p $@

$(BIN)/winlist: $(REPO)/host/gx/preview/tools/winlist.swift | $(BIN)
	xcrun swiftc -O -target $(LINK_TARGET) -o $@ $<

clean:
	@rm -rf $(OUT)
	@echo "cleaned $(OUT)"
