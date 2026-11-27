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
echo "== P2-AE boot ladder 3: resource_decompress_buffer era-caller chain + GSL config table off fake_disc + TLogo state machine to 7 =="
"$R/build/bringup/bin/ladder3_test"
echo "ladder3_test: EXIT=0"
echo
echo "== P2-AN boot ladder 4: texture supply chain — era find_gvrt_texture_chunk walk over a synthetic GVR off fake_disc, descriptor + texel buffer at the upload doorstep =="
"$R/build/bringup/bin/ladder4_test"
echo "ladder4_test: EXIT=0"
echo
echo "== P2-AU boot ladder 5: texture UPLOAD — AN pack through era init_texture_header_and_alloc∘init_ginja_texture_header∘get_texture_buffer_size(GXGetTexBufferSize bpp)∘alloc_buffer_from_heap, heap read-back hash == fixture texels =="
"$R/build/bringup/bin/ladder5_test"
echo "ladder5_test: EXIT=0"
echo
echo "BRINGUP GREEN"
