# P2-ORCL — GVRT fmt-9 / fmt-0xE DECODE ORACLE (lane ORCL, wave-5.5, card t_6f558da7)

Scratch-only lane (steps 1–3). ZERO repo builds (waiver `d4382fcd0659357f…`
untouched), ZERO host edits this pass, disc untouched (fixtures sliced
READ-ONLY from `local_assets/`). Decompressed matrix + board live under
`/tmp/orcl/` (gitignored scratch; `grid.png` = human-oracle deliverable).

## 1 — DECISIVE ERA WITNESS (asm, read-only @6b38b5383)

`build/GPOE8P/asm/gx/GXTexture.s:9-113` `.fn GXGetTexBufferSize`
(= 0x8038BE90, `.file "GXTexture.c"`, official Dolphin-SDK library unit —
`GinjaTextureHeader.cpp:93` calls it and includes `<dolphin/gx/GXTexture.h>`).
Switch `cmplwi r5,0x3c` + `jumptable_804F50A8` (`GXTexture.s:1170-1233`, 61
entries). Three real cases + dead case; size =
`bytes × ceil(w / 2^rx) × ceil(h / 2^ry)` (loop over mip count, `:35-107`):

| label | (rx,ry) | bytes | bpp class |
|---|---|---|---|
| `.L_8038BEB8` | (3,3) | 32 | **4 bpp** — TI 8×8 |
| `.L_8038BEC4` | (3,2) | 32 | **8 bpp** — TI 8×4 |
| `.L_8038BEC4/BED0 ×64` (`r5=0x40` iff fmt∈{6,22}, `:36-44`) | — | 64 | 32 bpp |
| `.L_8038BED0` | (2,2) | 32 | **16 bpp** — TI 4×4 |
| `.L_8038BEDC` | (0,0) | 32 | dead/CMPR-family aux space (`li r8,0; li r0,0`) |

Jumptable per-index classes (`idx: class`, A=BEB8/B=BEC4/C=BED0/D=BEDC):
(0x804F50A8, entries at `GXTexture.s:1172-1232`)
`0:A 1:B 2:B 3:C 4:C 5:C 6:C(×64) 7:D 8:A 9:B 10:C 11-13:D 14:A 15,16:D 17:B
18:D 19:C 20,21:D 22:C(×64) 23-31:D 32:A 33:D 34:B 35:C 36-38:D 39-42:B 43:C
44:C 45-47:D 48:A 49-56:D 57:B 58:B 59:D 60:C`

Enum authority in-tree: `libs/dolphin/include/dolphin/gx/GXEnum.h:144-180`
— `GX_TF_I4=0x0, I8=0x1, IA4=0x2, IA8=0x3, RGB565=0x4, RGB5A3=0x5,
RGBA8=0x6, CMPR=0xE; GX_TF_C4=0x8, C8=0x9, C14X2=0xA`. The table's bpp
series agrees entry-for-entry with those names (0→4, 1→8, 2→8, 3→16, 4→16,
5→16, 6→4×4×64=32bpp RGBA8 ✓, **8→4bpp C4 ✓, 9→8bpp C8 ✓, 10→16bpp C14X2 ✓,
14→8×8×32=4bpp CMPR=DXT1 ✓**).

## 2 — CALLER-SIDE CROSS-CHECK (one hypothesis survives)

* `init_ginja_texture_header` (`GinjaTextureHeader.s:133-182` /
  `src/pso/render/GinjaTextureHeader.cpp:62-95`): `header->format = (u8)fmt` (`:65`)
  — **the GVRT desc fmt word's LOW BYTE is passed verbatim as the
  `GXGetTexBufferSize` format arg** (`:93`, lvl from flag bit0, type=100).
  Attr bits: `fmt&0x100`→mip/+0x80000000, `fmt&0x200`→+0x8000,
  `fmt&0x800`→+0x40000000.
* `is_palette_texture_format` (`src/pso/ui/frontend/TAdSinglePlyChrSelectGC_803DA038.cpp:26-34`)
  = `switch((u8)fmt){case 8:case 9:case 10: return 1;}` → with the SDK enum
  this is **{C4, C8, C14X2}** — the CI family. (The FRMSPEC gloss
  "CI8/CI14/CI24" was off-by-one in the NAMES; the numbers were right.)
* `reload_texture_slot_pixels` copies `chunk->size-8` raw (FRMSPEC §F0) —
  so disk payload == `GXGetTexBufferSize` output. Manifest arithmetic
  (`docs/p2-logo-e-supply-1.md`) then agrees with EXACTLY ONE hypothesis:

| fmt u8 | era name (table+SDK) | law | manifest spot-checks |
|---|---|---|---|
| 9 | **GX_TF_C8** (CI8/PAL8) | 1 B/px, TI 8×4 | hyouji_a/b/c/actionpallet/duelcount/mainmenu 0x10000 @256², carsol 0x4000 @128² ✓ |
| 8 | GX_TF_C4 | 4 bpp, TI 8×8 | r256_symobj01..03/symparts 0x8000 @256² ✓ |
| 0xE | **GX_TF_CMPR** (DXT1, RGB5A3 endpoints) | w·h/2, TI 8×8 | Logo/Logo_e/sega/st/nintendo 0x8000/0x2000 ✓; titlealpha 0x8000, f512_mahoujin 0x20000 @512² ✓ |
| 5 | GX_TF_RGB5A3 | 2 B/px | f256_ad(_eng)/psotitle 0x20000 @256² ✓ (manifest "RGB565" mislabel — same bpp) |

**VERDICTS** (vs the card's doubts):
1. fmt 9 = **CI8, 1 index byte per texel** — BB-era "PAL8" label stands;
   the card's CI14 hypothesis is REFUTED by its own decisive witness:
   65536 B cannot hold 256² × 16bpp-class CI14/C14X2, and no nibble-packed
   CI14 variant exists in the table (CI-packing candidates all require
   ≠1 B/px). Hyouji GVPL sidecars carry exactly 256 C16 entries — a CI8
   TLUT. "2+2+2+2 unpacked CI14" is dead: the byte IS the index.
2. fmt 0x0E = **GX_TF_CMPR, NOT I4, not 'I4'** — the manifest's "I4" label
   is wrong (sizes agree with both only because both are 4bpp; the table
   + `I4=0x0≠0xE` + the DRAW-path gate `surface->header.format ==
   GX_TF_CMPR` in `src/pso/render/GinjaMaterial.cpp:372,390,423` (alpha-
   compare/z-loc legs on exactly the 0x0E/0x80E records + flag 0x40000000
   from fmt bit 0x800) pin CMPR. No 2bpp class exists in the table at all.
   f512_mahoujin 512² 0x20000 = cmpR w·h/2 ✓.

## 3 — DECODE MATRIX (scratch `/tmp/orcl/`, fixtures = local_assets, receipts re-hashed same-turn: sinbd `580CC3F5451A3EDC`, hyouji_a `7818A36ACF96C1EB`, sega_logo_e01 `A98C4BF3926BC315` @0x28430, e02 `E604EFC97A26324A` @0x30440 — matches manifest; NO disc read, NO new slices)

Records: `sinbd` (fmt 9, 256²), `sega_e01`/`sega_e02` (fmt 0xE, 256² — the
sega block lives inside the landed `logo_e.gvm`, so no gitignored additions
were needed). Palette: `hyouji_a.gvp` C16 (RGB5A3) 256 entries.
Coherence = mean adjacent RGB distance ÷ row-shuffled baseline (lower=better;
~1.0 = noise):

| record | hypothesis | coherence | board |
|---|---|---|---|
| sinbd | **C8 TI8×4 tiled + C16 TLUT** | **0.276** | PICK-1 |
| sinbd | C8 flat raster + TLUT (current driver route) | 0.716 | = owner's "random lines" |
| sinbd | CI14-packed nibble CANARY (table-dead) | 1.032 | dead ✓ |
| sega_e01 | **CMPR TI8×8, 4×DXT1 row-major sub-blocks, RGB5A3 endpoints** | **0.159** | PICK-2 |
| sega_e01 | I4 TI8×8 gray | 0.835 | dead |
| sega_e01 | I4 FLAT gray (manifest-label reading) | 0.228 | artifact (see note) |
| sega_e02 | **CMPR TI8×8** | **0.241** | PICK-2 |
| sega_e02 | I4 TI8×8 gray | 0.834 | dead |
| sega_e02 | I4 FLAT gray | 0.189 | artifact |

Sub-block ordering probe (e03/e04, rm=`bx+2·by` vs cm=`by+2·bx`): rm wins
both frames (0.232<0.279, 0.287<0.338). I4-flat trails CMPR on e01 and its
e02 "win" is the known DXT-index-nibble artifact (18% ink, full bbox,
endpoints census: 0x0 black / 0xFFFF white / 0x100 dark-green holly /
0xEF08 berry-red RGBA444 — nettle-logo palette). Vision reads the sinbd
tiled candidate as coherent HUD art with a **legible "© SONICTEAM / SEGA
2000, 2002" pixel-font line** — independent human-grade witness. The e01
CMPR candidate shows the corner-locked sting mark (frame 01 of an
8-frame animation).

**Ranked pick (HUMAN ORACLE decides; board = `/tmp/orcl/grid.png`, 9 labeled
tiles + metrics):** 1) sinbd = fmt 9 **GX_TF_C8**, TI 8×4 tile addressing
(`off = 32·((x>>3) + (w>>3)·(y>>2)) + 8·(y&3) + (x&7)`), indices through the
attached GVPL C16 bank (TWindowBase.cpp:866-901 regime). 2) 0xE frames =
**GX_TF_CMPR** DXT1-in-RGB5A3, TI 8×8
(`off = 32·((x>>3) + (w>>3)·(y>>3)) + 8·((x>>2&1) + 2·(y>>2&1))`), tlut -1.

## 4 — DRIVER IMPLICATIONS (step 4 stays conditional on relay)

`host/gx/dl/gx_logo_driver.cpp:337` expands via
`gxh_pal8_clut_expand` = **flat** row-major (`gx_logo_clut.h:36-42`) — the
era container is TI 8×4. That mismatch is the owner-visible "random lines"
defect class; the fix leg is a TI expand, NOT a re-label of the palette
(9 IS CI8 — BB label was right). Secondary findings for the relay:
(i) `gx_logo_clut.h:67-79` fmt==2 (RGB5A3) opaque leg decodes RGB444 from
bits 11-0 — loses 1 bit/channel vs the era 555-in-15-bits (DXT endpoint
census above proves 555); (ii) docs/p2-logo-e-supply-1.md "I4" labels for
fmt 0x0E/0x80E should read CMPR — integrator/owner call whether I4P's doc
gets an errata line (NOT my path, not staged).

## Deviations introduced
None beyond lane scope: zero builds, zero commits to host/**, no new
gitignored slices (all fixtures already landed), disc path never touched or
printed; scratch + board confined to /tmp/orcl/.
