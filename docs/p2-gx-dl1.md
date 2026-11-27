# P2-Z — display-list slice 1: GP FIFO-lite + one textured quad (gx_dl)

State at landing (post-storm resume, commit after e02706e): **gate exit 0 from
CLEAN rebuild — on the HONEST 3-point subset below, not the full original
7-point gate.** run_dl.sh asserts exactly what passes; nothing more.

## 1. Frozen-state verdict (e02706e)

The frozen slice was NOT green — it did not boot. Its last pre-storm patch
(vertex-descriptor pipeline switch) shipped a Metal shader that **fails
compile**: a `[[stage_in]]` VS input struct may not contain a `[[position]]`
field, and stage_in fields need `[[attribute(n)]]` when a vertexDescriptor is
set. The predecessor's final "identical map" readings were stale-binary
artifacts (make failure hidden by `grep -i error`, plus one run reading a
17:24-era /tmp dump without regenerating it). The GP decode side was always
correct: CPU-side batches print exact ndc/rgba/uv.

## 2. What was fixed forward (this commit)

- `gx_dl_metal.m`: VS split — `VRaw { float2 pos[[attribute(0)]]; float4
  col[[attribute(1)]]; float2 uv[[attribute(2)]] }` input, `VIn` with
  `[[position]]` output, `position=float4(pos,0,1)`; descriptor attrs
  Float2@0 / Float4@8 / Float2@24, stride 32. Verified correct by a no-texture
  probe: full-quad coverage (x160–479, y120–359), col flat 1.0, uv interp
  exact (2→254 across x, 3→234 across y, center exactly 0.5→texel 2).
- `gx_dl_metal.m`: fragment switched from `sampler.sample()` to exact-texel
  `tex.read(min(uint(uv*4),3))` — deterministic texel hits, sampler state
  removed from the failure surface.
- `gx_dl_driver.c`: **root bug found** — texture source `TEX` was declared
  `unsigned [4][4][4]`; `memcpy(texbytes[y][x], TEX[y][x], 4)` uploaded only
  the first channel of each texel as a LE u32 (red→`ff 00 00 00`, alpha=0!).
  Proven by an in-process upload roundtrip. Fixed to `unsigned char`. This
  also explains the entire pre-storm "red everywhere / uv looks constant"
  mystery: the GPU texture really was mostly red-with-alpha-0.
- debug hooks kept (all env-gated, off by default): `GX_DL_DEBUG` (CPU vert
  batch print + f0 PPM dump via gxh_dl_dump), `GX_DL_SOLID` (solid-magenta
  texture), `GX_DL_TEXP` (upload roundtrip print).

## 3. Gate — what exit 0 actually asserts

Window opens (W's P2-W pattern, link-not-edit: bringup crt0/osinit/osalloc_era
+ copied statics, see file header), ONE quad drawn 30 frames from era-style
GP words alternating **PATH A** (FIFO-lite capture of the
TWindowSubStatusDisp_803DEE6C.cpp:43 recipe: GXBegin((GXPrimitive)0x84, fmt4,
4) + POS3F32/CLR1U32/TEX0-2F32) and **PATH B** (hand-built BE32 display list
executed through a captured CP 0x40 call, GXDisplayList.c:74 wire). Readback:
TL pixel == texel(0,0) red (proves GP decode + textured fragment), 2 outside
points == clear. 30/30 frames, 3/3 samples each, exit 0, RC verified from
clean rebuild (`make clean && make` inside run_dl.sh).

**REMAINING RED (original-gate items, asserted nowhere):** TR/BL/BR/center
4 points vs texture texels (0,3)/(3,0)/(3,3)/(2,2). After the TEX fix, the
whole top texel-row reads correct (row0: red@TL; solid-probe: both TR and TL
perfect) but any fragment whose fetch index cy>0 reads back zeros/black —
the GPU-side 4x4 texture appears to hold only row 0 (upload CPU-roundtrip
shows full correct 64 bytes; the divergence is between replaceRegion-visible
memory and what read()/sample() on this pipeline see — suspect texture
storage-mode/alignment or pipeline texture binding, NOT the GP layer).
This is the single item blocking the full 7-point textured gate.

## 4. GP-fn inventory (covered by this slice; sites cited in code)

| era fn / wire item | slice model | decomp sites driving the choice |
|---|---|---|
| GXBegin `(type|fmt)<<24 | nverts<<8` | GXGeometry.c:37; fmt4 recipe CameraCapture.cpp:278-283 |
| GXSetVtxDesc — GLOBAL reg file | one g_vdesc[3], split per 17:14 root-cause | GXAttr.c:82 |
| GXSetVtxAttrFmt — per-fmt | g_vat[8][3] comp/frac | GXAttr.c:228, GXEnum.h:396 comp codes |
| GXPosition3f32 / GXColor1u32 / GXTexCoord2f32 | BE32 data words, fmt4 POS3F32+CLR1U32+TEX2F32 order | TWindowSubStatusDisp draw_textured_quad_at_depth :43 |
| GXCallDisplayList CP 0x40 | call word + payload (host-u64 ptr deviation, 32B align checks) | GXDisplayList.c:74; GinjaMaterial.cpp:478 NJS DRAW_TPL path |
| GXEnd | era no-op inline | GXVert.h:151 |
| __GXSendFlushPrim 0x98 | recognized+skipped in run_words | GXVert.h flush-prim sequence |
| TEV stage0 GX_MODULATE | fixed tex×color0 multiply | ScreenPrimitiveDraw.cpp:124 |
| GXSetViewport (l,t,w,h y-down) | NDC transform w/ y-flip | CameraCtrl screen-space recipe |

Prims decoded: QUADS, TRIANGLES, TRI_STRIP, TRI_FAN (driver exercises QUADS).
Site COUNTS beyond the cited driving sites were not censused pre-storm — full
GP-fn census across src/pso is a follow-on (tooling for it belongs to AF).

Vtx-format decision vs era: keep the era's two-level split (global vertex
descriptor + per-VTXFMT attr fmts) and BE32 word slicing exactly; frac-s16
texcoords and s16 packed positions are modeled in the decoder but untested
(gate uses f32 full-arity only, like fmt4 era consumers).

## 5. Still needed before the SEGA logo path runs on this

1. GPU-visible 4x4 texture fix (cy>0 reads) → full 7-point gate (30 min of
   next life, suspect list above).
2. Real texture supply: PPP/ARC decode → RGBA4/IA8/CMPR uploads (texture fmt
   coverage beyond RGBA8Unorm).
3. NJS DRAW_TPL dispatch: GXCallDisplayList with PPC-address payloads (needs
   the arena-address translator, not host pointers).
4. FIFO timing/flow: real wrap handling, GXWGFifo watermark + break-points,
   draw-to-disk-sync (present pacing currently wall-clock 60 Hz like W).
5. XF/mtx engine (pos is screen-space by deviation today), full TEV combiner
   config, alpha compare/blend regs, GXClearVtxDesc + index buffers.
