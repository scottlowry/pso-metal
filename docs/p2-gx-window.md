# P2-W — GX→Metal window slice (ledger steps 5 + 8–9: first real pixels)

Date 2026-11-26. Scope: `host/gx/**`. Decomp read-only @ `6b38b5383`.
Gate: `host/gx/run_window.sh` → **exit 0** on per-frame **drawable readback**
color-match (not eyeballs); window auto-closes after the frame budget.

## 1. Called-fn inventory (what boot→logo ACTUALLY calls)

`git grep`-style enumeration of `\b(GX|VI)[A-Za-z0-9_]+\(` over the ledger's
boot-to-logo path files (SystemBootInit.cpp, gjSystem.c, TLobby_801E2F94.cpp
(render_tick), TLogo.cpp, TAdSEGAlogo/Soniclogo/ADXlogo/VIConfigGC(.cpp +
_80308200), render_util.cpp): **41 distinct fns, 88 sites** (whole-tree GX
surface is 86 fn / 503 sites — the logo path is the 41×88 slice below).
Per-file splits reproduced from the grep; full counts in the firing table (§5).

| fn | sites | SBI | gjS | LB94 | VCFG | RUtil | kind in host/gx |
|---|---:|---:|---:|---:|---:|---:|---|
| GXSetZMode | 8 | · | 1 | 2 | · | 5 | SHIM |
| GXBegin | 6 | · | · | 6 | · | · | SHIM |
| GXSetZCompLoc | 5 | · | · | · | · | 5 | SHIM |
| GXCopyDisp | 4 | 2 | 2 | · | · | · | **REAL** |
| VIFlush | 4 | 1 | 2 | · | 1 | · | **REAL** |
| GXSetViewport | 3 | · | 2 | 1 | · | · | **REAL** |
| GXSetScissor | 3 | · | 1 | 2 | · | · | **REAL** |
| GXSetPixelFmt | 3 | 1 | 2 | · | · | · | **REAL** |
| GXSetCullMode | 3 | · | · | · | · | 3 | SHIM |
| GXSetAlphaCompare | 3 | · | · | · | · | 3 | SHIM |
| VIWaitForRetrace | 7 | 1 | 4 | · | 2 | · | **REAL** |
| VIGetRetraceCount | 3 | · | 3 | · | · | · | **REAL** |
| VISetNextFrameBuffer | 2 | · | 2 | · | · | · | **REAL** |
| VISetBlack | 2 | 1 | 1 | · | · | · | **REAL** |
| VIConfigure | 2 | · | 1 | · | 1 | · | **REAL** |
| VIGetDTVStatus | 2 | · | · | · | 2 | · | **REAL** |
| GXSetLineWidth | 2 | · | · | 2 | · | · | SHIM |
| GXSetChanAmbColor | 2 | 2 | · | · | · | · | SHIM |
| GXInitTlutRegion | 2 | · | 2 | · | · | · | SHIM |
| GXInit | 1 | · | 1 | · | · | · | **REAL** (opens window) |
| GXSetCopyClear | 1 | 1 | · | · | · | · | **REAL** (clear color) |
| GXDrawDone | 1 | · | 1 | · | · | · | **REAL** (drain+readback gate) |
| GXSetDispCopySrc/Dst/YScale | 1 each | · | 1 each | · | · | · | SHIM |
| GXGetYScaleFactor | 1 | · | 1 | · | · | · | **REAL** (CPU math) |
| GXSetCopyFilter | 1 | · | 1 | · | · | · | SHIM |
| GXSetColorUpdate | 1 | · | 1 | · | · | · | SHIM |
| GXLoadLightObjImm | 1 | 1 | · | · | · | · | SHIM |
| GXInitLightPos / GXInitLightColor | 1 each | 1 each | · | · | · | · | SHIM |
| GXSetChanMatColor | 1 | 1 | · | · | · | · | SHIM |
| GXSetTlutRegionCallback | 1 | · | 1 | · | · | · | SHIM |
| GXSetViewportJitter | 1 | · | 1 | · | · | · | **REAL** (mirrors viewport) |
| GXInvalidateVtxCache / GXInvalidateTexAll | 1 each | · | 1 each | · | · | · | SHIM (unified cache no-op lane TODO) |
| GXAdjustForOverscan | 1 | · | 1 | · | · | · | SHIM |
| VIInit | 1 | 1 | · | · | · | · | **REAL** |
| VIGetNextField | 1 | · | 1 | · | · | · | **REAL** (counter parity) |
| VIGetTvFormat | 1 | · | 1 | · | · | · | **REAL** (NTSC const) |
| VIPadFrameBufferWidth | 1 | · | 1 | · | · | · | **REAL** (640 already aligned) |

(SBI=SystemBootInit.cpp, gjS=gjSystem.c, LB94=TLobby_801E2F94.cpp,
VCFG=TAdVIConfigGC.cpp, RUtil=render_util.cpp. TLogo/TAdSEGA/Sonic/ADX logo
files contain **zero direct GX/VI calls** — they drive state through
gjSystem + render_tick, which is what makes this slice's file set sufficient.)
Signatures follow the **decomp call sites** where they differ from the SDK
(e.g. `GXSetDispCopyDst(fbWidth, xfbHeight)` 2-arg; `VIGetDTVStatus()` 0-arg
in TAdVIConfigGC.cpp:201).

## 2. Real vs counted-shim (host/gx/ layout)

- `gx_metal.m/.h` — **real**: NSWindow + layer-backed CAMetalLayer
  (`framebufferOnly=NO` so the **drawable itself** is getBytes-readable),
  device/queue + clear-encode + ±2/255 readback ported from the proven
  `spike/metal_clear.m`; manual `CATransaction flush` + event pump (no NSApp
  run loop in the driver).
- `gx_shims.c` — all 41 era-named fns. **20 REAL-backed** (window bring-up,
  clear-color state, GXCopyDisp→clear+present, GXDrawDone→wait+readback
  assert, VI retrace pacing + fb ping-pong, viewport/scissor/pixelfmt state
  mirrors) / **21 count-shims** (first-fire printf with decoded args, then
  silent counting; summary table printed by `gx_shim_report_summary()`).
- `gx_window_driver.c` — gate replay of the era order (VIInit → VIConfigure →
  init_gx_display → GXSetCopyClear+double GXCopyDisp → show_first_framebuffer
  → 30 render_tick-style frames). Fired **32 distinct fns / 33 presents /
  32 verifications / 0 failures** in CI.
- `gx_era.h` — port-local prototypes + the minimal GX types the path needs
  (N's `psosh_gx.h`/`psosh_vi.h` PSOSH_TAG decls stay compile-only scaffolds;
  this is the first real backing per their "exact signatures land in P2" note).
- `gx_window.mk` — links **Q's substrate unmodified** (`crt0_nomain.o` arena +
  `osinit.o` mini-OSInit compiled with bringup.mk's exact flag set incl.
  waiver force-include; deviation D3/D4/D5 inherited, nothing new needed).

## 3. Frame pacing decisions (step 9)

- **Retrace clock = wall-clock 59.94 Hz** (`VIWaitForRetrace` sleeps to the
  next 16.684 ms monotonic deadline and bumps the retrace counter);
  `VIGetRetraceCount` reads it. `CVDisplayLink` deliberately deferred: the
  gate needs a deterministic timer thread-free pacing, and the era contract
  is only "block one retrace" + counter monotonicity. TODO(P2): CVDisplayLink
  backing so retraces track the actual panel (D7-style mapping trap: era code
  busy-waits between `OSDisableInterrupts` — our shim has no interrupts, the
  loop terminates via the wall clock instead).
- **present_interval=2 honored as pacing math** (driver gates
  `VIGetRetraceCount() < f*2` like `present_frame_and_wait_vsync`);
  present itself rides `[drawable present]` (vsync-aligned by the window
  server).
- **Swap model**: era XFB double-buffer pointers are kept as identity slots
  (arena addresses from the driver); the host swaps drawables implicitly via
  the CAMetalLayer pool. `GXCopyDisp` (EFB→XFB copy) collapses onto
  clear+present because no pipeline exists yet — with a clear-only EFB the
  copy *is* the clear, so this is era-faithful for steps 8–9, NOT a shortcut
  that survives step 10+ (see §4).
- Colors: the two `GXSetCopyClear` literals of SystemBootInit.cpp:237 —
  `clear={0,0,0,0}` and `0xFFFFFF` — black/white. Chosen deliberately: both
  are colorspace-conversion endpoints (sRGB↔linear identity), so the ±2/255
  corner assert can't drift on wide-gamut displays (D6-class era→macOS trap).
- Alpha=0 on the black frame reads back as stored (BGRA8Unorm, no blend on a
  clear-only pass); assertion checks RGB corners.

## 4. Next-lane work (steps 10+ — display-list translator)

1. **GXBegin/GXEnd + GXVert inline writers → CPU vertex streamer** (N's
   GXBegin×23/GXSetVtxDesc×30/GXSetVtxAttrFmt×21 note): 6 GXBegin sites in
   TLobby_801E2F94.cpp (debug-text quads) are the first consumers; TLogo
   quads go through display lists, so pair this with `GXCallDisplayList`.
2. **GXCopyDisp becomes a real EFB→drawable blit** once anything draws into
   the EFB: introduce the EFB MTLTexture (RGBA16_FLOAT vs RGB8 choice pinned by
   `GXSetPixelFmt` mirror — the state is already recorded) + a textured-quad
   pipeline, yscale/vfilter from the DispCopy mirrors.
3. **GXSetZMode/ZCompLoc/CullMode/AlphaCompare → MTLDepthStencilState/pipeline
   keys** (ledger step 28 install_depth_alpha_state; counters already prove
   60×ZMode + 30×each/30-frame cadence the tracker must cache).
4. **GXInitTlutRegion×80 + GXLoadTlut** → TLUT storage as MTLTextures once
   CI8/IA8 textures load (step 10 texture tables); the callback contract
   (`GXSetTlutRegionCallback`) needs a host-side region table.
5. **Lighting**: `GXLoadLightObjImm` takes the raw 32-B BE bit-packed
   GXLightObj (ledger row 27) — bake into pipeline constants, don't overlay;
   its 1/30-frame cadence says it's boot-only, confirm when TLogo draws.
6. **VIInit/GXInit split**: fold `gxh_open` into a later `GXManage` bring-up
   once the full `init_video_and_gx` body (locked-cache heap shim, gjSystem
   cfg arithmetic) executes as written era code instead of this replay.
7. **CVDisplayLink retrace** (§3 TODO) before anything audio locks to frame
   pacing.
8. Wire `run_window.sh` into CI when a runner has a window server; keep the
   readback assert as the sole pass criterion (it already is).

## 5. Gate transcript (excerpt, real run)

```
gxh: window open 'pso-metal P2-W GX window' 640x480 BGRA8Unorm layer-backed, device=Apple M3 Ultra
[GX-REAL] GXInit(fifo=0x107b14000 size=0x40000 host=640x480) fired
...
gxh f04: drawable 640x480 TL-corner BGRA(0,0,0,0) MATCH rgb(0.000,0.000,0.000) [4/4 corners]
gxh f05: drawable 640x480 TL-corner BGRA(255,255,255,255) MATCH rgb(1.000,1.000,1.000) [4/4 corners]
...  (alternating through f33, all 4/4 MATCH)
=== P2-W called-fn firing table (32 distinct fns observed) ===
totals: 15 REAL-backed, 17 count-shims      (of the 32 fired; 20/21 implemented)
gx_window: window=1 presented=33 verified=32 failures=0 retraces=59 distinct-fns-fired=32
gx_window: PASS — 33 drawn frames all pixel-verified on the drawable
-- gx_window exit: 0
```
