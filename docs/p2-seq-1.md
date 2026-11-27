# P2-SEQ — SEGA-logo exit semantics: era model + GX_FRAMES_EXIT host leg (card t_02f43e93)

Lane SEQ (gx/dl BUILD SEAT), fleet wave-5.5b, 2026-11-27. Re-derivation:
prior attempt killed 82s in, zero artifacts; this run started from verified
intake (pso-metal @573ba98, DECOMP_ROOT=pso-decomp-port-pin @6b38b53837,
waiver `d4382fcd0659357f` PRE==POST across every make event, zero regen).
Disc untouched, bytes never committed; fnv/dims receipts only; disc path
never printed.

## 1 — ERA MODEL (L1 evidence, all @6b38b53837)

**The index arg is a FAMILY BASE, not a slice selector, and not a
slice-over-slice animation: `render_logo_with_default(list, tex_base, -1,
-150)` = `draw_title_background_quads(mode 0)` draws EIGHT quads in ONE
frame** — each static tile `title_background_tiles[i]` (i=0..7) samples
slice `tex_base + tiles[i].tex` (TitleBackground.cpp:18–52,58–60; the call
site TAdSEGAlogo.cpp:67). For SEGA (base 8) that is `sega_logo_e01..e08` =
list indices 8..15, laid out as **four 256×256 tiles covering (0,0)..(512,512)
plus a right strip of four 128×128 tiles at x=512..640** (e01 top-left, e02
lower-left, e03 upper-middle, e04 lower-middle, e05..e08 top-down at x≥512);
the 640×512 layout clips to the 640×480 VI field (e08 loses 32 rows). ADX
uses base 0, Sonic base 16 (TAdADXlogo.cpp:33, TAdSoniclogo.cpp:30) — same
descriptor, same tile table; TLogo::run_task chains SEGA→Sonic→ADX
(TLogo.cpp:169–189).

**The only per-frame animation in the SEGA-logo scene is the TScreenFade
black overlay alpha.** The ctor spawns `TScreenFadeIn(tl_00,
request_destroy)`; both fade ctors IGNORE the parent arg and attach under
`tl_fade` (TScreenFadeController.cpp:61,85 + the header comments), which
renders AFTER the `tl_00` scene sequencers (TMainTask.h:48/65), so the
overlay composites over the logo. Overlay = full 640×480 polygon,
q.tex=NULL, black RGB, per-vertex alpha=fade value, skipped when alpha==0
(TScreenFadeController.cpp:30–59); blend pair (0,8)/(1,6) ==
`set_default_alpha_blend` (render_util.cpp:286).

Tick ledger (f32 sims re-run by the host gate, L9-H1):
- **Fade-in**: alpha 255 → −8/tick → 32 ticks → `request_destroy`
  (queue_destruction + state=1, TAdSEGAlogo.cpp:61–64).
- **Hold**: state 1, `(f32)++m_wait_frames > 60.0f` → **61 ticks**
  (TAdSEGAlogo.cpp:74), scene fully visible, alpha 0 (no overlay drawn).
- **Fade-out**: `TScreenFadeOut(tl_00, destroy_and_mark_done)`, alpha 0 →
  +8/tick → 32 ticks (clamps at 255) → `sega_logo_done_flag=1`
  (TAdSEGAlogo.cpp:56–59), consumed by TLogo state 4 (:60/:170).
Era tick = 30 Hz: `set_present_interval(cfg->present_interval=2)`
(gjSystem.c:34,108–116) gates presents on `VIGetRetraceCount` (+1)
(gjSystem.c:71) → 480i double-field retraces ⇒ update gate ≈ 30 Hz
(boot-spine.md S7, §2 item 2). Whole scene ≈ (32+61+32)/30 ≈ **4.17 s**.

## 2 — HOST LEG GX_FRAMES_EXIT (L2/L3, gx_logo_driver.cpp L9)

Regime gate: `GX_FRAMES_EXIT=1` + `GX_LOGO_GVR` + `GX_FRAMES_GVM` (rides the
genuine frames regime; unset ⇒ zero L9 output/counts). Entry composite stays
the L8-P2 entry-frame regime; L9 adds **6 legs**:
- **L9-C1** mosaic law (tile table verbatim; indices 8..15 each exactly
  once; disjoint; 327680 slice texels; full 640×480 coverage after clip).
- **L9-C2** native-res CPU composite: 307200/307200 visible texels ±0 vs
  own CMPR expand; per-stage slice fnvs runtime-self (e01
  0xEE87BFB38B91E58A, e02 0x3785B7088F6B04C3, e03 0xF6E6A862D6F395D3, e04
  0x9F9221D16DC8EE78, e05–e07 0xB9F13A0AA87F2325 ×3 dup group, e08
  0x15B37D7ED9BA6325); screen fnv 0xB577D9FE441348AF.
- **L9-G1** GPU entry present: 256² canvas at max-margin cell px through the
  TEXN=256 uint2 read map, 65536/65536 ±0; readback fnv == canvas fnv
  0x52EE91728CDBF55E; PNG `/tmp/cla/seq_sega_composite.png` (12589 B).
- **L9-H1** ledger DERIVED by sim {32,61,32}; 61-tick hold re-presents
  scene-static (64-pt grid ±0 every tick, fnv 0xEE47497412F05F2A stable
  t=1/31/61); era-vs-host pacing print.
- **L9-F1** fade-out leg: alpha=min(8t,255) per tick, GPU readback == CPU
  model ±0 at t=16 (0x458EDE2334475DC4) and terminal all-RGB-black
  (0x1228EAD37DDA2325); done-flag set at clamp tick; PNG
  `/tmp/cla/seq_sega_faded.png` (4181 B, the terminal fully-faded frame).
- **L9-K1** canaries FAIL AS REQUIRED: base-0 (ADX) composite differs on
  14447/65536 cells (the dup-group slices keep the strip identical — honest
  number); 30.0f-limit hold derives 31 ≠ 61; +1.0f/tick fade reaches only
  32/255 by the era's 32nd tick.

DEVIATIONS — **SEQ-1** pacing: host tick = one immediate CAMetalLayer
present per render_tick (no retrace wait; FRM-3 lineage) — tick COUNTS
identical, wallclock differs. **SEQ-2** blend: the host pipeline has no GPU
blend stage, so the black overlay composites CPU-side
(`out=(c*(255-a)+127)/255`, `a'=a+(da*(255-a)+127)/255`); the era blend
equation itself is pinned by the (0,8)/(1,6)==set_default_alpha_blend cite —
the GPU legs verify upload+read pass-through ±0, not a hardware blend.
**SEQ-3** display: native mosaic proven CPU-side (L9-C2); presentation goes
through a 256² canvas at margin-safe cell px (min uv·256 boundary distance
0.2 texels x / 0.267 y — verified bijective over all 512 cell coordinates)
because the shipped pipeline is a fixed TEXN grid. One instrument bug caught
by the gate en route: u32-word canvas cast directly to bytes uploaded
byte-swapped RGBA → 3932/65536 mismatches; explicit byte staging fixed it
(30/30 rc=0 after).

## 3 — Gate table (rc=0 everywhere; waiver d4382fcd0659357f PRE==POST at all
3 make events; zero regen; DECOMP_ROOT pinned 6b38b53837)

| run | result |
|---|---|
| era FIXTURE (no hooks) | 13/13 PASS |
| AV-1 stand-in | 10/10 PASS |
| era GENUINE (sinbd+hyouji_a) | 13/13 PASS |
| GENUINE FRAMES (24/24 staying byte-green) | 24/24 PASS |
| **FRAMES + EXIT (13 L7 + 11 L8 + 6 L9)** | **30/30 PASS** |

## 4 — preview.sh compatibility (parent-export note; files untouched)

The EXIT legs ride the existing `logo-genuine` subject with the frames
envs PLUS one export, zero preview edits:

```sh
export GX_FRAMES_GVM=$PWD/host/bringup/local_assets/logo_e.gvm
export GX_FRAMES_SLICES=$PWD/host/bringup/local_assets/logo_e.slices.json
export GX_FRAMES_EXIT=1
PREVIEW_HOLD=20 OUTDIR=/tmp/prev-exit host/gx/preview/preview.sh logo-genuine
```

The last-presented frame under that combo is the terminal fade-out frame
(all black) — the integrator may want a hold-subject variant later; nothing
here blocks it. Direct runs used `/tmp/cla/seq_r*.log` + `/tmp/cla/f*.log`
(machine-local, not committed).

Files this lane wrote: host/gx/dl/gx_logo_driver.cpp (L9 block +
sys/stat include), host/gx/dl/gx_dl_metal.m + gx_dl.h (gxh_dl_dump_png
seam only), docs/p2-seq-1.md, build/gxdl/** (gitignored). No era file touched;
no fake_disc, fixture, parse/gen_waivers, README, or STATUS change.
