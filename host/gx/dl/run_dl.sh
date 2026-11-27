#!/bin/sh
# P2-Z one-command display-list gate. Clean rebuild, then 30 frames of ONE
# textured quad driven through the era GP wire format — alternating PATH A
# (FIFO-lite capture of GXBegin fmt4 + POS3F32/CLR1U32/TEX0-2F32, the
# TWindowSubStatusDisp_803DEE6C.cpp:43 recipe) and PATH B (hand-built BE
# display list executed via a captured CP 0x40 call, the GXCallDisplayList
# wire of GinjaMaterial.cpp:478). Readback asserts 7 points/frame: 4 quad
# corners + center vs texture texel colors, 2 outside vs clear color.
# Exit 0 only if all 30 frames verify. Codes: 2 window/boot, 3 encode/decode
# fatal, 4 readback mismatch, 5 bookkeeping (W's spike convention).
set -e
B=$(cd "$(dirname "$0")" && pwd)
R=$(cd "$B/../../.." && pwd)
export PSO_FAKE_DISC="$R/host/bringup/fake_disc"

echo "== pso-metal P2-Z display-list: clean rebuild (DECOMP_ROOT=${DECOMP_ROOT:-$HOME/Hermes/pso-decomp/pso-decomp}) =="
make -C "$B" -f gx_dl.mk clean
make -C "$B" -f gx_dl.mk all

echo
echo "== run: era GP wire replay, 30 frames, 7-point drawable readback =="
set +e
"$R/build/gxdl/bin/gx_dl"
rc=$?
set -e
echo "-- gx_dl exit: $rc"
if [ "$rc" = 0 ]; then echo "GX DL GREEN: first textured geometry translated from era display-list words"; fi
exit "$rc"
