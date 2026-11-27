#!/bin/sh
# P2-W one-command GX→Metal window gate (extends host/bringup/run.sh pattern).
# Builds the windowed binary (Q's substrate + host/gx Metal backing) and runs
# the boot→logo GX replay: 30 frames alternating the two era clear literals,
# each proven by DRAWABLE readback at all four corners (CI-safe: the assert,
# not eyeballs, is the gate — the window auto-closes when the frames end).
# Exit 0 only if every readback matched. Codes: 2 window/device, 3 encode,
# 4 readback mismatch, 5 bookkeeping (spike/metal_clear.m convention).
set -e
B=$(cd "$(dirname "$0")" && pwd)
R=$(cd "$B/../.." && pwd)
export PSO_FAKE_DISC="$B/../bringup/fake_disc"

echo "== pso-metal P2-W gx window: build (DECOMP_ROOT=${DECOMP_ROOT:-$HOME/Hermes/pso-decomp/pso-decomp-port-pin}) =="
make -C "$B" -f gx_window.mk all

echo
echo "== run: boot→logo GX replay, 30 frames, per-frame drawable readback =="
set +e
"$R/build/gxwindow/bin/gx_window"
rc=$?
set -e
echo "-- gx_window exit: $rc"
if [ "$rc" = 0 ]; then echo "GX WINDOW GREEN: first real pixels proven by readback"; fi

echo
echo "== P2-BA V4 phase: LOGO VISIBLE — era request_main_phase -> TLogo::run_frame -> begin_gx_frame executes; render_tick presents the CLUT-expanded sinbd surface through the AQ replay; region readback gates =="
echo "== P2-BB V5: default regime = ERA CLUT SUPPLY (GVPL chunk -> load_gvpl_palette -> init_and_load_tlut); AV-1 stand-in kept behind -DGX_LOGO_AV1 — both regimes must be green =="
make -C "$B/dl" -f gx_dl.mk logo
set +e
GX_DL_TEXN= "$R/build/gxdl/bin/gx_logo"   # driver self-pins TEXN=32 before open
lrc=$?
set -e
echo "-- gx_logo exit: $lrc"
if [ "$lrc" = 0 ]; then echo "LOGO VISIBLE: sinbd texels proven in the backbuffer by readback (era CLUT supply regime)"; fi

echo
echo "== P2-BB V5 AV-1 stand-in regime (synthetic-fixture regime stays green) =="
make -C "$B/dl" -f gx_dl.mk logo-av1
set +e
GX_DL_TEXN= "$R/build/gxdl/bin/gx_logo_av1"
av1rc=$?
set -e
echo "-- gx_logo_av1 exit: $av1rc"
if [ "$av1rc" = 0 ]; then echo "AV-1 REGIME GREEN: pinned stand-in palette path intact behind define"; fi
[ "$rc" = 0 ] || exit "$rc"
[ "$av1rc" = 0 ] || exit "$av1rc"
exit "$lrc"
