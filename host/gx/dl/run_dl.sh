#!/bin/sh
# P2-Z/P2-AQ one-command display-list gate. Clean rebuild, then 30 frames of
# ONE textured quad driven through the era GP wire format — alternating PATH A
# (FIFO-lite capture of GXBegin fmt4 + POS3F32/CLR1U32/TEX0-2F32, the
# TWindowSubStatusDisp_803DEE6C.cpp:43 recipe) and PATH B (hand-built BE
# display list executed via a captured CP 0x40 call, the GXCallDisplayList
# wire of GinjaMaterial.cpp:478).
#
# P2-AQ: the FULL original 7-point gate is live — readback asserts per frame
# the 4 quad corners (2 px inside) + center color-matched to the addressed
# texture texel (TL->(0,0) red, TR->(3,0) green, BL->(0,3) blue,
# BR->(3,3) white, CT->(2,2) yellow) and 2 outside points vs the clear.
# Exit 0 only if all 30 frames x 7 samples verify. Codes: 2 window/boot,
# 3 encode/decode fatal, 4 readback mismatch, 5 bookkeeping (W's convention).
#
# `run_dl.sh --debug` runs the P2-AQ 16-texel diagnostic AFTER the gate:
# each texel gets a distinct color, the clear is distinctive (3,7,11), and one
# frame dumps BOTH the CPU-side texture roundtrip (gxh_dl_tex_roundtrip) and
# the GPU-sampled color resolved at all 16 texel cells — the instrument that
# localized the scalar read(cx,cy) miscompile (docs/p2-gx-dl2.md §2).
set -e
B=$(cd "$(dirname "$0")" && pwd)
R=$(cd "$B/../../.." && pwd)
export PSO_FAKE_DISC="$R/host/bringup/fake_disc"
DEBUG=${1:-}

echo "== pso-metal P2-Z/P2-AQ display-list: clean rebuild (DECOMP_ROOT=${DECOMP_ROOT:-$HOME/Hermes/pso-decomp/pso-decomp-port-pin}) =="
make -C "$B" -f gx_dl.mk clean
make -C "$B" -f gx_dl.mk all

echo
echo "== run: era GP wire replay, 30 frames, 7-point drawable readback =="
set +e
"$R/build/gxdl/bin/gx_dl"
rc=$?
set -e
echo "-- gx_dl exit: $rc"
if [ "$rc" = 0 ]; then echo "GX DL GREEN: full 7-point textured-quad gate — era display-list words to exact texels"; fi

if [ "$DEBUG" = "--debug" ] && [ "$rc" = 0 ]; then
  echo
  echo "== debug: 16-texel distinct-color diagnostic (CPU roundtrip + GPU sample map) =="
  set +e
  GX_DL_TEX16=1 "$R/build/gxdl/bin/gx_dl"
  drc=$?
  set -e
  echo "-- gx_dl --debug exit: $drc"
  [ "$drc" = 0 ] || rc=$drc
fi
exit "$rc"
