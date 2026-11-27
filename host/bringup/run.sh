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
echo "== P2-BA boot ladder 6: texture CONSUMER — the sinbd texlist walk resolves through the era consumer chain (load_texture_entry dispatch -> loaders[1] -> find_gvrt_texture_chunk -> create_texture_state -> reload_texture_slot_pixels) under the AR per-TU NJS shadow; NJS_HANDLE adapter receipt per docs/p2-gx-tex-consumer.md §3 =="
"$R/build/bringup/bin/ladder6_test"
echo "ladder6_test: EXIT=0"
echo
echo "== P2-L7 boot ladder 7 (rung-5 PLAY-1): the five promoted run_frame legs EXECUTE era bodies verbatim (begin_frame TMainTask::TObject.cpp:640, poll_reset_request ControllerInput.cpp:258, process_async_file_reads AsyncFileRead.cpp:198 parse-PASS@apply-7, update_sound_request_slots SoundRequests.cpp:340 guard, is_ready TLobby_800F8AA8.cpp:1371 via LB-2 pump seam); invariants by REAL reads + flip probes, L7-R3 order unchanged =="
echo "== P2-L7 boot ladder 7 (rung-5 PLAY-2): the main_task flag spine EXECUTE era bodies verbatim — L2 run_task TObject.cpp:283-290, R3 render :509-525 (incl. the :510 ALLOW_OR_DISALLOW_CHILD re-pin), R4 render_screen_overlay :644-652, + the three era-empty REAL bodies (render_after_scene :505, debug_render_overlay :533, TObject::debug_render :174 promoted); the 8 sub-passes stay counted externs, render_pass era-INIT 1 @:25 (spec §3 D7); probes L7-P5..P9 + invariants L7-I2a..e by REAL reads =="
echo "== P2-L7 boot ladder 7 (rung-5 PLAY-3): render_tick R9 decomposition EXECUTES era bodies verbatim — :491-505 (closure :505, §3 D2) + the 13-leg era-ordered roster (9 verbatim sub-slices: kbd run_tasks SystemBootInit.cpp:100-107 §3 D5 + run_end_of_render_tasks :113-115, effect_render :10/:12 §3 D4, set_cull_none render_util.cpp:39, scene_fog enable/disable :26-32, copy_efb gjSystem.c:52-57, present_frame :59-61); GX capture seams + counted legs 4/6 + SINGLE present (rides LOGO-3 gx-side, never re-minted); lbl_805D3584=1 era INIT (§3 D7); probes L7-P10..P12 + invariants L7-I3a..d + rung-5 EXIT probes L7-Ea..Ec (is_ready gate -> request_main_phase REAL fire, cur 1->0xA->2->5 host-driven walk on the byte-anchored :342/:343/:346/:389/:390 — dispatcher body = rung-6) =="
"$R/build/bringup/bin/ladder7_test"
echo "ladder7_test: EXIT=0"
echo
echo "== P2-HG1 handle-program golden: window registry bound on the crt0 era-window anchors (IMAGE g_sinbd +0x32000 / POOL THeap +0x34000 / ARAM offset-space @pso_aram_base); NJS_HANDLE(T,slot) BYTE-exact round-trip, slot==0<->NULL, ARAM leg + L-FLOOR unbind fail-loud, stamp-at-genesis WRITE-ONCE, and the NEGATIVE CANARY — a planted W-INT stack-derived site MUST loud-reject (gate-can-fail proof, docs/p2-handle-program-1.md §5) =="
"$R/build/bringup/bin/hg1_test"
echo "hg1_test: EXIT=0"
echo
echo "BRINGUP GREEN"
