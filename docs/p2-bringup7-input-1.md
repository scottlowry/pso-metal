# P2-BRINGUP7 input-seat finish — receipts + lossless disposition (card t_ab192a31)

Lane BRINGUP7 (pso-metal BUILD SEAT), 2026-11-27. Completes BRINGUP6
(`37c6f4f`): the staged 15/17 input seat, its exact harness fix spec, and the
lossless HOLD list. Intake verified same-turn: pso-metal `main` @`9621a95`,
tracked-clean (untracked `build/**` only). DECOMP_ROOT
`/Users/scott/Hermes/pso-decomp/pso-decomp-port-pin` @`6b38b53837` READ-ONLY,
re-verified every event. Waiver `host/bringup/waiver-header.sha256` pin
`d4382fcd0659357f…`: PRE==POST on all 3 make events + both re-attests
(verify-only; `gen_waivers`/`make parse` NEVER run; zero regen — regenerated
slice manifests byte-identical: input_slices.sha256 `6b0ac02dde87…`,
vi_era_slice.cpp `9ca721d48169…`). Disc path never printed; no asset bytes.

## Landed (commits on main, never pushed)

| step | commit | content | gate evidence |
|---|---|---|---|
| 1 | `3d623d0` | hg3_input harness fix: `pso_input_injected[4]` shadow is THE PADRead source; `pso_input_pad_*`+init write the shadow, shim copies injected→out; `pad_status` written ONLY by PADRead (hardware-faithful). Test-API only, ZERO era-slice change | `run_input.sh` rc=0, **hg3_input 17/17**; TEETH PROVEN LIVE: scratch negative probe (37c6f4f pre-fix `input_state.cpp` compiled outside the repo build tree against the SAME green glue/test objects) → 15/17 FAIL rc=1, red on EXACTLY C0c teeth + C1 press-visibility |
| 2 | `d920ed9` | R-D VI def retirement (gx files granted by BRINGUP7): `vi_era_harness.h` 0x190 wrong-shape twin block REMOVED → `#include "pso_input.h"` (era 0x1B0, LAYOUT_ASSERTs live); `vi_branch_driver.c` bank def REMOVED, return-0 stub → forwarder to VERBATIM `pso_era_test_previous_buttons_packed` (zero-init bank PRODUCES the same A3 era-zero answer); `camera_ctrl_instance` host def (NULL, IN0-3); `vi_branch.mk`: seat include regime + `$(INPUT_OBJS)` both binaries + `-lm`, DECOMP default moved to the pin | `run_vi.sh` rc=0, **all 7 PROOF keys PASS** (both binaries, all routes); slice file byte-identical pre/regen (no fnv drift → no red-stop); post-step census: `ControllerInputState` defs = input_state.cpp ×2 ONLY, `u8 raw[0x50]` grep host/gx = zero |
| 3 | `08b0c15` | Pump flip: `ladder7_seam.cpp` `controller_stuff` fires `pso_input_pump_frame()` iff `pso_input_pump_live` (DEFAULT 0; kbd seam keeps note+counter — ONE read_pads/frame law, deviation vs §3.4 wording disclosed); `controller_reset_requested` def moved seam→seat (nm-proved C-linkage duplicate `_controller_reset_requested`); prelude gets forward decls ONLY (bringup `TCameraCtrl` carrier must not meet the era graph; bringup's existing `_camera_ctrl_instance` satisfies the glue ref); `bringup.mk` links `$(INPUT_OBJS)` + `-lm` into ladder7/hg3_walk/hg3_trial; `run.sh` INSERT-ONLY hg3_input section before GREEN | **FULL `run.sh` rc=0 BRINGUP GREEN**: boot_spine 0 · prs 16/16 · ladders 13/26/27/28/18 · ladder7 62/62 · HG1 29/29 · HG2 270/270 · hg3_walk 26/26 · hg3_trial 41/41 · hg3_input 17/17 (counts grepped from the redirected log; rc-from-redirect law). `run_vi.sh` re-attested on the exact committed tree (7 PROOF keys). Windows/AV-1: no link-line contact — grep proves no `host/gx` TU includes `vi_era_harness.h` outside the vi set and none includes `ladder7_run_prelude.h` |

## W-POOL row-2 audit — banked formula MEASURED (live-handle stamp still OPEN)

37c6f4f banked: trial-window row-2 take =
`0x1E*sizeof(TWindowMessageLine) + sizeof(WindowScrollBar) + shared DrawContext
+ window object headers` vs the 16 KB span. Sizes read from the pin headers
same-turn, each cross-checked against its own era `LAYOUT_ASSERT` and against
pointer-bearing fields (scratch sizeof probe refused by the D4 include policy
at `NJS_OBJECT_size` — era-header asserts used instead):

| component | size | era assert (file:line) | pointer-free? |
|---|---|---|---|
| `TWindowMessageLine` {s32 flags; s32 timer} | 0x8 | (×0x1E rows ⇒ 0xF0, TWindowMessage.cpp:340) | yes |
| `WindowScrollBar` {NJS_POINT2 extent; s32 frame} | 0xC | WindowScrollBar.h:25 | yes |
| `WindowDrawContext` (19 f32 + 2 int-shaped, shared singleton) | 0x4C | WindowDrawContext.h:56 | yes |
| `TWindowMessageLog` object header (derives TWindowMessage 0x90) | 0x98 | TWindowMessageLog.h:33 | vptr/ptrs — era width governs |

**Take = 0xF0 + 0xC + 0x4C + 0x98 = 0x1E0 (480 B); headroom vs 0x4000 span =
0x3E20 (15,904 B) — 2.93% used.** The trial window alone cannot approach the
row-2 ceiling with the banked components; the residual that only the
HG3-284 ctor leg can stamp LIVE: `WindowTextEntry` allocations made by
`append_line` (text storage per line is NOT in the banked formula) and the
`TWindowCtrlMain`/`Screen`/`Sub`/`ClipOut` control objects. Carried below.

## HOLD / UNLANDED (lossless)

- **HG3-284 ctor leg NOT started** — build event 4 intentionally NOT spent:
  the window-chain ctor needs the whole TWindowCtrl/TWindowBase chain
  (RS §1 T7-S1/T7-S9: class (c)/(d), MISSING) + new()/mem_alloc + W-POOL
  handle regime — a multi-event job; starting it at cap could only bank a red
  half-leg. Next lane: author from RS T7-S1 with the §3.3 injection API now
  LIVE (`pso_input_pad_axes(0,0,0,d)` per HG3-286 note); stamp the LIVE
  row-2 handles incl. the WindowTextEntry residual above.
- HG3-285/286/287-state-machine/288/289/290 legs UNLANDED (rung-7 card 2 =
  BRINGUP-7-as-specced in RS §3 rides after; `hg3_trial` untouched 41/41).
- `pso_input_pump_live` has NO flipper yet (flag-off byte-green is what
  shipped; first flipper = HG3-286 probe; with-flag-ON era-idle equivalence is
  covered by construction: hg3_input C0a runs the same pump ×3).
- `gx_logo_driver.cpp:88-89` counted stubs UNTOUCHED (separate link scope;
  §3.4 step-3's dl-tree wrapper promotion = gx dl lane's call).
- README suite-row update (hg3_input row) = integrator's per-lane pattern.

Cost: **3 build events of the 4 budget** (1 seat-fix gate + scratch teeth
probe, 2 VI retire + gates, 3 pump-flip + full run.sh + VI re-attest);
event 4 SPARED. One scratch sizeof probe (no repo make, D4-refused,
disclosed). host/bringup/fake_disc, parse*/gen_waivers, README/STATUS,
host/gx/dl, host/gx/window files untouched; never pushed; board untouched.
Claims==stat per commit (`git show --stat` each).
