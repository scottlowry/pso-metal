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
| 0xE | **GX_TF_CMPR** (DXT1, BE-u16 **RGB565** endpoints — ~~RGB5A3 endpoints~~ SUPERSEDED by CMPRBE §6, card t_41325af7) | w·h/2, TI 8×8 | Logo/Logo_e/sega/st/nintendo 0x8000/0x2000 ✓; titlealpha 0x8000, f512_mahoujin 0x20000 @512² ✓ |
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
| sega_e01 | **CMPR TI8×8, 4×DXT1 row-major sub-blocks, RGB5A3 endpoints** | **0.159** | PICK-2 — endpoint law SUPERSEDED CMPRBE §6; layout pick STANDS; re-measured 0.1058 |
| sega_e01 | I4 TI8×8 gray | 0.835 | dead (layout-only; re-measured 0.8321 — unchanged) |
| sega_e01 | I4 FLAT gray (manifest-label reading) | 0.228 | artifact (see note) |
| sega_e02 | **CMPR TI8×8** | **0.241** | PICK-2 — re-measured 0.1738 under CMPRBE law |
| sega_e02 | I4 TI8×8 gray | 0.834 | dead (re-measured 0.8332 — unchanged) |
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
**GX_TF_CMPR** DXT1-in-RGB5A3 [endpoint gloss SUPERSEDED — BE u16 RGB565,
see §6], TI 8×8
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

## 5 — FIX LANDED (relay 2026-11-27, owner-confirmed oracle)

Owner verdict: board top-left = genuine sinbd art incl. the "SONIC TEAM SEGA
2000 2002" line; bottom-left pair = SEGA logo fragments. Both ranked picks
CONFIRMED. Implemented in `host/gx/dl/**` (seat scope):

1. **TI 8×4 expand** — `gx_logo_clut.h` new `gxh_pal8_clut_expand_ti8x4`
   (era addressing law §3; replaces the flat call at the era-supply leg,
   `gx_logo_driver.cpp` expand site). Flat kept ONLY as the L7-P4 canary.
   AV-1 leg untouched (verified below).
2. **RGB5A3 decode fix** — `gxh_tlut_rgba8` fmt==2: opaque leg 444→**555**
   (bits 14..0) and alpha leg RGBA322→**RGBA444** (both were lossy), with
   the green-nettle endpoint proof cited in-source. Second-order catch: the
   16-bit carrier was being read NATIVE-LE on arm64 — era working value is
   BE (PPC `lwz`); the BB-3 comment claimed BE but the code didn't. Fixed to
   explicit BE read (fmt==3 RGBA8 leg untouched).
3. **L7-P4 coherence gate** (era regime only, `#ifndef GX_LOGO_AV1`):
   adjacent-vs-row-shuffled metric on the GPU READBACK `got[]` + planted
   FLAT-decode CANARY (old behavior) computed at runtime; PASS requires
   readback < 0.500 AND canary ≥ 0.500, both printed every run.
   Fixture regime: leg counts, prints SKIPPED.

**Gate results post-fix (all rc=0; waiver `d4382fcd0659357f` PRE==POST on
every one of 2 make events, never regen):**

| run | result | receipts |
|---|---|---|
| era GENUINE (sinbd.gvr+hyouji_a.gvp) | **13/13 PASS** | coherence TI **0.2763** / CANARY **0.7164** (python-board values 0.2764/0.7164 — agree); region fnv OLD `0x76D59FC1622CEE65` → NEW `0xC7088D6B25AA194A` (moved honestly: TI addressing + palette fix); era CLUT palette fnv OLD `0x1CC5C22B9BC80AA5` (recomputed from the pre-fix decode path) → NEW `0x5430FC251C0E5F40`; intermediate single-fix build measured `0x6B8D53E17B2937C0` (TI without BE fix) — proves both legs were required |
| era FIXTURE | 13/13 PASS | L7-P4 counts as SKIPPED leg |
| AV-1 stand-in | **10/10 PASS** | region fnv `0x88A8EF94CB09FCD0` — UNCHANGED (untouched-path proof) |

**End-to-end owner proof:** `GX_DL_DEBUG=1` genuine run → 640×480 backbuffer
dump → 256×256 quad crop at (192,112) → `/tmp/cla/sinbd_fixed.png`
(outside repo; artifact bytes not committed). **Byte-exact vs the
owner-verified python board render: 0 differing bytes / 196608.**
md5 `df99b87c2f79376ef87332a66af9048b`.

**Build-seat note (repo-wide, integrator):** decomp live tree moved to
`620fdbed88` (upstream 11:19–14:40 restructure dropped `src/pso/unk/` +
`include/pso/unk` into new homes) — this seat rebuilt against a READ-ONLY
`git archive 6b38b53837` export in scratch (`DECOMP_ROOT` override, zero
writes to decomp). Other lanes' next builds will hit the same wall; the
pin story needs an integrator decision.

## 6 — CMPRBE ENDPOINT RE-ANCHOR (card t_41325af7, build seat, 2026-11-27)

Owner boot footage (machine-local reference; numeric stats citable only, the
image is NEVER committed) exposed the endpoint law in §2/§3 as a BUG-MADE-
LAW: GC hardware reads CMPR endpoints as BIG-ENDIAN u16 plain **RGB565**
(no alpha branch ever; index-3 at `c0<=c1` is OPAQUE BLACK, not transparent).
The layout/structure picks (TI 8×8, rm `bx+2·by`, tlut -1) STAND — index
bits were never affected; only the endpoint interpretation was wrong, which
is why every self-consistency gate passed for days. Block census on the
landed sega_e01 bytes (python, same-turn): `second halfword == 0xFFFF` in
3837/4096 blocks, `first == 0xFFFF` in 49, field = index-1 (62388/65536 px).
Fix landed in `host/gx/dl/gx_logo_clut.h` (`gxh_cmpr_block_rgba8` BE halves
+ new `gxh_rgb565_rgba8`; CI8/TLUT `gxh_rgb5a3_rgba8` path UNTOUCHED —
different format, signboard human-verified).

Coherence anchors RE-MEASURED under the fixed law (python port of the C
gate metric incl. the xorshift row-shuffle; C runtime agrees to 4 dp):

| record | law | was (RGB5A3 endpoints) | now (BE/RGB565) |
|---|---|---|---|
| sega_e01 | CMPR TI8×8 | 0.159 (C 0.1589) | **0.1058** (C 0.1058) |
| sega_e02 | CMPR TI8×8 | 0.241 (C 0.2410) | **0.1738** |
| sega_e01/e02 | I4-TI canary | 0.835/0.834 | 0.8321/0.8332 (layout-only, law-independent) |

The fixed law decodes MORE coherent art under the metric than the old law —
the coherence instrument never had chroma-direction sensitivity (mean
adjacent RGB distance, channels summed), hence the blind spot. The hue/
chroma witness is now gate-enforced (docs/p2-seq-1.md §5, L9-U1 inverted).

## Deviations introduced
Steps 1–3: none (see below). Step 4: no new gitignored slices; disc
untouched; suite beyond host/gx/dl NOT run (integrator reruns run_window);
AV-1 + fixture regimes byte-identical in behavior where claimed.
