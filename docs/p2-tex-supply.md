# P2-AN — Texture supply chain to the upload doorstep

Lane AN, Phase 2. Closes AE's named second GX-wall (docs/p2-bringup-3.md: the quest
path ends at `0x801EB5EC bl load_sin_board_image__FPCvUl`, whose continuation does
TEXTURE UPLOAD from decoded bytes). This rung supplies those decoded bytes
**authentically, without touching host/gx**: a synthetic GVR archive off fake_disc,
walked by the REAL era decoder slice into an era-shaped descriptor + byte-true
texel buffer — the last pure-logic artifact before the GX door.

## 1. Fixture byte map (`host/bringup/fake_disc/`, generator `gen_gvr_fixture.py`)

### `sinbd_tex.gvr` — 0x41C bytes (fnv1a64 `0x93b9be4e18606d07`)

This is exactly the image shape `load_sin_board_image(image, size)` receives after
`qst_op_call_image_data`'s PRS decode (PRS chain itself proven in AE rung 3).

| Off | Bytes | Field | Swap regime | Why |
|---|---|---|---|---|
| 0x00 | `47 42 49 58` "GBIX" | chunk magic | **psoProt** (identity on LE) | era SWAPS it: 0x803D9CD8 `lwz` + `bl swap_gvrt_word` (:9CE4); PPC working = 0x58494247 = raw LE load on arm64 |
| 0x04 | `04 00 00 00` | chunk len 4 | **psoProt** | era swaps: 0x803D9D34; skip math :9D48-:9D54 (`srwi/slwi` = len & ~3) lands next chunk at +12 |
| 0x08 | `00 00 09 99` | global id 0x999 | **psoBe** | era reads RAW, unswapped: 0x803D9D40 `lwz r0,0(r29)` / :9D44 `stw r0,0(r28)` — D1-class silent-flip site, port must ADD the swap |
| 0x0C | `47 56 52 54` "GVRT" | chunk magic | **psoProt** | same seam as GBIX (H D3; compat/README-byteswap.md §2) |
| 0x10 | `08 04 00 00` | chunk size 0x408 | **psoProt** | era swaps in the GVM walk (:436-437); texel run length consumed as `chunk->size - 8` (text_803DD5E0.cpp:442) = 1024 |
| 0x14 | `00 00 00 09` | desc.format 9 | **psoBe** | era `move_data` copies 8 RAW bytes (0x803D9D0C-:9D18, `li r5,0x8`) into `GvrTextureDesc` — NO swap anywhere; PPC working = BE-interpreted |
| 0x18 | `00 20` | desc.width 32 | **psoBe** | same raw-copy field |
| 0x1A | `00 20` | desc.height 32 | **psoBe** | same raw-copy field |
| 0x1C | 1024 B texels | D4 payload | **byte-true** | H §2.1 D4: "texel words … uploaded/copied as bytes; keep byte-true". Pattern `(x*3 + y*5) ^ ((x^y)<<2)` (fnv1a64 `0xa92d59be5839fce5`) |

Format 9 is palettized per the era predicate `is_palette_texture_format`
(cases 8/9/10, TAdSinglePlyChrSelectGC_803DA038.cpp:26-33) → `tlut_name` kept
(text_803DA798.cpp:147-151).

### `gsl_tab2.dat` — 4 × GslArchiveEntry (0x30 stride) = 192 B

Rows 0–2 are AE's `gsl_tab.dat` **verbatim** (ladder3 B1/B2/B3 stay green — the
old file is untouched); row 3 adds
`sinbd_tex.gvr, archive_offset 0, file_size 0x41C, decoded_size 0x41C` in the same
LE-image working-value regime AE pinned (deviation DA-4: host fill stands for
`open_gsl_archive`; the D1 true-disc `psoBeRead/Write32` TOC pass per
compat/README-byteswap.md §1 remains the port's job at the real-DMA chokepoint).

## 2. The GVRT trap, executed (docs/p1-format-net-disc.md §2.4 + README §2)

A single GVR image mixes BOTH regimes: the DC-legacy fields the era *already*
swaps (magic/len/size → working value survives as `psoProtReadNN` identity on our
LE host) and the GC-era fields the era reads *raw* (GBIX id, GvrTextureDesc →
`psoBe*` conversions, the D1 pattern). Golden B1 proves the equivalence chain
`p2an_era_mirror32(psoBeRead32("GVRT" bytes)) == psoProtRead32(same bytes) ==
0x54525647`; golden B2 pins the failure mode — bolting `psoBe32` onto the
era-swap'd magic double-swaps it to 0x47565254 and the walk falls into
`texture_error.code = 4` (0x803D9D5C-:9D70). All conversions flow through
`compat/pso_byteswap.h`; the only TU that can bind it is
`host/bringup/ladder4_supply.c` (the header refuses C++ via its C11 gate), so the
C++ TUs call its thin extern-"C" wrappers. **Zero hand-rolled swaps.**

## 3. Real decode vs slice vs harness

* **VERBATIM era slices** (guarded generated TUs, same ladder.mk pattern as Y/AE):
  `find_gvrt_texture_chunk` (text_803D96A8.cpp:160-186, home 0x803D9C98 — NEW rule
  `$(GEN)/ladder4_gvr_glue.cpp`; guard checks 4 traced body lines + absence of the
  swap fn) + AE's `find_gsl_entry` (ladder3_gsl_glue.o) + Y's `move_data`
  (move_data_glue.o) consumed by the walk at 0x803D9D18.
* **NOT sliced**: the era *definition* of `swap_gvrt_word` (text_803D96A8.cpp:155-
  158) — replaced at the prelude by the psoProt identity seam (deviation AN-1
  below). Its formula is kept as `p2an_era_mirror32` for the PPC-side golden
  arithmetic only.
* **Not real-called** (documented boundary, not a cheat): everything past the walk —
  `bind_texlist_texture/create_texture_state` need the texture slot/surface pools
  (`.sbss` state + `find_free_texture_slot` reaching GX-adjacent state), and the
  texel-sizing fn `init_texture_header_and_alloc(u8*, s32, s32, s32)` has **no
  definition in the decomp at 6b38b5383** (forward decl only,
  text_803D96A8.cpp:23). This is the honest doorstep; the supply pack below is its
  input.

## 4. Deviations roster (continues AE's DA-# with AN-#)

* **AN-1** `swap_gvrt_word` provided as `*p = psoProt32(*p)` identity seam
  (ladder4_gvr_prelude.h), era call structure preserved in the verbatim slice.
  Per compat/README-byteswap.md §2; on a BE host psoProt32 becomes the byte
  mirror automatically (pso_byteswap.h:93-98), so no era semantics are lost.
* **AN-2** psoBe fixups for the era-raw fields (GBIX id, desc) happen at the
  SUPPLY boundary in ladder4_test.cpp after the era walk wrote its raw bytes —
  never inside the slice (rule: slices stay era-verbatim; README §2.4 working
  values converge at the boundary).
* **AN-3** Disc-table fields stay in AE's rung regime (raw LE-image working
  values, DA-4 host fill); the real-disc BE TOC + `open_gsl_archive` swap-on-load
  pass is deferred to the DMA-integration lane, pinned by README §1, not
  re-litigated here.
* **AN-4** PAL8 = 1 byte/texel assumption for `hdr.size` sanity (D1 golden cross-
  checks `size-8 == w*h`). The bpp table lives in the unported
  `init_texture_header_and_alloc(u8*,s32,s32,s32)`; the consumer lane must
  re-derive size from the real header fn — the GVRT `size` field itself (0x408) is
  the era-authoritative length.
* **AN-5** No heap alloc in this rung (`TQuestVmDownLoadServer::alloc`/`xmalloc`
  copy leg of load_sin_board_image skipped): the walk is exercised on the disc
  buffer directly; copy-into-heap is already proven for the same shim path in AE
  B4 / Y W4.

## 5. Golden coverage (ladder4_test.cpp — 27 checks, all `.s`-addr-quoted)

L0.1-4 layout mirrors (GvrTextureDesc 8B ↔ `li r5,0x8`; chunk 0x10 ↔
`addi r3,r29,0x10`; texture_error.code@8 ↔ `stw r0,0x8(r3)` :9D68).
A0-A6 fixture truth through the AE chain (4×0x30 table, `find_gsl_entry` stride
walk hit @idx3, shim read == entry.file_size, ASCII magic, len/size psoProt).
A7-A8/B1-B3 the two-regime byte table proofs incl. the pinned trap.
C1-C7 the REAL era walk: pixels == img+0x1C (:9D1C), unswapped id raw + fixup
(:9D40-44), byte-true desc copy (:9D18), no-GBIX sentinel leg (:9CC8-:9CCC),
both unknown-tag error legs (`beq`/`bge` switch edges :9CF0/:9CF4 → :9D5C).
D1-D4 supply-pack + texel stability: full 1024-byte pattern recompute, pinned
fnv1a64s, read-only proof (image hash byte-unchanged across the walk), canary
`raw[0x41C] == 0xCC`.

Stability: `make -f bringup.mk clean && run.sh` twice — identical
`texels fnv1a64=0xA92D59BE5839FCE5`, `image fnv1a64=0x93B9BE4E18606D07`, exit 0.

## 6. Consumer handoff note (for the next gx lane — this lane writes NO host/gx code)

The pack printed at the doorstep (values golden-pinned above):

```
GinjaTextureHeader-shape { format=9, width=32, height=32, size=1024,
                           flags=0x8000 /* colour-indexed */,
                           pixels=<img+0x1C, byte-true> }   id=0x999   tlut=0
```

Expected consumer signature: the era chain `load_texture_entry`
(attr 0x40000000 → `texture_loaders[1] = load_texture_from_memory`,
text_803DD5E0.cpp:26-58/:120) → `bind_texlist_texture` → `create_texture_state`
(text_803DA798.cpp:132-158) → `reload_texture_slot_pixels(slot, pixels)` +
`copy_texture_pixels(GinjaTextureHeader*, src)` — i.e. the Z-lineage quad's
texture side consumes `{GinjaTextureHeader-shaped descriptor, raw PAL8 texel
buffer}` exactly as produced here (`sin_board_texture_surface.set_render_rect` /
`set_texture_name(..., 0x40000000)`, TObjSinBoard.cpp:111-121, is the caller
shape). What remains for the gx lane is the upload body itself
(init_texture_header_and_alloc + GXInitTexObj*): point it at this
descriptor + pixels and the logo/sin-board path is fed.

AJ's p2-njs-confirm.md (landed mid-run, consumed): **NJS_TEXNAME /
NJS_TEXLIST CONFIRMED LOCKED-FORMAT** (resolutions-1:60/61 → HIGH) and image-copy
obligation #4 pins "PRS-decompressed texel chunks keep BE texel order / palette
banks". Both match this rung exactly: the `sin_board_texture_name` entry shape
(`{filename,attr,union}` set field-wise at TObjSinBoard.cpp:119 via
set_texture_name) keeps u32 slots — the supply pack stores no host pointers into
any image record — and the 1024 texel bytes were never reinterpreted
(D3/D4 check them as BYTES only).

## 7. Files / regression

New: `host/bringup/ladder4_{test.cpp,supply.c}`,
`include/ladder4_gvr_prelude.h`, `fake_disc/{gen_gvr_fixture.py,sinbd_tex.gvr,gsl_tab2.dat}`,
this doc. Modified: `bringup.mk` (ladder4 block appended; prior rules byte-
identical), `run.sh` (block inserted before `BRINGUP GREEN`; prior lines
byte-identical), `fake_disc/README.md`.

run.sh after `make clean`: boot_spine ✓, Q 16/16, Y 13/13, AE 26/26, **AN 27/27**,
exit 0 (2026-11-26, arm64-apple-macos12.0, decomp @ 6b38b5383).
