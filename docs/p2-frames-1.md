# P2-FRMB — genuine logo_e FRAMES: CMPR expand leg + L8 gate (card t_12a35f0c)

Lane FRMB (gx/dl BUILD SEAT), fleet wave-5.5d, 2026-11-27. Inputs read in
card order: `docs/p2-oracle-1.md` §5 (format resolution — **fmt 0xE =
GX_TF_CMPR**, manifest "I4" labels SUPERSEDED; fmt 9 = CI8 TI8x4 fixed and
landed at 964272c), `docs/p2-frames-spec-1.md` (F0–F7, 0xE override applied
to its I4-leg sections), kanban `t_68144c7b/report.md`+canaries.json (bank
map — no GVPL legs apply to logo_e), `docs/p2-logo-e-supply-1.md` §S2.
Decomp READ-ONLY via pin `DECOMP_ROOT=…/pso-decomp-port-pin` @6b38b53837
(live tree at 620fdbed8 NOT used — anchor-drift bait per card). Disc
untouched; asset bytes never committed (fnv/dims receipts only); disc path
never printed.

## 1 — Same-turn supply verification (F0 re-established on THIS machine)

`host/bringup/local_assets/logo_e.gvm` (gitignored): size **0x78530**
(492,848 B), fnv1a64 **0x1C29B433471C69A8** ✓ (python, same turn as build).
Header: `GVMH` / hdr.size 0x3a8 LE / flags 0x000F BE / count 24 BE ✓.
Chunk walk from 0x3b0 (`off += LE(+4) + 8`): 24 GVRT, **end 0x78530 == file
end** ✓. All 24 slice fnvs re-hashed from the file and equal the F5/§S2
table 24/24; dir attr==index, names, fmt 0x000E, GBIX == F2 pins
{0x1B0A8,0x1AFC2,0x10CAB4}+k ✓ 0 mismatches (set-math in python).
Dedupe groups measured: **16 distinct**, `0x5B73ADEAFF301D0C` ×7
{06,12,13,14,20,21,22}, `0xC8D2B06B7CF1BB0C` ×3 {07,15,23} — F0 correction
standards, not the manifest's ×6.
Runtime-reference sidecar written to `local_assets/logo_e.slices.json`
(gitignored; archive fnv + 24 {name,fnv}). The driver TU compiles with
**zero genuine hashes**: every expected value loads at runtime (F6.1).

## 2 — CMPR expand leg (gx_logo_clut.h, DEVIATION FRM-1)

`gxh_cmpr_block_rgba8` + `gxh_cmpr_expand_ti8x8`: GX_TF_CMPR = DXT1 blocks
with **RGB5A3 endpoints** (shared decoder `gxh_rgb5a3_rgba8`, extracted from
the fmt==2 TLUT leg — one decoder serves both), TI 8×8 addressing
(`off = 32·((x>>3)+(w>>3)·(y>>3)) + 8·(bx&1 + 2·(by&1))`, sub-blocks
row-major = the surviving rm probe), endpoints u16 **LE** in-block, index
bits `2·(x&3)` of byte `4+(y&3)`, interpolation `c0>c1 ? [c0,c1,⅔,⅓] :
[c0,c1,½,transparent]`. Era truth: GX hardware TCMP at DRAW time
(GXTexture.s jumptable idx 14, `GinjaMaterial.cpp:372,390,423`
`format == GX_TF_CMPR` gate); the era never decodes 0xE on host — the host
CPU-expand is the display-path deviation class, pinned in-source with the
oracle witnesses. Planted wrong-decode CANARY `gxh_i4_ti8x8_gray_expand`
(the superseded manifest "I4" reading), gate-instrument like L7-P4's flat
canary.

Texel anchors vs owner-verified oracle tiles (sega_e01/e02, board
`/tmp/orcl/grid.png`): python prototype of this EXACT law reproduced the
oracle coherence values — **sega_e01 0.1588 (oracle 0.159), sega_e02 0.2410
(oracle 0.241)**, I4-TI canary 0.8352/0.8343 (oracle 0.835/0.834).

## 3 — Hooks + descriptor + L8 legs (gx_logo_driver.cpp)

Per spec F6, mirroring the GX_LOGO_GVR regime:
- `GX_FRAMES_GVM=<logo_e.gvm>` + `GX_FRAMES_SLICES=<logo_e.slices.json>`;
  parked at arena `+0x1680000` (0x80000 budget, clear of the
  `+0x1600000/+0x1640000` gvr/gvp sites); hook unset ⇒ zero L8 output,
  zero count (CI defaults byte-identical). L8 requires the genuine sinbd
  supply (`GX_LOGO_GVR`) for the 256² TEXN/quad regime — the frames-only
  combo prints a loud SKIP note and does not count.
- `register_texture_logo_e` path: 24-entry descriptor with era strings
  (F1), GBIX pins (F2 — era identifiers, compile-time legal, not hashes),
  three-handle identity (F3), driver-equivalent walk (permitted by I4
  §S4): dir stride 0x26 BE decode, GVRT LE-size walk, `chunk->size-8`
  payload copies to **supply-side malloc, never the era cache heap**
  (I4 §S4.4), tlut −1 binds (palette legs asserted skipped), dedupe ×7/×3
  in the GATE only (era keeps 24 separate surfaces — all GBIX distinct).
- DEVIATION FRM-2: host-resident descriptor mirror (the era
  NJS_TEXNAME/NJS_TEXLIST are u32 image-handle slot space under P2-AR;
  this descriptor never crosses the AR boundary).
- DEVIATION FRM-3: the two frames draws ride the SAME era render_tick
  replay (AQ display path) after the L7 era-chain legs complete — a
  second present in-process, replay bodies untouched.
- DEVIATION FRM-4: F6.5's "escape hits == 0" was pinned under the
  superseded I4 (opaque) reading; CMPR mode `c0<=c1` index 3 IS
  genuinely transparent. Escape clause counts are asserted, printed, and
  honestly non-zero: **adx 2 / sega 61** (python census agrees: 61 a=0
  texels in sega_e01).

## 4 — Gate results (rc=0 on every run; waiver `d4382fcd0659357f`
re-hashed at both ends of the build session, PRE==POST, never regen;
2 make events, `logo` + `logo-av1` targets, DECOMP_ROOT pinned)

| run | result | receipts |
|---|---|---|
| era FIXTURE (no hooks) | **13/13 PASS** | byte-identical leg set to 964272c (L8 silent) |
| AV-1 stand-in | **10/10 PASS** | fixture regime untouched |
| era GENUINE (sinbd+hyouji_a, no frames hook) | **13/13 PASS** | coherence TI 0.2763/CANARY 0.7164 (unchanged) |
| GENUINE FRAMES (sinbd + `GX_FRAMES_GVM` + slices) | **24/24 PASS** = 13 L7 + 11 L8 | L8-S1/S2/W1/W2/X1/D1/P4/P1-P2/P3/P5/H1; drawn readback `gc_adx01` + `sega_logo_e01` ±0 over 65536/65536 cell centers each; readback region fnv adx `0xC85BCC16827B9577`, sega `0x5A3A5AF5154503EC` (runtime-self anchor, not compiled); coherence CMPR **0.1589** vs planted I4-TI CANARY **0.8321** (oracle 0.159/0.835; baseline shuffle differs from python by construction) |

L8 leg inventory: S1 supply receipt (size+fnv vs sidecar) · S2 walk
end==file end · W1 dir/descriptor/GBIX · W2 palette-skipped tlut −1 ×24 ·
X1 24/24 slice-fnv vs sidecar · D1 dedupe 16/×7/×3 · P1/P2 draw+readback
±0 + full coverage · P3 hash-posture (distinct regions, no compiled
expectation) · P4 CMPR-vs-I4 coherence gate (canary must fail) · P5
128² byte law `w*h/2` (CPU; the 256 TEXN shader variant cannot address a
128 grid — draw regime is 256² only per BB-4) · H1 handle identity +
post-free NULL with empty twins.

## 5 — preview.sh compatibility (files READ-ONLY for this lane; invocation
notes for the integrator)

`preview.sh` only exports `GX_LOGO_GVR/GVP` for the `logo-genuine` subject,
but the driver inherits the parent environment, so frames ride the existing
subject with NO preview edits:

```sh
export GX_FRAMES_GVM=$PWD/host/bringup/local_assets/logo_e.gvm
export GX_FRAMES_SLICES=$PWD/host/bringup/local_assets/logo_e.slices.json
PREVIEW_HOLD=20 OUTDIR=/tmp/prev-frames host/gx/preview/preview.sh logo-genuine
```

Each iteration draws sinbd (era chain) then gc_adx01 + sega_logo_e01; the
final presented frame is sega_logo_e01 (last render_tick) — the PNG shows
the genuine SEGA-mark frame at the 256² 1:1 quad. `GX_DL_DEBUG=1`
with `failures==0` dumps the same backbuffer to `/tmp/logo_dump.ppm`.
Integrator may add a `frames-genuine` subject later; nothing here blocks it.

## Deviations introduced
FRM-1..FRM-4 above (all era-truth-documented, texel-anchored against the
owner-verified oracle tiles). No new gitignored asset bytes beyond the
machine-local slices.json sidecar + README line. No fake_disc change, no
fixture-regime change, no writes outside host/gx/dl/**, docs/, the
gitignored local_assets sidecar+README, and build/gxdl/** (gitignored).
