# P2-SEQ5 disposition — tile-grid seam investigation (card t_b011c961), UNLANDED

Lane SEQ5 (pso-metal BUILD SEAT), fleet wave cards-20261127E, 2026-11-27.
Intake verified same-turn at lane start: pso-metal main @2e069a7277 (tracked-
clean), DECOMP pin 6b38b53837 READ-ONLY, waiver `d4382fcd0659357f283e675726c1
076338e909550c067513459280025e9f0eee` PRE==POST at lane start and at this
disposition (zero make events ran — build events spent 0/3, 55-min cap hit in
ANALYSIS, no repo build entered). Zero regen; gen_waivers/make parse NEVER
run. Board untouched. Never pushed. Footage image used LOCAL-ONLY (never
byte-committed, never copied; nothing beyond its already-receipted stats cited
here). Disc path never printed. NO repo code/doc file was modified by this
lane before this file; no receipts moved (every cmp fnv holds — nothing was
re-decoded inside the repo).

## 1 — Defect REPRODUCED numerically (integrator's measurement space)

Machine-verified by python PNG decode (stdlib zlib port of the same delta
metric) of /tmp/cla/seq_sega_composite.png (640x480, 12487 B, the post-CMPRBE
dump): worst column-edge sums x=366:13623, x=312:13413, x=369:13357,
x=252:12260, x=371:12246; worst row-edge sums y=275:43595, y=263:32186,
y=198:31564 — same structure and order of magnitude as the card's x=384:13124 /
x=255/256:10095/3944 / y=192:6849 / y=256:6844 (the card's px land on canvas
cell-duplicate offsets of the same lines). Defect confirmed on the artifact.

## 2 — Suspect (1) NOT CONFIRMED: no in-tile order yields continuity

Objective seam-minimization run (scratch python, OUTSIDE the repo build tree,
NOT a repo make) over the gitignored local slices (e01..e04 + gc_adx01..04,
payload at record off+0x10, w*h/2 CMPR — loader legs identical to the driver):

- Space scanned EXHAUSTIVELY: all 24 sub-block position permutations x 8
  in-block index-fetch laws (byte axis {y,3-y,x,3-x} x shift axis {2x,2(3-x),
  2y,2(3-y)}); endpoints fixed at the CMPRBE law (BE u16 RGB565, opaque-black
  index-3). Metric: mean |luma delta| across tile-internal 4px grid lines
  (x%8==3 / y%8==3) vs non-grid lines, per 256x256 slice.
- Result: EVERY candidate stays ratio >= 1.21; the current documented law
  measures 2.13 (mean over the 8 content slices). NO order reaches the ~1.0
  continuity the card's suspect (1) requires ("glyph strokes continuous across
  sub-block boundaries"). The hot 4px-grid signature is INVARIANT under all 8
  in-block laws (they permute pixels only INSIDE 4x4 blocks) and no position
  permutation both minimizes the metric AND preserves the human-verified
  glyph structure (L8-P4/PICK-2 board).
- Era-literature check (same-turn, external doc YAGCD "Yet Another Gamecube
  Documentation", CMP section): "CMP ... 2x2 blocks of 4x4 tiles ... sub-tiled
  into four parts, in zigzag order: 0 1 / 2 3" — that IS the landed
  placement (rm = bx + 2*by). The doc is silent on in-block bit endianness.
- DECOMP pin grep: no CPU-side TCMP decoder exists in the era source
  (GXTexture.c/GinjaMaterial.cpp name CMPR only as HW format selection) —
  no era body can PROVE an alternative order; era truth for the bit layout
  remains UNPROVEN, exactly as the card flagged.
- Partial signals banked (below): (a) the BE-word reading of the 4 index bytes
  (byte 4+y, shift 2*(3-x) — same PPC big-endian regime that CMPRBE PROVED for
  the endpoint halfwords) MINIMIZES both the seam ratio and the repo's OWN
  coherence instrument on ALL FOUR gc_adx content slices (coherence
  0.121/0.156/0.135/0.217 vs current 0.166/0.187/0.162/0.258; seam
  1.43/0.97/1.56/1.53 vs 2.48/1.60/1.87/2.19) — but it does NOT fix the SEGA
  wordmark composite (band tile-edge 1346->1058, internal grid 1208->1371:
  mixed, gate-clean NOT reached). (b) a transposed in-block law scores lowest
  on SEGA slices but contradicts the DXT1-like sub-tile docs.

DISCIPLINE CALL: landing any order swap as "era truth" on this evidence would
repeat the bug-made-law failure CMPRBE just retired (self-consistent gates
passing for days under a wrong law). The card's gate design ALSO cannot be
honestly built yet: its bound "tile-grid line deltas <= MEDIAN glyph-edge
delta" is vacuous on this art (measured median non-grid line delta = 0 — a
~94% white field), so EVERY candidate law would pass or fail on noise.
Therefore NO code change, NO receipts re-anchor, NO composite re-dump: an
unchanged law re-dump would reproduce the identical artifact (zero value,
one build event). Refusal disclosed with numbers, not silence.

## 3 — What remains genuinely open (ranked by measured residual)

1. Composite x=255/256 + y=256 SLICE-BOUNDARY seams (card 10095/3944/6844):
   these are seams BETWEEN independently-encoded 256x256 slices (e01|e03,
   e01|e02). If the era art is continuous THERE, no in-slice order can fix
   it — it would require the era slices to OVERLAP or the quad texcoord
   spans to be fractional (SEQ L1 v1=v0+... laws — suspect (2)/(3) legs the
   card ordered 2nd/3rd; NOT yet examined this lane, time cap hit).
   Measured: in THIS decode, glyph rows 204-275 cross y=256 at slice
   bottoms — check era TAdSEGAlogo quad texcoord/vertex spans vs the 640x512
   layout BEFORE any order change next attempt.
2. BE-word in-block endianness (suspect (1) cousin): strongest cross-art
   signal (all 4 ADX slices improve on repo coherence), zero cost to test as
   a canary first (gxh_cmpr_block_* pattern), does NOT resolve the SEGA
   wordmark grid alone.
3. The card's premise "in-tile 4x4 order prime suspect" is REFUTED as a
   COMPLETE explanation by the exhaustive scan — whatever the fix is, the
   era art itself likely carries a real 4px-scale texture (a designed
   mosaic), so any future seam gate MUST be calibrated against the FOOTAGE
   (the only era-truth witness with the presented result), not against
   intra-decode smoothness.

## 4 — Banked gate spec for the next attempt (drop-in, unlanded)

L9-S1 SEAM-CONTINUITY: over the accepted-era native composite, restricted to
GLYPH rows (rows whose edge-sum > k*image-median, k=4 — the 0-median trap
above), gridline mean delta <= GLYPH-EDGE median (non-grid edge-line deltas
restricted to the same rows) * 1.35; canary = present the SAME raster under
the superseded law through l9_alt_law_present (instrument exists,
gx_logo_driver.cpp:432-467) -> gate MUST reject (teeth). Measured baselines
for threshold calibration (wordmark band cols): current grid 1207.7 /
tileedge 1346.0 / other 380.1.

## 5 — Lossless receipt table (<=10 rows)

| item | value |
|---|---|
| intake | main @2e069a7277 verified same-turn; DECOMP pin 6b38b53837 RO |
| waiver | d4382fcd0659357f... PRE==POST (lane start + disposition); zero regen; 0 make events |
| build events | 0/3 spent; 55-min cap hit in scratch analysis (no repo build entered) |
| defect | confirmed on PNG: worst cols {366:13623,312:13413,369:13357,252:12260}; rows {275:43595,263:32186,198:31564} |
| suspect 1 | REFUTED as complete cause: 24 perms x 8 in-block laws scanned, best ratio 1.21, current 2.13, none continuous; YAGCD zigzag '0 1/2 3' = current placement |
| banked (a) | BE-word idx (byte 4+y, shift 2(3-x)): coherence BETTER on all 4 ADX (0.121/0.156/0.135/0.217 vs 0.166/0.187/0.162/0.258); SEGA composite NOT clean |
| banked (b) | transposed in-block law: lowest SEGA ratio ~1.51, contradicts DXT1-like docs |
| open | slice-boundary seams x=256/y=256 (suspects 2/3, texcoord-span check) NOT reached at cap |
| gate spec | banked §4 (glyph-restricted, footage-calibrated; naive median bound vacuous: median=0) |
| board | untouched (t_b011c961); never pushed; no repo file changed except this doc |

Files this lane wrote: docs/p2-seq5-disposition-1.md ONLY (claims==stat).
