#!/bin/sh
# P2-Q one-command bringup runner: builds the bringup fragment (port flags +
# waiver regen + guarded era-prs glue) and executes everything.
# Exit 0 only if the boot spine AND all golden cases pass.
set -e
B=$(cd "$(dirname "$0")" && pwd)
R=$(cd "$B/../.." && pwd)
export PSO_FAKE_DISC="$B/fake_disc"

echo "== pso-metal bringup: build (DECOMP_ROOT=${DECOMP_ROOT:-$HOME/Hermes/pso-decomp/pso-decomp}) =="
make -C "$B" -f bringup.mk "${MAKE_GOALS:-all}"

echo
echo "== boot spine: crt0 -> mini OSInit -> DVD shim -> era PrsDecoder::prs_decompress =="
"$R/build/bringup/bin/boot_spine"
echo "boot_spine: EXIT=0 (era game function executed on arm64)"

echo
echo "== PRS golden vectors (oracle: hand-traced from era prs.s) =="
"$R/build/bringup/bin/prs_golden_test"
echo "prs_golden_test: EXIT=0"

echo
echo "== P2-Y boot ladder: era PRS wrappers (TScreenFadeController) + era heap (memory.cpp over OSAlloc port) =="
"$R/build/bringup/bin/ladder_test"
echo "ladder_test: EXIT=0"
echo
echo "BRINGUP GREEN"
