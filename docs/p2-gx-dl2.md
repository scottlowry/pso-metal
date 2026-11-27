# P2-AQ — display-list slice 2: kill the cy>0 texel-zero bug, FULL 7-point gate

State at landing: **run_dl.sh exit 0 from CLEAN rebuild × 2 on the FULL
original 7-point gate** (4 quad corners + center color-matched to the
addressed texel, 2 outside vs clear; 30 frames × 7 samples, samples_ok=210,
verify_failures=0). The 16-texel diagnostic (`run_dl.sh --debug`) is an exact
identity map. Header era certified: `build/waivers/pso_assert_waivers.h` =
`08fa246e212003fa07342e34206cc0809278877a2ed3cca0cd0f9399a573f1d9` at intake
AND before both final gate runs — no stitch landed mid-lane; if the
integrator's ~99→~800 stitch lands later tonight, the gate should be re-run
once against the new header (nothing in the fix depends on waiver content,
but the certification era is the intake one).

## 1. Reproduced pre-fix state (Z's docs/p2-gx-dl1.md §3)

`run_dl.sh` @ 2e7ba38: 3/3 samples × 30 frames green, exit 0; PPM + direct
point sampling confirm the cy>0 divergence with the ORIGINAL TEX pattern.
CPU-side `GX_DL_TEXP` upload roundtrip: all 64 bytes correct, again.

## 2. Root cause (one paragraph)

**The scalar-coordinate overload `texture2d<float>::read(size_t x, size_t y)`
is miscompiled by this machine's runtime MSL compiler (Apple M3 Ultra,
macOS 27.3, Metal from SDK MacOSX27.2).** It is not texture supply,
storage-mode/binding, y-origin flip, mips, or upload pitch — every suspect on
Z's list was eliminated by direct measurement (16 distinct-color texel
diagnostic: CPU upload table, `getBytes` GPU-texture roundtrip incl.
dims/fmt/levels/usage print, and a 16-cell GPU-sampled map; uv and col
pass-through fragment shaders). The broken behavior is a property of the
fetch instruction path itself: with dynamic coords the entire cy==0 screen
band returns the texture *diagonal* texel(cx,cx) and every cy>0 fetch returns
(0,0,0,0) — even though floor(uv.y*4) measured 1/2/3 correctly in the same
pipeline — and a fully-literal `read(2u, 2u)` on an in-range, verified
texel-returned zero too. The TL corner worked because (0,0) is the diagonal's
fixed point: read(x=cx,y=cx) and read(cx,cy) agree exactly at (0,0). The
vector-coordinate overload `read(uint2(cx, cy))` and `sample()` with the
NEAREST sampler both return the exact 16-texel grid; the shipped fragment now
uses the uint2 form (keeps Z's intent — deterministic texel hit, sampler out
of the failure surface).

## 3. Fix diff summary (host/gx/dl only)

- `gx_dl_metal.m`: shipped fragment `tex.read(cx, cy)` → `tex.read(uint2(cx, cy))`;
  fragment source split into env-selectable debug variants `GX_DL_FSHOP` 0–6
  (uv emit / col emit / sample / scalar-read-no-col / literal scalar read /
  uint2 read) so this class of bug is a one-run repro; added `gxh_dl_sample()`
  (raw N-point drawable readback → packed RGBA) and
  `gxh_dl_tex_roundtrip()` (CPU getBytes table + tex dim/fmt/levels/usage print).
- `gx_dl_driver.c`: `GX_DL_TEX16=1` diagnostic mode — 16 distinct texel colors,
  distinctive clear (3,7,11) to separate "read zero" from "never covered",
  one frame, prints CPU-side upload table + GPU-texture roundtrip + all 16
  cell-center samples classified to texel coordinates. GATE upgraded from the
  honest 3-point subset to the FULL 7-point set (xs/ys/want arrays), 30
  frames PATH A/B alternating, unchanged exit-code conventions.
- `gx_dl.h`: prototypes for the two new backend entry points.
- `run_dl.sh`: documents the 7-point gate; `--debug` runs the 16-texel
  diagnostic as a secondary mode after the gate (own exit line; gate exit 0
  required for the debug leg to run).

## 4. 7-point gate transcript (clean rebuild; identical on both runs)

```
== run: era GP wire replay, 30 frames, 7-point drawable readback ==
gx_dl f00: FIFO(capture) — 7/7 samples MATCH (corners+center+outside)
gx_dl f01: DL  (CP 0x40) — 7/7 samples MATCH (corners+center+outside)
...  (f02–f28 alternating, 7/7 each) ...
gx_dl f29: DL  (CP 0x40) — 7/7 samples MATCH (corners+center+outside)

gx_dl: frames=30 vtxdesc_cmds=30 dl_calls=15 verts=120 tris=60 presented=30
       verify_failures=0 samples_ok=210
-- gx_dl exit: 0
GX DL GREEN: full 7-point textured-quad gate — era display-list words to exact texels
```

Gate points: TL(162,122)→texel(0,0) red · TR(478,122)→(3,0) green ·
BL(162,358)→(0,3) blue · BR(478,358)→(3,3) white · CT(320,240)→(2,2) yellow ·
(100,100),(540,400) = clear black. Run 1 (with --debug): exit 0, all 16
diagnostic cells map to their own texel. Run 2: exit 0. Both on header
`08fa246e212003fa…f1d9`.

## 5. Lesson pinned for the lane

On this toolchain, prefer `read(uint2(...))` / `sample()` over scalar-coord
`read(x, y)` in runtime-compiled MSL; a wrong-texel-zero symptom in a
correctly-uploaded texture can be the fetch lowering, not the plumbing —
diagnose with a distinct-texel fill + literal-coordinate probe before
touching supply code.

## 6. Next-to-logo items (deltas vs docs/p2-gx-dl1.md §5)

1. ~~GPU-visible 4x4 texture fix (cy>0 reads) → full 7-point gate~~ — DONE
   (this lane). §5.1 of dl1 is closed.
2. Real texture supply: PPP/ARC decode → RGBA4/IA8/CMPR uploads beyond
   RGBA8Unorm (data/docs probes live in AK+AO; AN handoff note needed if
   their supply landed — gx_dl consumes `gx_dl_set_texture(w,h,rgba8)` and
   the upload path is supply-agnostic, so a decode front-end can drop in).
3. NJS DRAW_TPL PPC-address payloads (arena translator; GXDisplayList.c:74
   wire already proven on both PATH A/B).
4. FIFO timing: real wrap handling, GXWGFifo watermark + break-points,
   draw-to-disk-sync pacing.
5. XF/mtx engine, full TEV combiner config, alpha compare/blend, index
   buffers (unchanged from dl1 §5.5).
