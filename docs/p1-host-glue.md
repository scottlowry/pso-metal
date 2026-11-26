# P1 host glue — Metal substrate

Phase 1 prep: what the host side must provide before ported PSO boot code
(`init_video_and_gx`, docs/boot-spine.md S7) has anywhere to render.

## P1-K: headless Metal offscreen clear — PROVEN (2026-11-26)

Spike: `spike/metal_clear.m`, runner `spike/run_metal.sh`. Creates the default
`MTLDevice`, a 640×480 `BGRA8Unorm` render-target texture (the exact size/format
PSO's framebuffer needs: `GXNtsc480IntDf` → XFB 640×480 RGB, `GX_PF_RGB8_Z24`,
boot-spine.md S6/S7/S9), encodes a render pass that clears it to
RGBA(0.25, 0.5, 0.75, 1.0) with no window, no `CAMetalLayer`, and no shaders
(loadAction clear alone is a valid pass), reads the whole texture back into host
memory via `-[MTLTexture getBytes:]`, and asserts 5 sampled pixels plus a
full-frame scan within ±2/255. Exit 0 = pass; CI-able.

Real run output on this Mac (Apple M3 Ultra, MacOSX27.2 SDK, clang 21):

```
Metal device: Apple M3 Ultra (lowPower=0)
Render target: 640 x 480 BGRA8Unorm
Render pass (clear to RGBA 0.25,0.50,0.75,1.00): OK
pixel(  0,  0) = BGRA(191,128,64,255) -> rgba(0.2510,0.5020,0.7490,1.0000) MATCH
pixel(639,479) = BGRA(191,128,64,255) -> rgba(0.2510,0.5020,0.7490,1.0000) MATCH
pixel(319,240) = BGRA(191,128,64,255) -> rgba(0.2510,0.5020,0.7490,1.0000) MATCH
pixel(  0,479) = BGRA(191,128,64,255) -> rgba(0.2510,0.5020,0.7490,1.0000) MATCH
pixel(639,  0) = BGRA(191,128,64,255) -> rgba(0.2510,0.5020,0.7490,1.0000) MATCH
Full-frame check: all 307200 pixels within tolerance (±0.0078)
PASS: offscreen 640x480 clear + readback round-trip
-- metal_clear exit: 0
```

Findings that transfer to the real layer:

- **Headless works.** `MTLCreateSystemDefaultDevice()` succeeds with no window
  server session; the port's boot path can bring up graphics before any UI.
- **Quantization:** clear 0.5 → byte 128, 0.25 → 64, 0.75 → 191. GX writes 8-bit
  channel values; the Metal clear path must round byte→float exactly as
  `n/255.0` and tests need ±1 LSB tolerance, never bitwise-equal compare.
- **macOS header gap:** `MTLTextureUsageHostReadback` does not exist in the
  macOS SDK (iOS-only); on macOS textures are `getBytes:`-readable with
  `RenderTarget|ShaderRead` usage. Don't copy iOS readback samples verbatim.
- `bytesPerRow = width*4` tight packing holds for a fresh texture; the EFB-copy
  layer still must pass `bytesPerRow` through rather than assume.

## What the real GX→Metal layer must add next

Per host-surface.md the graphics pipe is **86 GX functions / 503 call sites** —
the XL subsystem. This spike covers only the framebuffer end of the chain
(`show_first_framebuffer` + clear). In dependency order:

1. **GX state shadow + depth buffer.** `GXInit` display structs → viewport/scissor
   (640×480), `GXSetPixelFmt(RGB8_Z24)` → paired `Depth32_Stencil` (or D24-similar)
   texture, `MTLRenderPassDescriptor` with depth attachment + loadAction mapped
   from `GXSetClearZValue`-style clears. Highest-frequency calls to support first:
   `GXSetZMode`×33, `GXSetZCompLoc`×25 (`gx/GXPixel.h`) → `MTLDepthStencilState`
   table keyed by the GX compare-fn enum.
2. **Vertex streamer + FIFO semantics.** `GXBegin`×23 / `GXSetVtxDesc`×30 /
   `GXSetVtxAttrFmt`×21 (`gx/GXGeometry.h`) + the `GXWGFifo` inline writers: a
   CPU-side streamer that decodes the big-endian FIFO command words / immediate
   vertex data into a host vertex buffer, then `[encoder drawPrimitives:]`.
   `GXVert.h` inlines are BE bit-packers — port the semantics, not the bytes
   (boot-spine.md S7: "re-implement GX semantics on Metal, do not port the byte
   stream"). `GXPosition3f32`/`GXColor1u32`/`GXTexCoord2f32` → `MTLVertexDescriptor`.
3. **Tev/lighting fixed-function.** `GXSetTevOrder`-family + light objects →
   compiled `MTLRenderPipelineState`s approximating the tev combiner; exact
   blend order is the known risk (host-surface.md flags this as the XL driver).
4. **Present pacing.** `set_present_interval(2)` → retrace-paced present against
   `CVDisplayLink`/`MTLDrawable` once a window exists; double-FB flip is just
   two of these 640×480 textures.
