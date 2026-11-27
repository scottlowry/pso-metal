# P2-BB — V5b wave receipts (lane BB2: genuine sinbd logo supply)

Lane BB2, 2026-11-27. Contract: `fleet/cards-20261127/V5b.md`; prior receipts
`docs/p2-v-wave2.md` (V5 S1–S4). Card t_ce4689a1 S2/S3-genuine closes here.
Waiver header D8 pin for EVERY make this lane: `495cc91a5b9694c4…`
(NEW era — apply-6 re-pin era; re-hashed pre/post every build, zero regen).

## S1 — resource-archive index WALK DECODED (commit `6346e04`)

The disc image carries its own archive-entry index (the era GSL regime's disc-
side twin). Format established read-only, empirically:

- stride-12 records `{u32 name-pool-offset, u32 data-offset, u32 size}`, name
  pool a contiguous null-separated block (calibrated via two anchor entries
  whose (data-offset,size) bytes were byte-identity-matched against loose
  disc files); `find_gsl_entry`-style semantics: name@0-class walk, stride
  constant, strcmp-at-name.
- Verification: 8 loose `.rel` entries re-read from (offset,size) come back
  byte-identical to the loose files (8/8, same-turn). Full enumeration:
  21,390 records persisted to scratch (kept off-repo by policy).
- Genuine texture supply enumerated (names + sizes only, no bytes committed,
  disc path never printed): `Logo.prs`/`Logo_e.prs` (GVMH, 24 GVRT I4 frames
  256x256/128x128 — the TLogo SEGA/ADX streams `load_streamed_prs_texture
  ("logo_e")` names), `sega_logo(_e).prs`, `st_logo.prs`, `f256_ad_pic(_2).prs`,
  `f256_backtext.prs` (I4), and the PAL8 family `f256_hyouji(.small).prs`
  (GVMH, 13 records, desc format word 0x0A09 → era u8 format 9, 256x256
  PAL8 label — fmt-9 CONFIRMED GX_TF_C8 by era .s witness (docs/p2-oracle-1.md); issue #8 defect = flat-vs-TI-8x4 addressing) with SEPARATE palette records `f256_hyouji_a/b/c.gvp` (raw `GVPL`
  chunks, C16, 256 entries) — exactly the era regime TWindowBase.cpp:873-885
  shows (`TLanguage::load_palette_tlut(<.gvp>)` + `load_disc_prs_texture`).

## S2 — genuine blob extracted (local_assets regime; NO bytes committed)

`PRS-decompress f256_hyouji.prs` (era-format stream, header+name-dir
decoded with the TLanguage `read_file_header` working-value rules) → slice
record 0 byte-true (0x10010 B):

- `host/bringup/local_assets/sinbd.gvr` — 65552 B, fnv1a64
  `0x580CC3F5451A3EDC`, GVRT fmt 0x0A09 (u8 9, labeled PAL8 — label CONFIRMED (era GXEnum/GXGetTexBufferSize disassembly; oracle in docs/p2-oracle-1.md; issue #8 root cause = flat-vs-tiled addressing), 256x256.
- `host/bringup/local_assets/hyouji_a.gvp` — 528 B, fnv1a64
  `0x7818A36ACF96C1EB`, sha256[:8] `d22b6dc6`, magic `GVPL`, format 2 (C16
  TLUT), entry_count 256.

## S3 — GX_LOGO_GVR + GX_LOGO_GVP through gx_logo (era phase)

Driver seams (loader/test legs only; every era body untouched):
`GX_LOGO_GVP` sidecar loader (same gitignored local_assets regime);
supply buffers relocated (0x30000 site collided with sinbd/pool/surf at
genuine scale); cache heap 0x10000→0x40000 (genuine sin_board allocs);
TEXN now derived from the supply grid (peek of the GVRT desc BE w/h at
0xC/0xE — pipeline builds AFTER this); deviations BB-3 (GVPL header words
normalized to the BB-1 carrier working regime in the loader leg) and BB-4
(genuine quad sized 1:1 N×N about the same center — a 256-row grid is not
cell-exactly samplable through a 320x240 quad; the fixture keeps its
original 320x240/32 grid byte-identically) declared in-file.

Genuine run `GX_LOGO_GVR=…sinbd.gvr GX_LOGO_GVP=…hyouji_a.gvp
build/gxdl/bin/gx_logo` — exit 0, 12/12 PASS (all gates below are self-consistency proofs; genuine render defect root-caused to flat-vs-TI-8x4 addressing — issue #8, docs/p2-oracle-1.md):

- L7-W1 era consumer bound the GENUINE surface `{id 0xFFFFFFFF, fmt 9,
  256x256, pixels heap}` through the era walk at genuine dims.
- L7-T1 CLUT leg executed from the GENUINE GVPL sidecar; era-chain palette
  fnv1a64 `0x1CC5C22B9BC80AA5` (genuine C16 decoded through the era bodies).
- L7-P1 texel map EXACT at 256x256 vs the era-chain surface (±0) — self-consistency gate: pipeline vs its own era-chain output, not disc ground truth (issue #8).
- L7-P2 region coverage 65536/65536 AT GENUINE DIMS (genuine PAL8 carries
  a=0 texels reading back as the cleared word; the leg proves every pixel
  is drawn-or-matches-its-transparent-texel, fixture leg stays strict).
- L7-P3 readback fnv1a64 `0x76D59FC1622CEE65` == runtime reference
  `0x76D59FC1622CEE65` (self-consistency anchor; NO literal pin of the
  genuine hash anywhere in the TU).

## S4 — both regimes + suites green (D8 `495cc91a…`-era throughout)

| suite | result |
|---|---|
| `GX_LOGO_GVR=… GX_LOGO_GVP=… gx_logo` (genuine era regime) | 12/12 PASS, exit 0 |
| `host/gx/run_window.sh` (synthetic, era phase default) | 12/12 PASS, exit 0 |
| `host/gx/run_window.sh` (AV-1 regime phase) | 10/10 PASS, exit 0 |
| `host/bringup/run.sh` | BRINGUP GREEN 128/128 |
| `host/gx/dl/run_dl.sh` | GX DL GREEN (7-point textured-quad gate) |
| `host/gx/run_vi.sh` | VI-BRANCH PASS both legs @6b38b5383 |
| waiver header | `495cc91a5b9694c4…` verified pre/post EVERY make (NEW sha era) |

L7-C1E/L7-C2E goldens stay byte-exact on the synthetic fallback leg (era
palette fnv `0xE7F4FBE2487670E5` == AV-1, printed by the fixture runs); the
genuine leg correctly SKIPS the golden equality (known-bytes proof applies
to the synthetic carrier only).

## Deviations introduced (this lane)

- BB-3: genuine GVPL header fields normalized to the host carrier regime at
  the loader leg (era bodies untouched; palette payload byte-true).
- BB-4: genuine-regime logo quad sized N×N 1:1 (320x240/32 fixture quad
  unchanged); L7-P2 genuine leg proves drawn-coverage incl. transparent
  texels (a=0 entries are readback-indistinguishable from the cleared word).
