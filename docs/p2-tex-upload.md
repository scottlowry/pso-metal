# P2-AU — texture-upload composition: real texel bytes through REAL era code

Lane AU, Phase 2, the rung after AN's doorstep (docs/p2-tex-supply.md §6), built
on AS's port map (docs/p2-an4-triage.md §4). The AN descriptor pack now flows
through the era upload composition to a heap-allocated pixel buffer whose
read-back hash **equals AN's fixture texels**: the port has loaded a real
texture through era logic. Evidence oracle: pso-decomp @ 6b38b5383 era `.s`
(cited addr+insn at every golden, `host/bringup/ladder5_test.cpp`) + the
byte-matched decomp C. Prior rungs untouched; run.sh green twice after
`make clean` (2026-11-27, arm64-apple-macos12.0).

## 1. The chain as wired (file:line)

```
AN pack {fmt 9, 32x32, texels=img+0x1C}          ladder5_test.cpp S1-S3
  -> init_texture_header_and_alloc  (era 0x803DA12C, text_803DA12C.cpp:9-15)
       PORT: host/bringup/ladder5_seam.cpp:34 — composition, deviation AU-1
  -> init_ginja_texture_header      (era 0x803DB0AC, GinjaTextureHeader.cpp:62-84)
       VERBATIM slice, gen ladder5_tex_glue.cpp (bringup.mk ladder5 block)
  -> get_texture_buffer_size        (era 0x803DB15C, GinjaTextureHeader.cpp:86-94)
       VERBATIM, same slice
  -> GXGetTexBufferSize + __GXGetTexTileShift (era 0x8038BE90,
       libs/dolphin/src/gx/GXTexture.c:53-96 / :98-139 — the bpp table)
       VERBATIM, gen ladder5_gxtexture_glue.c, compiled as C11 (era unit is C;
       C++ rejects its u32->enum call site)
  -> alloc_buffer_from_heap         (era 0x803DB1B4, GinjaTextureHeader.cpp:96-105)
       VERBATIM slice; OSAllocFromHeap = P2-Y era port (osalloc_era.c),
       heap carved over the arena (AU-4), handle lands in the era .sbss global
       locked_cache_heap_handle (RenderSbssGlobals.cpp:12; def in seam TU)
  -> move_data (era TLobby memmove wrapper, Y slice) fills header.pixels
  -> read-back:  fnv1a64(pixels, 1024) == 0xA92D59BE5839FCE5  (E4a RECEIPT)
                 == AN fixture texels; image stays 0x93B9BE4E18606D07 (E4c)
```

Whole-file vs slice — **probes measured, not assumed**:
* `GinjaTextureHeader.cpp` whole-file: REFUSED (`fatal error:
  'dolphin/gx/GXTexture.h' file not found` at :2 under the D4 executing-TU
  include set; behind that sit `<dolphin/os/OSAlloc.h>` + NinjaTypes.h,
  stitch-pending per docs/p2-njs-confirm.md/AR). -> three-function slice.
* `GXTexture.c` whole-file: REFUSED (`'dolphin/gx.h' file not found` at :1 —
  PPC reg types, CHECK_GXBEGIN HW). -> two-function slice.
* `text_803DA12C.cpp` whole-file: PARSE-CLEAN (0 errors, AS was right) but NOT
  executable verbatim — see AU-1. -> hand-composed seam.

Guards: each generated slice is anchored on its exact era signature line and
must contain the hand-traced body tokens (bpp arms, stores, `li r5,0x8`-class
proves); negative guards pin the leak edges (`OSFreeToHeap`/`GXInitTexObj` must
NOT appear), same style as rungs Y/AE/AN. Era files read-only; no make build/,
no gen_waivers, D8 waiver sha gate untouched (bringup.mk reused as-is).

## 2. GinjaTextureHeader struct map vs era (prelude mirror, offset-pinned)

Era def `include/pso/render/GinjaMaterialState.h:13-21`; every write below is
`build/GPOE8P/asm/pso/render/GinjaTextureHeader.s` (`.fn` blocks :133-182,
:186-211, :214-239). Mirror + `static_assert`s live in
`host/bringup/include/ladder5_tex_prelude.h`.

| off(era) | field | era store | golden |
|---|---|---|---|
| +0x00 | format (u32, low byte only) | `clrlwi r0,r6,24` :803DB0CC + `stw r0,0x0(r30)` :803DB0D0 | E1b (fmt 9); F6b (0x809 -> 9: attr bits never reach sizer) |
| +0x04/+0x08 | reserved | **no era store exists** | E1d 0xCC canaries intact — writer touches exactly 6 fields |
| +0x0C | width | `stw r4,0xc(r30)` :803DB0D4 | E1c |
| +0x10 | height | `stw r5,0x10(r30)` :803DB0D8 | E1c |
| +0x14 | size | `stw r3,0x14(r30)` :803DB13C (r3 = bl :803DB138 sizer) | E1f; F2 (still computed when ret==4) |
| +0x18 | flags | zero `stw 0x18` :803DB0F8; or-back :803DB128-30; bits fmt&0x100->0x80000000, fmt&0x200->0x8000, fmt&0x800->0x40000000 | E1e (fmt 9 -> 0); E3a; F6; F6b |
| +0x1C | pixels ptr | `stw r3,0x1c(r30)` :803DB1EC (alloc hit path only) | E1g/E1h; F2/F3 untouched |

Host slot for `pixels` is +0x20 (AU-5): ILP32->LP64 pointer widening, the exact
V1 precedent of osalloc_era.c Cell — all scalars keep era offsets; the era store
pins the FIELD SLOT. sizeof 0x20 -> 0x28.

`OSAllocFromHeap` result placement is additionally pinned through the OSAlloc
header, not just the C: `cell@pixels-32 {size=1056=round32(1024+HEADERSIZE 32),
requested=1024, hd_idx=heap+1}` vs era `addi r3,r6,0x20` :80370BEC +
`clrrwi rX,rY,5` :80370CCC (E1h; block inside arena heap bounds, E1g).

## 3. The bpp table, hand-traced from `.s` (AN-4 closed)

Era `get_texture_buffer_size` is NOT a black box and needed no guessing:
`GXGetTexBufferSize` @ 0x8038BE90 (bl `4B FB 0C F1` @0x803DB1A0) compiles the
format switch as `jumptable_804F50A8` (gx/GXTexture.s:1171-1231) onto 4 arms:
.L_8038BEB8=(row 3,col 3), .L_8038BEC4=(3,2), .L_8038BED0=(2,2),
.L_8038BEDC=(0,0); tileBytes 0x40 iff fmt==6||0x16 else 0x20
(:8038BEE4-:8038BFFC), non-mip size = ((w+2^sX-1)>>sX)*((h+2^sY-1)>>sY)*tileBytes
(:8038BFA8-:8038BFD8, u16 masks :8038BF34/:8038BFA8 = the C u16 params).
**Entry[9] = `.L_8038BEC4` (3,2)** -> 32x32 fmt 9: nx=4, ny=8, 32B tiles ->
**1024 == GVRT size-8 == w*h: the AN-4 "PAL8 = 1 byte/texel" assumption is now
era-table PROVEN** (and GX_TF_C8 == 0x9 in the SDK enum, GXEnum.h:176).
Golden E2 sweeps 7 formats {0,1,3,4,6,9,0xA} through the real function against
the table-derived values (512/1024/2048/2048/4096/1024/2048).
Mipmap branch: mtctr(max_lod=100, `li r7,0x64` :803DB19C)/bdnz loop with the
1x1 break (:8038BF64) — sums pinned: 32x32 mip = 1024+256+64+32+32+32 = **1440**
(E3b), 32x16 mip = 512+128+32+32+32+32 = **768** (F2).

## 4. Failure paths — era behaviour, .s-defined vs UNDEF-PORT

| case | expected (golden) | status |
|---|---|---|
| bad GVR magic | walk NULL, texture_error.code=4 (:803D9D5C-:9D68 `stw r0,0x8(r3)`), upload chain never entered | F1 .s-defined |
| fmt&0x100 non-square | init ret=4 (:803DB0E8-:803DB0F0), header STILL fully written incl size (uncond. :803DB13C), wrapper SKIPS alloc (text_803DA12C.s :803DA144), pixels untouched | F2 .s-defined |
| heap miss (OOM) | OSAllocFromHeap 0 -> `li r31,0x3` :803DB1E4, pixels NOT written; ret 3 is what the wrapper's r3 returns (:803DA154 tail) | F3 .s-defined |
| fmt 99 (>0x3C) / fmt 7 (table default) | silent (0,0)-shift compute 32768 (:8038BE94 `cmplwi r5,0x3c`/`bgt`; tbl[7]=.L_8038BEDC) — **no panic branch exists** | F4/F4b .s-defined; the C `ASSERTMSGLINEV(0x19B,0)` side is UNDEF-PORT (retail-stripped, see §5 AU-2) |
| over-size w=2048 | no clamp, no panic: 256*8*32=65536 — ASSERTMSGLINEV(0x1AB/0x1AC) generated ZERO era .s instructions | F5 .s-defined silent; panic path UNDEF-PORT |

## 5. Deviations roster (continues AN-#)

* **AU-1** The era wrapper (text_803DA12C.cpp:9, parse-clean whole-file) is 1-arg
  and depends on PPC r4-r6 pass-through (p2-an4-triage §1) — unrepresentable on
  arm64 AAPCS. The seam implements the era COMPOSITION sanctioned by triage §4
  ("era 4-arg behavior is wrapper + tail-call") in the only 4-arg shape the era
  itself names it: the caller's forward decl text_803D96A8.cpp:23
  `s32 init_texture_header_and_alloc(u8*, s32, s32, s32)`. Control flow AND r3
  return flow mirror text_803DA12C.s instruction-for-instruction (E1a, F2).
  The ghost-register literal body is deliberately NOT executed on host.
* **AU-2** ASSERTMSGLINEV/ASSERTMSGLINE/ASSERTLINE compile to evaluated-out
  no-ops in the GX slice. Ground: era `.fn GXGetTexBufferSize`
  (0x8038BE90..0x8038BFEC) is a leaf — no bl __assert/OSReport, no panic branch
  anywhere; the asserts generated zero instructions. Not host convenience: the
  no-op IS the retail behaviour (F4/F5 pin it).
* **AU-3** `__cntlzw` -> `__builtin_clz((u32)x)` (PPC intrinsic binding). Era-
  dead: the pow2 pre-compute nx/ny feeding those stripped asserts never appears
  in .s; the substitution cannot change any computed size (verified: E2/F4/F5
  sizes are pure arithmetic).
* **AU-4** locked_cache_heap_handle points at OSCreateHeap(arena+0x1400000,
  +64KB) — host stand-in for the GC locked-cache region, same Y-arena regime as
  DA-4/AN; real locked-cache geometry is the DMA/loader lane's job. Heap id is
  era-verified via cell hd_idx (E1h).
* **AU-5** pixels field LP64 widening (+0x1C -> +0x20, §2) — V1 precedent.
* **AN pack flags reconciliation (not a deviation, a closure):** the AN pack's
  `flags=0x8000` was a supply-side predicate note (is_palette_texture_format
  8/9/10). The ERA header flag for the actual fixture word (format=9) is **0**
  — 0x8000 requires bit 0x200 in the format word (F6 pins both directions).
  Downstream must not conflate the pack annotation with header->flags.
  AN-4 itself: CLOSED (§3). AN-5 copy-leg: exercised via era move_data here.

## 6. What the gx consumer needs next (no host/gx written in this lane)

* Handoff signature is now era-typed: consumers take `GinjaTextureHeader*`
  (this port's mirror; +0x1C era pixels slot holds the heap texel buffer of
  exactly `header->size` bytes) — i.e. `reload_texture_slot_pixels(slot,
  header->pixels)` / `create_texture_state`'s surface shape
  (text_803DA798.cpp:132-158), fed by the seam output proven here.
* AR allow-list touchpoint: GXInitTexObj*(pixels-as-guest-u32, w, h,
  header->format (already u8-clean), wrap, flags-bit0 mipmap) belongs to the AR
  C++ per-TU allow-list stage (docs p2-an4-triage §4 line 3: "same GX lane
  territory"); the host u64 pixels pointer must be stamped through the
  guest-address shim before GX sees it — do not widen GXInitTexObj params.
* Texture-table handle discipline (docs/p2-njs-confirm.md, note only per lane
  rule): NJS_TEXNAME.texattr stays u32 — header/slot pointers must travel as
  handles, never host pointers, once the texlist binds this texture.
* mipmap: the era sizer ALREADY reserved the full chain (E3: 1440 for 32x32
  mip) — the consumer passes `flags & 0x80000000 ? 1 : 0` as GXInitTexObj's
  mipmap byte and nothing else about sizing is its problem.

## 7. Files / regression

New: `host/bringup/{ladder5_seam.cpp,ladder5_test.cpp}`,
`host/bringup/include/{ladder5_tex_prelude.h,ladder5_gx_defs.h}`. Modified:
`bringup.mk` (ladder5 block appended after AN's; prior rules byte-identical),
`run.sh` (block before BRINGUP GREEN; prior lines byte-identical). This doc.

`make -f bringup.mk clean && ./run.sh` twice, exit 0 both:
boot_spine, prs 16/16, Y 13/13, AE 26/26, AN 27/27 (verbatim green),
**AU 28/28**, BRINGUP GREEN. Cross-run diff: only OSGetTick values and one
ASLR heap VA; every pinned hash, size and count identical
(`texels fnv1a64=0xA92D59BE5839FCE5`, `image fnv1a64=0x93B9BE4E18606D07`).
