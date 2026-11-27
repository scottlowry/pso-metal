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

echo "== pso-metal P2-W gx window: build (DECOMP_ROOT=${DECOMP_ROOT:-$HOME/Hermes/pso-decomp/pso-decomp}) =="
make -C "$B" -f gx_window.mk all

echo
echo "== run: boot→logo GX replay, 30 frames, per-frame drawable readback =="
set +e
"$R/build/gxwindow/bin/gx_window"
rc=$?
set -e
echo "-- gx_window exit: $rc"
if [ "$rc" = 0 ]; then echo "GX WINDOW GREEN: first real pixels proven by readback"; fi
exit "$rc"
